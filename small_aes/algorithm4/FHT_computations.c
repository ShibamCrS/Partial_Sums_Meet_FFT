#include "FHT_computations.h"
void fht_uint(uint64_t *buf, uint64_t n) {
    uint64_t size = 1UL << n;
    for (uint64_t i = 0; i < n; ++i) {
        uint64_t s1 = 1UL << i;
        uint64_t s2 = s1 << 1;
        for (uint64_t j = 0UL; j < size; j += s2) {
            for (uint64_t k = 0; k < s1; ++k) {
                uint64_t u = buf[j + k];
                uint64_t v = buf[j + k + s1];
                buf[j + k] = u + v;
                buf[j + k + s1] = u - v;
            }
        }
    }
}
void circuit_evaluation_1(uint64_t *IS){
    for(uint64_t i=0; i<(1ULL << (WORD*2)); i++){
        uint8_t y = i & 0x0F;
        uint8_t x1 = (i >> WORD) & 0x0F;
        uint8_t x = SS1(x1);
        IS[i] = (x==y);
    }
    fht_uint(IS, (WORD*2));
}
void circuit_evaluation_2(uint64_t *IS){
    for(uint64_t i=0; i<(1ULL << (WORD*2)); i++){
        uint8_t y = i & 0x0F;
        uint8_t x2 = (i >> WORD) & 0x0F;
        uint8_t x = SS2(x2);
        IS[i] = (x==y);
    }
    fht_uint(IS, (WORD*2));
}

void circuit_evaluation_3(uint64_t *IS){
    for(uint64_t i=0; i<(1ULL << (WORD*2)); i++){
        uint8_t y = i & 0x0F;
        uint8_t x3 = (i >> WORD) & 0x0F;
        uint8_t x = SS3(x3);

        //compute indicator
        IS[i] = (x==y);
    }
    fht_uint(IS, (WORD*2));
}
void circuit_evaluation_4(uint64_t *D){
    for(uint64_t i=0; i<(1ULL << WORD); i++){
        uint8_t x = SS4(i);
        D[i] = 0;
        uint64_t z = 0UL;
        for(int j=0; j<WORD; j++){
            uint64_t zz = ((x >> j) & 0x01);
            z = z | (zz << SEP4*j);
        }
        D[i] = z;
    }
    fht_uint(D, WORD);
}


void fht_step_1(bitmap *chi, bitmap *A, uint64_t c3, uint64_t *IS){
    uint64_t *C = (uint64_t *)malloc(SIZE2*sizeof(uint64_t));

    for(uint64_t lsb=0UL; lsb< (1UL<<LSB1); lsb++){
        //Read Ciphertexts Parity
        for(uint64_t c1c0=0UL; c1c0<SIZE2; c1c0++){
            uint64_t separated_value = 0UL;
            for(uint64_t msb=0UL; msb < (1UL << MSB1); msb++){
                uint64_t c2 = (msb << LSB1) | lsb;
                uint64_t c3c2 = (c3 << WORD) | c2;
                uint64_t c3c2c1c0 = (c3c2 << (WORD*2)) | c1c0;
                uint64_t c_parity = bt_get(chi, c3c2c1c0) & 0xFFFFFFFFFFFFFFFFUL;
                separated_value = separated_value | (c_parity << (msb*SEP1));
            }
            C[c1c0] = separated_value;
        }
        fht_uint(C, (WORD*2));
        
        //multiply
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = (C[i]*IS[i]);
        }

        fht_uint(C, (WORD*2));
        //shift
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = C[i] >> (2*WORD);
        }
        //Collect Data
        for(uint64_t k1a=0UL; k1a<SIZE2; k1a++){
            uint64_t pos1 = (k1a*SIZE2);
            uint64_t Z = C[k1a];
            for(uint64_t msb=0UL; msb < (1UL << MSB1); msb++){
                uint64_t z = (Z >> (SEP1*msb)) & 0x01;
                if(z == 1){
                    uint64_t c2 = (msb << LSB1) | lsb;
                    uint64_t c3c2 = (c3 << WORD) | c2;
                    uint64_t pos = pos1 + c3c2;
                        bt_set_1(A, pos);
                }
            }
        }
    }
    free(C);
}

void fht_step_2(bitmap *A, bitmap *B, uint64_t k1, uint64_t *IS){
    uint64_t *C = (uint64_t *)malloc(SIZE2*sizeof(uint64_t));

    for(uint64_t lsb=0UL; lsb< (1UL << LSB2); lsb++){
        for(uint64_t c2=0UL; c2<SIZE1; c2++){
            for(uint64_t a=0UL; a<SIZE1; a++){
                uint64_t c2a = (c2 << WORD) | a;
                uint64_t separated_value = 0UL;
                for(uint64_t msb=0UL; msb < (1UL << MSB2); msb++){
                    uint64_t c3 = (msb << LSB2) | lsb;
                    uint64_t c3c2 = (c3 << WORD) | c2; 
                    uint64_t pos =  k1*SIZE3 + a*SIZE2 + c3c2;
                    uint64_t a_parity = bt_get(A, pos) & 0xFFFFFFFFFFFFFFFFUL;
                    separated_value = separated_value | (a_parity << (msb*SEP2));
                }   
                C[c2a] = separated_value;
            }
        }
        fht_uint(C, (WORD*2));
        //multiply
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = (C[i]*IS[i]);
        }
        fht_uint(C, (WORD*2));
        //shift
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = C[i] >> (WORD*2);
        }
        //Collect Data
        for(uint64_t k2=0UL; k2<SIZE1; k2++){
            uint64_t pos1 = k2*SIZE2;
            for(uint64_t b=0UL; b<SIZE1; b++){
                uint64_t k2b = (k2<<WORD) | b;
                uint64_t Z = C[k2b];
                for(uint64_t msb=0UL; msb < (1UL << MSB2); msb++){
                    uint64_t z = (Z >> (SEP2*msb)) & 0x01;
                    if(z == 1){
                        uint64_t c3 = (msb << LSB2) | lsb;
                        uint64_t c3b = (c3<<WORD) | b;
                        uint64_t pos = pos1 + c3b;
                        bt_set_1(B, pos);
                    }
                }
            }
        }
    }
    free(C);
}

void fht_step_3(bitmap *B, bitmap *O, uint64_t *IS){
    uint64_t *C = (uint64_t *)malloc(SIZE2*sizeof(uint64_t));
    for(uint64_t lsb=0UL; lsb< (1UL << LSB2); lsb++){
        for(uint64_t c3b=0UL; c3b<SIZE2; c3b++){
            uint64_t separated_value = 0UL;
            for(uint64_t msb=0UL; msb < (1UL << MSB3); msb++){
                uint64_t k2 = (msb << LSB3) | lsb;
                uint64_t pos =  (k2*SIZE2)+c3b;
                uint64_t b_parity = bt_get(B, pos) & 0xFFFFFFFFFFFFFFFFUL;
                separated_value = separated_value | (b_parity << (msb*SEP3));
            }
            C[c3b] = separated_value;
        }
        fht_uint(C, (WORD*2));
        //multiply
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = (C[i]*IS[i]);
        }
        fht_uint(C, (WORD*2));
        //shift
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = C[i] >> (WORD*2);
        }
        //Collect Data
        for(uint64_t k3=0UL; k3<SIZE1; k3++){
            for(uint64_t c=0UL; c<SIZE1; c++){
                uint64_t k3c = (k3<<WORD) | c;
                uint64_t Z = C[k3c];
                for(uint64_t msb=0UL; msb < (1UL << MSB3); msb++){
                    uint64_t z = (Z >> (SEP3*msb)) & 0x01;
                    if(z == 1){
                        uint64_t k2 = (msb << LSB3) | lsb;
                        uint64_t pos = k2*SIZE2 + k3*SIZE1 + c;
                        bt_set_1(O, pos);
                    }
                }
            }
        }
    }
    free(C);
}
void fht_step_4(bitmap *C, uint8_t *SUM, uint64_t k3, uint64_t k2, uint64_t *D){
    uint64_t *s = (uint64_t *)malloc(SIZE1*sizeof(uint64_t));
    for(uint64_t c=0; c<SIZE1; c++){
        uint64_t pos = (k2*(1UL << (WORD*2))) + (k3*(1UL<<WORD)) + c;
        s[c] = (uint64_t)bt_get(C, pos);
    }
    fht_uint(s, WORD);

    //Multiply
    for(uint64_t i=0UL; i<SIZE1; i++){
        s[i] = s[i]*D[i];
    }
    fht_uint(s, WORD);
    for(uint64_t i=0UL; i<SIZE1; i++){
        s[i] = s[i] >> WORD;
    }

    for(uint64_t i=0; i<SIZE1; i++){
        uint64_t z = s[i];
        for(int j=0; j<WORD; j++){
            uint8_t zz = (z & (1UL << SEP4*j)) >> (SEP4*j);
            SUM[i] = SUM[i] | (zz << j);
        }
    }
    free(s);
}
