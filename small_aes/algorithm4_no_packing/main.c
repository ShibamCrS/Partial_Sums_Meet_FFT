#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include<unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../utility/utility.h"
#include "../utility/bitmap.h"
#include "../utility/smallAES.h"

#include "FHT_computations.h"
#include "key_guess.h"
#include "guessK0.h"

/**********************Generating Random Data*********************/
#include "../utility/xoshiro256plusplus.h"
uint64_t SEED[4];
void generate_random_nibble(uint8_t *random_data){
    uint64_t random64 = xx_next(SEED);
    for(int i=0; i<16; i++){
        random_data[i] = (random64 >> (4*i)) & 0x0F;
    }
}
void generate_random_round_keys(uint8_t **rk, int rounds){
    for(int i=0; i<rounds+1; i++){
        rk[i] = (uint8_t *)malloc(16*sizeof(uint8_t));
        generate_random_nibble(rk[i]);
    }
}
/******************************************************************/


/********************************************************************************
 *This Function considers a set of all possible keys and run Algorithm 4. If the
 *integral property does not satisfy for some key, it marked that key as a wrong key.
 *We compute for k0 in parallel (loop at line 2 of Algorithm 4)
 *******************************************************************************
 */
uint64_t  using_thread(){
    struct timespec start, end;
    double time_meter;

    printf("Precomputations, Reading Data etc. \n");
    clock_gettime(CLOCK_MONOTONIC, &start);
    bitmap **ciphertexts = (bitmap **)malloc(DATA_SETS*sizeof(bitmap*));
    for(uint64_t j=0; j<DATA_SETS; j++){
        char fname[256];
        sprintf(fname, DATA_PATH_6, j);
        ciphertexts[j] = bt_load(fname);
    }

    uint64_t IS_A[SIZE2];
    circuit_evaluation_1(IS_A);
    uint64_t IS_B[SIZE2];
    circuit_evaluation_2(IS_B);
    uint64_t IS_C[SIZE2];
    circuit_evaluation_3(IS_C);
    uint64_t IS_D[SIZE1*WORD];
    circuit_evaluation_4(IS_D);
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("Preprocessing DONE --- Elapsed time %f\n", time_meter);

    uint64_t threads = THREADS;
    pthread_t thread_ids[threads];
    Args  thread_args[threads];

    uint64_t data = SIZE1;
    uint64_t data_per_thread = SIZE1/threads;

    for(int i=0; i<threads; i++){
        thread_args[i].A = bt_init(SIZE4);
        thread_args[i].B = bt_init(SIZE3);
        thread_args[i].C = bt_init(SIZE3);
        thread_args[i].RK = bt_init(SIZE4);
 
        thread_args[i].IS_A = IS_A;
        thread_args[i].IS_B = IS_B; 
        thread_args[i].IS_C = IS_C;
        thread_args[i].IS_D = IS_D;

        thread_args[i].ciphertexts = ciphertexts;
        thread_args[i].start = i * data_per_thread;
        thread_args[i].end = (i+1) * data_per_thread;
        thread_args[i].found_key = 0;
        thread_args[i].Key = 0;
    }   

    for(int i=0; i < threads; i++){
        pthread_create(thread_ids + i, NULL, guess_over_k0, (void*) (thread_args + i));
    }
    for(int i=0; i < threads; i++){
        pthread_join(thread_ids[i], NULL);
    }
    for(int j=0; j<DATA_SETS; j++){
        bt_destroy(ciphertexts[j]);
    }
    free(ciphertexts);

    for(int i=0; i<threads; i++){
        if(thread_args[i].found_key==1){
            return (thread_args[i].Key);
        }
    }
}
void find_key(){
    struct timespec start, end;
    double time_meter;
    uint64_t k0k1k2k3k4 = using_thread();

    printf("Key = %lx\n",k0k1k2k3k4);
    printf("\n\n\n ATTACK COMPLETED \n");
    printf("------Recoverd Keys at Nibble k6_0, k6_13, k6_10, k6_7, k5_0---------\n");
    uint8_t key_6[16];
    memset(key_6, 0x00, 16);
    key_6[0]   = (k0k1k2k3k4 >> (WORD*4)) & 0x0F;
    key_6[13]  = (k0k1k2k3k4 >> (WORD*3)) & 0x0F;
    key_6[10]  = (k0k1k2k3k4 >> (WORD*2)) & 0x0F;
    key_6[7]   = (k0k1k2k3k4 >> (WORD*1)) & 0x0F;
    uint8_t k4 = k0k1k2k3k4 & 0x0F;
    printf("key_5_prime[0] : %02lX\n", k4);
    printf( "key_6 : ");
    printreg(key_6, 16);
    printf("-------------------------------------------------------------------\n");

    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("Full Calculation Done --- Elapsed time %f\n", time_meter);
}
void save_data(uint8_t *p, char *fname, uint8_t **rk){
    bitmap *ciphertexts = bt_init(SIZE4);
    bt_zero(ciphertexts);
    data_collection_phase_threaded(ciphertexts, p, rk);
    //data_collection_phase(ciphertexts, p, rk);
    bt_save(ciphertexts, fname);
    /* bt_print(ciphertexts); */
	//scanf("?");
    bt_destroy(ciphertexts);
    //printf("\n=============Saved=============== \n");
}

void oracle_6r(uint8_t **rk){
    struct timespec start, end;
    double time_meter;
    char fname[256];
    printf("Encryption process is running...\n");
    for(int i=0; i<DATA_SETS; i++){
        sprintf(fname, DATA_PATH_6, i);
        clock_gettime(CLOCK_MONOTONIC, &start);
        uint8_t p[16];
        generate_random_nibble(p);
        printf("Plaintext : ");
        printreg(p, 16);
        save_data(p, fname, rk);
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_meter = ((double)(end.tv_sec - start.tv_sec));
        printf("Elapsed time encryption phase %d: %f s\n",i, time_meter);
    }
}

void attack(){
    struct timespec start, end;
    double time_meter;
    
    uint8_t *rk[ROUNDS+1];
    generate_random_round_keys(rk, ROUNDS);
    for(int i=0; i<ROUNDS+1; i++){
        printf("key_%d : ",i);
        printreg(rk[i], 16);
    }
    /*
    for(uint8_t i=0; i<16; i++){
        rk[5][i] = i;
    }
    */
    //rk5_prime = MC^(-1)(rk5)
    uint8_t rk5_prime[16];
    memcpy(rk5_prime, rk[5], 16);
    inverseMixColumn(rk5_prime);
    
    oracle_6r(rk);
    printf("\n=============Saved Data=============== \n");
    printf("Original: \n");
    printf("key_5_prime[0] : %02X\n",(unsigned char)rk5_prime[0]);
    printf("key_6 : ");
    printreg(rk[ROUNDS], 16);
    find_key(rk[ROUNDS][0]);
    printf("Original: \n");
    printf("key_5_prime[0] : %02X\n",(unsigned char)rk5_prime[0]);
    printf("key_6 : ");
    printreg(rk[ROUNDS], 16);
    for(int i=0; i<ROUNDS+1; i++){
        free(rk[i]);
    }
}
int main(){
    xx_initialize(SEED);
    printf("SEED: ");
    printreg(SEED, 32);
    attack();
    return 0;
}
