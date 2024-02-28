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
void circuit_evaluation_1(double *IS){
    for(uint64_t i=0; i<(1ULL << (WORD*2)); i++){
        uint8_t y = i & 0xFF;
        uint8_t x1 = (i >> WORD) & 0xFF;
        uint8_t x = SS1(x1);
        IS[i] =(double)(x==y);
    }
    fht_double(IS, (WORD*2));
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
        for(int j=0; j<WORD; j++){
            uint64_t zz = ((x >> j) & 0x01);
            D[SIZE1*j + i] = zz;
        }
    }
    for(int j=0; j<WORD; j++) {
        fht_uint(D + SIZE1*j, WORD);
    }
}



void fht_step_1(bitmap *chi, bitmap *A, uint64_t c3, double *IS){
    double *C = (double *)malloc(SIZE2*sizeof(double));
    uint64_t *H = (uint64_t *)malloc(SIZE2*sizeof(uint64_t));

    for(uint64_t c2=0UL; c2<SIZE1; c2++){
        //Read Ciphertexts Parity
        for(uint64_t c1c0=0UL; c1c0<SIZE2; c1c0++){
            uint64_t c3c2 = (c3 << WORD) | c2;
            uint64_t c3c2c1c0 = (c3c2 << (WORD*2)) | c1c0;
            C[c1c0] = bt_get(chi, c3c2c1c0) & 0xFFFFFFFFFFFFFFFFUL;
        }
        fht_double(C, (WORD*2));

        //multiply
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = (C[i]*IS[i]);
        }

        fht_double(C, (WORD*2));
        //shift
        for(uint64_t i=0UL; i<SIZE2; i++){
            C[i] = C[i]/pow(2, (WORD*2));
            H[i] = round(C[i]);
        }

        //Collect Data
        for(uint64_t k1a=0UL; k1a<SIZE2; k1a++){
            uint64_t pos1 = (k1a*SIZE2);
            uint64_t Z = H[k1a];
            uint64_t z = Z & 0x01;
            if(z == 1){
                uint64_t c3c2 = (c3 << WORD) | c2;
                uint64_t pos = pos1 + c3c2;
                bt_set_1(A, pos);
            }
        }
    }
    free(C);
    free(H);
}

void fht_step_2(bitmap *A, bitmap *B, uint64_t k1, double *IS){
    uint64_t *H = (uint64_t *)malloc(SIZE2*sizeof(uint64_t));
    double *C = (double *)malloc(SIZE2*sizeof(double));

    for(uint64_t c3=0UL; c3< SIZE1; c3++){
        for(uint64_t c2=0UL; c2<SIZE1; c2++){
            for(uint64_t a=0UL; a<SIZE1; a++){
                uint64_t c2a = (c2 << WORD) | a;
                uint64_t c3c2 = (c3 << WORD) | c2;
                uint64_t pos =  k1*SIZE3 + a*SIZE2 + c3c2;
                C[c2a] = bt_get(A, pos) & 0xFFFFFFFFFFFFFFFFUL;
            }
        }
        fht_double(C, (WORD*2));
        //multiply
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = (C[i]*IS[i]);
        }
        fht_double(C, (WORD*2));
        //shift
        for(uint64_t i=0UL; i<SIZE2; i++){
            C[i] = C[i]/pow(2, (WORD*2));
            H[i] = round(C[i]);
        }


        //Collect Data
        for(uint64_t k2=0UL; k2<SIZE1; k2++){
            uint64_t pos1 = k2*SIZE2;
            for(uint64_t b=0UL; b<SIZE1; b++){
                uint64_t k2b = (k2<<WORD) | b;
                uint64_t Z = H[k2b];
                uint64_t z = Z & 0x01;
                if(z == 1){
                    uint64_t c3b = (c3<<WORD) | b;
                    uint64_t pos = pos1 + c3b;
                    bt_set_1(B, pos);
                }
            }
        }
    }
    free(C);
    free(H);
}


void fht_step_3(bitmap *B, bitmap *O, double *IS){
    double *C = (double *)malloc(SIZE2*sizeof(double));
    uint64_t *H = (uint64_t *)malloc(SIZE2*sizeof(uint64_t));

    for(uint64_t k2=0UL; k2< SIZE1; k2++){
        for(uint64_t c3b=0UL; c3b<SIZE2; c3b++){
            uint64_t pos =  (k2*SIZE2)+c3b;
            C[c3b] = bt_get(B, pos) & 0xFFFFFFFFFFFFFFFFUL;
        }
        fht_double(C, (WORD*2));
        //multiply
        for(uint64_t i=0; i<SIZE2; i++){
            C[i] = (C[i]*IS[i]);
        }
        fht_double(C, (WORD*2));
        //shift
        for(uint64_t i=0UL; i<SIZE2; i++){
            C[i] = C[i]/pow(2, (WORD*2));
            H[i] = round(C[i]);
        }
        //Collect Data
        for(uint64_t k3=0UL; k3<SIZE1; k3++){
            for(uint64_t c=0UL; c<SIZE1; c++){
                uint64_t k3c = (k3<<WORD) | c;
                uint64_t Z = H[k3c];
                uint64_t z = Z & 0x01;
                if(z == 1){
                    uint64_t pos = k2*SIZE2 + k3*SIZE1 + c;
                    bt_set_1(O, pos);
                }
            }
        }
    }
    free(C);
}

void fht_step_4(bitmap *C, uint8_t *SUM, uint64_t k3, uint64_t k2, uint64_t *D){
    uint64_t *s = (uint64_t *)malloc(SIZE1*sizeof(uint64_t));
    uint64_t *H = (uint64_t *)malloc(SIZE1*sizeof(uint64_t));

    for(uint64_t c=0; c<SIZE1; c++){
        uint64_t pos = (k2*(1UL << (WORD*2))) + (k3*(1UL<<WORD)) + c;
        s[c] = (uint64_t)bt_get(C, pos);
    }
    fht_uint(s, WORD);

    for(int j=0; j<WORD; j++){
        //Multiply
        for(uint64_t i=0UL; i<SIZE1; i++){
            H[i] = s[i]*D[SIZE1*j + i];
        }
        fht_uint(H, WORD);
        for(uint64_t i=0UL; i<SIZE1; i++){
            H[i] = H[i] >> WORD;
        }

        for(uint64_t i=0; i<SIZE1; i++){
            uint64_t zz = (H[i] & 0x01);
            SUM[i] = SUM[i] | (zz << j);
        }
    }
    free(s);
    free(H);
}

