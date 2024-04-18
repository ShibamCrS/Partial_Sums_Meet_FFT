#include "attack.h"

void fht_float_n(float *buf, uint64_t n) {
    uint64_t size = 1UL << n;
    for (uint64_t i = 0; i < n; ++i) {
        uint64_t s1 = 1UL << i;
        uint64_t s2 = s1 << 1;
        for (uint64_t j = 0UL; j < size; j += s2) {
            for (uint64_t k = 0; k < s1; ++k) {
                float u = buf[j + k];
                float v = buf[j + k + s1];
                buf[j + k] = u + v;
                buf[j + k + s1] = u - v;
            }
        }
    }
}

void compute_FK4(uint8_t *FK4, uint64_t k4){
     for(uint64_t i=0; i<(1ULL << LOG_DATA); i++){
        FK4[i] = evaluate_F(i, k4);
    }
}
void mat_vec(float *C, float *C1){
    for(uint64_t i=0; i<(1UL<<LOG_DATA); i++){
        double temp = C[i]*C1[i];
        temp = temp/ pow(2,LOG_DATA);
        C1[i] = (float)temp;
    }
    fht_float_n(C1, LOG_DATA);

}
void fht_step(bitmap *c, uint8_t *sum, uint64_t k4, uint64_t data_index){
    uint64_t size = (1UL << LOG_DATA);
    float C[size];
    float C1[size];
    for(uint64_t i=0UL; i<size; i++){
            C[i] = (float)bt_get(c, i); 
    }
    fht_float_n(C, LOG_DATA);

    uint8_t FK4[(1UL << LOG_DATA)];
    compute_FK4(FK4, k4);
    for(uint64_t j=0UL; j<WORD; j++){
        for(uint64_t i=0UL; i<size; i++){
            C1[i] = (float)( ((FK4[i] >> j) & 0x01) );
        }
        fht_float_n(C1, LOG_DATA);
        mat_vec(C, C1);
        
        for(uint64_t i=0UL; i<size; i++){
            int64_t z =  ((int64_t)C1[i])  & 0x01;
            sum[(data_index * (1UL<<LOG_DATA)) + i] ^= (z << j);
        }
    }
}

