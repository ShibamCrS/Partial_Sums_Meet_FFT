#include "FHT_computations.h"
#include "../../FFHT/fht_header_only.h"
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

void circuit_evaluation_1(double **IS){
    for(int a=0; a<(1UL<<WORD); a++){
        for(uint64_t i=0; i<(1ULL << (WORD*2)); i++){
            uint8_t x = SS01(i);
            //compute indicator
            IS[a][i] = (double)(x==a);
        }
        fht_double(IS[a], (WORD*2));
    }
}
void circuit_evaluation_2(double *IS){
    for(uint64_t i=0; i<(1ULL << (WORD*2)); i++){
        uint8_t y = i & 0xFF; 
        uint8_t x2 = (i >> WORD) & 0xFF;
        uint8_t x = SS2(x2);

        IS[i] = (double)(x==y);
    }
    fht_double(IS, (WORD*2));
}

void circuit_evaluation_3(double *IS){
    for(uint64_t i=0; i<(1ULL << (WORD*2)); i++){
        uint8_t y = i & 0xFF;
        uint8_t x3 = (i >> WORD) & 0xFF;
        uint8_t x = SS3(x3);

        //compute indicator
        IS[i] = (double)(x==y);
    }
    fht_double(IS, (WORD*2));
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




void fht_step_1(bitmap *c, bitmap *A, uint64_t c3, double *IS_A){
    uint64_t size = 1ULL << (WORD*2);
    uint64_t pos1 = (c3*(1UL<<(WORD*2)));
    double *C = (double *)malloc(size*sizeof(double));
    double *IS = (double *)malloc(size*sizeof(double));
    uint64_t *H = (uint64_t *)malloc(size*sizeof(uint64_t));

    for(uint64_t lsb=0UL; lsb< (1UL << LSB1); lsb++){
        //Read Ciphertexts Parity
        for(uint64_t c1c0=0UL; c1c0<size; c1c0++){
            uint64_t separated_value = 0UL;
            for(uint64_t msb=0UL; msb < (1UL << MSB1); msb++){
                uint64_t c2 = (msb << LSB1) | lsb;
                uint64_t c3c2 = (c3 << WORD) | c2;
                uint64_t c3c2c1c0 = (c3c2 << (WORD*2)) | c1c0;
                uint64_t c_parity = bt_get(c, c3c2c1c0) & 0xFFFFFFFFFFFFFFFFUL;
                separated_value = separated_value | (c_parity << (msb*SEP1));
            }
            C[c1c0] = (double)separated_value;
        }
        fht_double(C, (WORD*2));

        for(uint64_t a=0UL; a<(1UL<<WORD); a++){
            memcpy(IS, IS_A+((1UL<<(2*WORD))*a), size*sizeof(double));
            //multiply
            for(uint64_t i=0; i<size; i++){
                IS[i] = (C[i]*IS[i]);
            }
            fht_double(IS, (WORD*2));

            //shift
            for(uint64_t i=0UL; i<size; i++){
                IS[i] = IS[i]/pow(2, (WORD*2)); 
                H[i] = round(IS[i]);
            }
            

            //Collect Data
            
            for(uint64_t k1k0=0UL; k1k0<(1UL<<(2*WORD)); k1k0++){
                uint64_t pos2 = (k1k0*(1UL<<(WORD*3)));
                uint64_t Z = H[k1k0];
                for(uint64_t msb=0UL; msb < (1UL << MSB1); msb++){
                    uint64_t z = (Z >> (SEP1*msb)) & 0x01;
                    if(z == 1){
                        uint64_t c2 = (msb << LSB1) | lsb;
                        uint64_t c2a = (c2 << WORD) | a;
                        uint64_t pos = pos2 + pos1 + c2a;
                        bt_set_1(A, pos);
                    }
                }
            }
        }
    }
    free(IS);
    free(C);
    free(H);
}
void fht_step_2(bitmap *A, bitmap *B, uint64_t k1k0, double *IS){
    uint64_t size = 1ULL << (WORD*2);
    double *C = (double *)malloc(size*sizeof(double));
    uint64_t *H = (uint64_t *)malloc(size*sizeof(uint64_t));

    for(uint64_t lsb=0UL; lsb< (1UL << LSB2); lsb++){
        for(uint64_t c2a=0UL; c2a<size; c2a++){
            uint64_t separated_value = 0UL;
            for(uint64_t msb=0UL; msb < (1UL << MSB2); msb++){
                uint64_t c3 = (msb << LSB2) | lsb;
                uint64_t c3c2a = (c3 << (2*WORD)) | c2a;
                uint64_t pos =  (k1k0*(1UL<<(WORD*3)))+c3c2a;
                uint64_t a_parity = bt_get(A, pos) & 0xFFFFFFFFFFFFFFFFUL;
                separated_value = separated_value | (a_parity << (msb*SEP2));
            }
            C[c2a] = (double)separated_value;
        }
        fht_double(C, (WORD*2));
        //multiply
        for(uint64_t i=0; i<size; i++){
            C[i] = (C[i]*IS[i]);
        }
        fht_double(C, (WORD*2));
        //shift
        for(uint64_t i=0UL; i<size; i++){
            C[i] = C[i]/pow(2, (WORD*2)); 
            H[i] = round(C[i]);
        }
        
        //Collect Data
        for(uint64_t k2=0UL; k2<(1UL<<WORD); k2++){
            uint64_t pos1 =(k2*(1UL<<(WORD*2))); 
            for(uint64_t b=0UL; b<(1UL<<WORD); b++){
                uint64_t k2b = (k2<<WORD) | b;
                uint64_t Z = H[k2b];
                for(uint64_t msb=0UL; msb < (1UL << MSB2); msb++){
                    uint64_t z = (Z >> (SEP2*msb)) & 0x01;
                    if(z == 1){
                        uint64_t c3 = (msb << LSB2) | lsb;
                        uint64_t c3b = (c3<<WORD) | b;
                        uint64_t pos = pos1 +c3b;
                        bt_set_1(B, pos);
                    }
                }
            }
        }
    }
    free(H);
    free(C);
}
void fht_step_3(bitmap *B, bitmap *O, double *IS){
    uint64_t size = 1ULL << (WORD*2);
    double *C = (double *)malloc(size*sizeof(double));
    uint64_t *H = (uint64_t *)malloc(size*sizeof(uint64_t));

    for(uint64_t lsb=0UL; lsb< (1UL << LSB2); lsb++){
        for(uint64_t c3b=0UL; c3b<size; c3b++){
            uint64_t separated_value = 0UL;
            for(uint64_t msb=0UL; msb < (1UL << MSB3); msb++){
                uint64_t k2 = (msb << LSB3) | lsb;
                uint64_t pos =  (k2*(1UL<<(WORD*2)))+c3b;
                uint64_t b_parity = bt_get(B, pos) & 0xFFFFFFFFFFFFFFFFUL;
                separated_value = separated_value | (b_parity << (msb*SEP3));
            }
            C[c3b] = (double)separated_value;
        }
        fht_double(C, (WORD*2));
        //multiply
        for(uint64_t i=0; i<size; i++){
            C[i] = (C[i]*IS[i]);
        }
        fht_double(C, (WORD*2));
        //shift
        for(uint64_t i=0UL; i<size; i++){
            C[i] = C[i]/pow(2, (WORD*2)); 
            H[i] = round(C[i]);
        }

        //Collect Data
        for(uint64_t k3=0UL; k3<(1UL<<WORD); k3++){
            uint64_t pos1 = (k3*(1UL<<WORD));
            for(uint64_t c=0UL; c<(1UL<<WORD); c++){
                uint64_t k3c = (k3<<WORD) | c;
                uint64_t Z = H[k3c];
                for(uint64_t msb=0UL; msb < (1UL << MSB3); msb++){
                    uint64_t z = (Z >> (SEP3*msb)) & 0x01;
                    if(z == 1){
                        uint64_t k2 = (msb << LSB3) | lsb;
                        uint64_t pos = (k2*(1UL << (WORD*2))) + pos1 + c;
                        bt_set_1(O, pos);
                    }
                }
            }
        }
    }
    free(H);
    free(C);
}

void fht_step_4(bitmap *C, uint8_t *SUM, uint64_t k3, uint64_t k2, uint64_t *D){
    uint64_t size = 1ULL << WORD;

    uint64_t *s = (uint64_t *)malloc(size*sizeof(uint64_t));
    for(uint64_t c=0; c<size; c++){
        uint64_t pos = (k2*(1UL << (WORD*2))) + (k3*(1UL<<WORD)) + c;
        s[c] = (uint64_t)bt_get(C, pos);
    }
    fht_uint(s, WORD);

    //Multiply
    for(uint64_t i=0UL; i<size; i++){
        s[i] = s[i]*D[i];
    }
    fht_uint(s, WORD);
    for(uint64_t i=0UL; i<size; i++){
        s[i] = s[i] >> WORD;
    }

    for(uint64_t i=0; i<(1UL << WORD); i++){
        uint64_t z = s[i];
        for(int j=0; j<WORD; j++){
            uint8_t zz = (z & (1UL << SEP4*j)) >> (SEP4*j);
            SUM[i] = SUM[i] | (zz << j);
        }
    }
    free(s);
}

