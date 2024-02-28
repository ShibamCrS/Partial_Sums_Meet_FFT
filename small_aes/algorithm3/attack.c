#include "attack.h"

void* FHT_over_ranges(void *args1){
    FHTArgs *args = (FHTArgs*) args1;
    
    for(uint64_t c3=args->start; c3<args->end;  c3+= 1){
        fht_step_1(args->ciphertexts, args->A, c3, args->IS);
    }
    pthread_exit(NULL);
}
void using_thraed(bitmap *c, bitmap *A){
    uint64_t threads = THREADS;
    pthread_t thread_ids[threads];
    FHTArgs  thread_args[threads];

    uint64_t cube_size = (1ULL << WORD);
    uint64_t data_in_each_thread = (cube_size / threads);
    uint64_t ciphertext_size_uint64 = (1UL << (4*WORD)) >> 6;

    uint64_t **IS = (uint64_t **)malloc((1ULL<<WORD)*sizeof(uint64_t *));
    for(uint64_t i=0UL; i< (1ULL<<WORD); i++){
        IS[i] = (uint64_t *)malloc((1ULL << (WORD*2))*sizeof(uint64_t));
    }   
    circuit_evaluation_1(IS);
    
    uint64_t *temp = (uint64_t *)malloc((1ULL<<(3*WORD)) * sizeof(uint64_t));
    for(uint64_t i=0UL; i< (1ULL<<WORD); i++){
        memcpy(temp + ((1UL<<(2*WORD))*i), IS[i], (1UL<<(2*WORD))*sizeof(uint64_t));
        free(IS[i]);
    } 
    free(IS);

    for(int i=0; i<threads; i++){
        thread_args[i].A = A;
        thread_args[i].ciphertexts = c;
        thread_args[i].start = i * data_in_each_thread;
        thread_args[i].end = (i+1) * data_in_each_thread;
        thread_args[i].IS = temp;
    }   
    for(int i=0; i < threads; i++){
        pthread_create(thread_ids + i, NULL, FHT_over_ranges, (void*) (thread_args + i));
    }   
    for(int i=0; i < threads; i++){
        pthread_join(thread_ids[i], NULL);
        /* bt_destroy(thread_args[i].ciphertexts); */
        /* free(thread_args[i].IS); */
    }   
    free(temp);
}

/***********************************************************************
 *This function computes first step of the attack, i.e., preparation 
 *of the array A_1 (line 2 - 5) using multiple threads. Each thread 
 * considers a part of k0 keys 
 * ********************************************************************/
void compute_first_step(bitmap *A, int dataset_index){
    /* Reading Data*/
    bitmap *ciphertext;
    char fname[256];
    sprintf(fname, DATA_PATH_6, dataset_index);
    ciphertext = bt_load(fname);

    using_thraed(ciphertext, A);

    bt_destroy(ciphertext);
}
