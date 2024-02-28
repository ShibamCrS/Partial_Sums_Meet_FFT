#include "attack.h"
#include "../../FFHT/fht_header_only.h"

int STOP = 0;
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

void *key_guess_over_ranges(void *args1){
    struct timespec start, end;
    double time_meter;
    PSArgs *args = (PSArgs *) args1;

    #ifdef FIXK0
    for(uint64_t k4=0x1C; k4<(0x1C + 1); k4++){
    #else
    for(uint64_t k4=args->start; k4<args->end; k4++){
    #endif
        if(STOP == 1){
            break;
        }
        uint64_t size_sum = (1ULL << LOG_DATA )*DATA_SETS;
        /* printf("Size Sum = %lld(%lf)\n", size_sum, log((double)size_sum)/log(2)); */
        uint8_t *sum = (uint8_t *)malloc(size_sum*sizeof(uint8_t));
        memset(sum, 0x00, (1ULL << LOG_DATA )*DATA_SETS);
        int valid_k4 = 0;
        printf("\n=============Start K4 %d=============== \n",k4);
        clock_gettime(CLOCK_MONOTONIC, &start);
        for(uint64_t i=0UL; i<DATA_SETS; i++){
            fht_step(args->ciphertexts[i], sum, k4, i);
        }
        printf("\n=============Done Pre Calculation...Now searching K =============== \n");
        for(uint64_t k=0UL; k<(1ULL << LOG_DATA); k++){
            int final_flag = 1;
            for(uint64_t i=0ULL; i<DATA_SETS; i++){
                if(sum[i*(1ULL << LOG_DATA) + k] != 0){
                    final_flag = 0;
                    break;
                }
            }
            if(final_flag == 1){
                valid_k4 = 1;
                uint64_t k4k3k2k1k0 = k4;
                k4k3k2k1k0 = (k4k3k2k1k0 << (4*WORD)) | k;
                args->recovered_key = k4k3k2k1k0;
                args->found_key = 1;
                printf("Key inside thread = %lx\n",k4k3k2k1k0);
                STOP = 1;
                break;
            }
        }
        free(sum);
        if(valid_k4 == 1){
            break;
        }
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_meter = ((double)(end.tv_sec - start.tv_sec));
        printf("Elapsed time in one K=%ld: %f s\n", k4, time_meter);
   
        printf("\n=============Done With  K4 %d=============== \n",k4);
    }
    pthread_exit(NULL);
}
uint64_t Key_Guess_Using_Thread(bitmap **c){
    uint64_t threads = THREADS_TODOS_ATTACK;
    pthread_t thread_ids[threads];
    PSArgs  thread_args[threads];

    uint64_t cube_size = (1ULL << WORD);
    uint64_t data_in_each_thread = (cube_size / threads);

    for(int i=0; i<threads; i++){
        thread_args[i].ciphertexts = c;
        thread_args[i].found_key = 0;
        thread_args[i].start = i * data_in_each_thread;
        thread_args[i].end = (i+1) * data_in_each_thread;
    }

    for(int i=0; i < threads; i++){
        pthread_create(thread_ids + i,NULL,key_guess_over_ranges,(void*)(thread_args + i));
    }
    for(int i=0; i < threads; i++){
        pthread_join(thread_ids[i], NULL);
    }
    for(int i=0; i<threads; i++){
        if(thread_args[i].found_key==1){
            return (thread_args[i].recovered_key);
        }
    }
}
