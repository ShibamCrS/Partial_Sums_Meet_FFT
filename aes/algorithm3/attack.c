#include "attack.h"

void* FHT_over_ranges(void *args1){
    FHTArgs *args = (FHTArgs*) args1;
    
    for(uint64_t c3=args->start; c3<args->end;  c3+= 1){
        fht_step_1(args->ciphertexts, args->A, c3, args->IS);
    }
    pthread_exit(NULL);
}

void using_thraed(char *fname, bitmap *A){
    struct timespec start, end;
    double time_meter;
    clock_gettime(CLOCK_MONOTONIC, &start);

    printf("Preparing Threads, Reading Data etc.\n");
    bitmap *ciphertexts = bt_load(fname);
    uint64_t threads = THREADS;
    pthread_t thread_ids[threads];
    FHTArgs  thread_args[threads];

    uint64_t cube_size = (1ULL << WORD);
    uint64_t data_in_each_thread = (cube_size / threads);
    
    //Pre-calculation FFT(I( a = S0(X0) + S1(X1) ) )
    double **IS = (double **)malloc((1ULL<<WORD)*sizeof(double *));
    for(uint64_t i=0UL; i< (1ULL<<WORD); i++){
        IS[i] = (double *)malloc((1ULL << (WORD*2))*sizeof(double));
    }
    circuit_evaluation_1(IS);
 
    double *temp = (double *)malloc((1ULL<<(3*WORD))*sizeof(double));
    for(uint64_t i=0UL; i< (1ULL<<WORD); i++){
        memcpy(temp + ((1UL<<(2*WORD))*i), IS[i], (1UL<<(2*WORD))*sizeof(double));
        free(IS[i]);
    }   
    free(IS);

    for(int i=0; i<threads; i++){
        thread_args[i].A = A;
        thread_args[i].ciphertexts = ciphertexts;
        thread_args[i].start = i * data_in_each_thread;
        thread_args[i].end = (i+1) * data_in_each_thread;
        thread_args[i].IS = temp;
        /* (double *)malloc((1ULL << (WORD*3))*sizeof(double)); */
        /* memcpy(thread_args[i].IS, temp, (1ULL << (WORD*3))*sizeof(double)); */
    }
    printf("Threads Prepared\n");
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("Preprocessing DONE --- Elapsed time %f\n", time_meter);

    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i=0; i < threads; i++){
        pthread_create(thread_ids + i, NULL, FHT_over_ranges, (void*) (thread_args + i));
    }
    for(int i=0; i < threads; i++){
        pthread_join(thread_ids[i], NULL);
    }
    bt_destroy(ciphertexts);
    free(temp);
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("First Step DONE --- Elapsed time %f\n", time_meter);
}
/***********************************************************************
 *This function computes first step of the attack, i.e., preparation 
 *of the array A_1 (line 2 - 5) using multiple threads. Each thread 
 * considers a part of k0 keys 
 * ********************************************************************/
void compute_first_step(bitmap *A, uint64_t dataset_index){
    bitmap *ciphertext;
    char fname[256];
    sprintf(fname, DATA_PATH_6, dataset_index);
    /* ciphertext = bt_load(fname); */

    using_thraed(fname, A);
    /* bt_destroy(ciphertext); */
}

