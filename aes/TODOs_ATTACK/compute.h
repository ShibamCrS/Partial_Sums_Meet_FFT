void fht_double_n_1(double *X, uint64_t start_index, uint64_t n){
    uint64_t size_1 = (1UL << (n-2));
    fht_double(X + start_index, n-2);
    fht_double(X + start_index + size_1, n-2);

    for(uint64_t i=0UL; i<size_1; i++){
        double temp1 = X[i + start_index];
        double temp2 = X[i + start_index + size_1];
        X[i + start_index]  = temp1 + temp2;
        X[i + start_index + size_1] = temp1 - temp2;
    }
}
void fht_double_n(double *X, uint64_t n){
    uint64_t size_1 = (1UL << (n-1));
    fht_double_n_1(X, 0UL, n);
    fht_double_n_1(X, size_1, n);

    for(uint64_t i=0UL; i<size_1; i++){
        double temp1 = X[i];
        double temp2 = X[i+size_1];
        X[i] = temp1 + temp2;
        X[i+size_1] = temp1 - temp2;
    }
}
void compute_FK4(uint8_t *FK4, uint64_t k4){
     for(uint64_t i=0; i<(1ULL << LOG_DATA); i++){
        FK4[i] = evaluate_F(i, k4);
    }
}
void mat_vec(double *C, double *C1){
    for(uint64_t i=0; i<(1UL<<LOG_DATA); i++){
        double temp = C[i]*C1[i];
        temp = temp/ pow(2,LOG_DATA);
        C1[i] = (double)temp;
    }
    fht_double_n(C1, LOG_DATA);

}
void fht_step(bitmap *c, uint8_t *sum, uint64_t k4, uint64_t data_index){
    uint64_t size = (1UL << LOG_DATA);
    double *C = (double *)malloc(size*sizeof(double));
    double *C1 = (double *)malloc(size*sizeof(double));
    for(uint64_t i=0UL; i<size; i++){
            C[i] = (double)bt_get(c, i);
    }
    fht_double_n(C, LOG_DATA);

    uint8_t *FK4 = (uint8_t *)malloc(size*sizeof(uint8_t));
    compute_FK4(FK4, k4);
    for(uint64_t j=0UL; j<WORD; j++){
        for(uint64_t i=0UL; i<size; i++){
            C1[i] = (double)( ((FK4[i] >> j) & 0x01) );
        }
        fht_double_n(C1, LOG_DATA);
        mat_vec(C, C1);

        for(uint64_t i=0UL; i<size; i++){
            int64_t z =  ((int64_t)C1[i])  & 0x01;
            sum[(data_index * (1UL<<LOG_DATA)) + i] ^= (z << j);
        }
    }
    free(FK4);
    free(C);
    free(C1);
}

