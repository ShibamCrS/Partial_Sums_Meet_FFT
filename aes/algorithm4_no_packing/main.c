#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../utility/utility.h"
#include "../utility/bitmap.h"
#include "../utility/aes.h"

#include "FHT_computations.h"
#include "key_guess.h"
#include "guessK0.h"

/**********************Generating Random Data*********************/
#include "../utility/xoshiro256plusplus.h"
uint64_t SEED[4];
void generate_random(uint8_t *random_data, int nrof_byte){
    int required_call = nrof_byte/8 + 1;
    uint64_t *random64 = (uint64_t *)malloc(required_call*sizeof(uint64_t));
    for(int i=0; i<required_call; i++){
        random64[i] = xx_next(SEED);
    }
    memcpy(random_data, random64, nrof_byte);
    free(random64);
}
/******************************************************************/

/********************************************************************************
 *This function examines a collection of all potential keys and executes Algorithm 4.
 If the integral property is not met for a certain key, that key is identified as
 incorrect. We calculate for k0 concurrently using multiple threads (loop at line 2 of
 Algorithm 4).
 *******************************************************************/
uint64_t  using_thread(){
    struct timespec start, end;
    double time_meter;
    
    clock_gettime(CLOCK_MONOTONIC, &start);
    printf("Precomputations \n");
    bitmap **ciphertexts = (bitmap **)malloc(DATA_SETS*sizeof(bitmap*));
    for(uint64_t j=0; j<DATA_SETS; j++){
        char fname[256];
        sprintf(fname, DATA_PATH_6, j); 
        ciphertexts[j] = bt_load(fname);
    }
    
    //Precomputations
    double IS_A[SIZE2];
    circuit_evaluation_1(IS_A);
    double IS_B[SIZE2];
    circuit_evaluation_2(IS_B);
    double IS_C[SIZE2];
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
    printf("Threads Prepared\n");
        
    clock_gettime(CLOCK_MONOTONIC, &start);
    for(int i=0; i < threads; i++){
        pthread_create(thread_ids + i, NULL, guess_over_k0, (void*) (thread_args + i));
    }
    for(int i=0; i < threads; i++){
        pthread_join(thread_ids[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("DONE --- Elapsed time %f\n", time_meter);

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
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    uint64_t k0k1k2k3k4 = using_thread();

    printf("Key = %lx\n",k0k1k2k3k4);
    uint8_t key_6[16];
    memset(key_6, 0x00, 16);
    key_6[0]   = (k0k1k2k3k4 >> (WORD*4)) & 0xFF;
    key_6[13]  = (k0k1k2k3k4 >> (WORD*3)) & 0xFF;
    key_6[10]  = (k0k1k2k3k4 >> (WORD*2)) & 0xFF;
    key_6[7]   = (k0k1k2k3k4 >> (WORD*1)) & 0xFF;
    uint64_t k4 = k0k1k2k3k4 & 0xFF;
    printf("key_5_prime[0] : %02lX\n", k4);
    printf( "key_6 : ");
    printreg(key_6, 16);

    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("Full Calculation Done --- Elapsed time %f\n", time_meter);
}
void save_data(uint8_t *p, char *fname, __m128i *rk){
    bitmap *ciphertexts = bt_init(1UL<<LOG_DATA);
    bt_zero(ciphertexts);
    data_collection_phase_threaded(ciphertexts, p, rk);
    bt_save(ciphertexts, fname);
    bt_print(ciphertexts);
	//scanf("?");
    bt_destroy(ciphertexts);
    //printf("\n=============Saved=============== \n");
}

void oracle_6r(__m128i *rk){
    struct timespec start, end;
    double time_meter;
    char fname[256];
    printf("Encryption process is running...\n");
    for(uint64_t i=0UL; i<DATA_SETS; i++){
        sprintf(fname, DATA_PATH_6, i);
        clock_gettime(CLOCK_MONOTONIC, &start);
        uint8_t p[16];
        generate_random(p, 16);
        printf("Plaintext : ");
        printreg(p, 16);
        save_data(p, fname, rk);
        clock_gettime(CLOCK_MONOTONIC, &end);
        time_meter = ((double)(end.tv_sec - start.tv_sec));
        printf("Elapsed time encryption phase %ld: %f s\n",i, time_meter);
    }
}

void attack(){
    struct timespec start, end;
    double time_meter;
    
    /* uint8_t Key[16]={0x1,0x2,0x3,0x4,0xF,0x6,0x7,0x8,0x9,0xA,0x0F,0x0E,0x0D,0x0C,0x0E,0x00}; */
    uint8_t Key[16];
    generate_random(Key, 16);
    __m128i rk[11];
    AES_128_Key_Expansion(Key, rk);
    uint8_t *rk5 = (uint8_t *)(rk + 5);
    uint8_t *rk6 = (uint8_t *)(rk + 6);

    for(int i=0; i<ROUNDS+1; i++){
        printf("key_%d : ",i);
        printreg(&rk[i], 16);
    }

    uint8_t rk5_prime[16];
    memcpy(rk5_prime, rk5, 16);
    inverseMixColumn(rk5_prime);
    
    oracle_6r(rk);
    printf("\n=============Saved Data=============== \n");
    printf("Original: \n");
    printf("key_5_prime[0] : %02X\n",(unsigned char)rk5_prime[0]);
    printf("key_6 : ");
    printreg(rk6, 16);
    
    uint8_t rk6_data[16];
    memcpy(rk6_data, rk6, 16);
    printf("Done....Start Finding Key\n");
    find_key();
    printf("Original: \n");
    printf("key_5_prime[0] : %02X\n",(unsigned char)rk5_prime[0]);
    printf("key_6 : ");
    printreg(rk6, 16);
}
int main(){
    xx_initialize(SEED);
    printf("SEED: ");
    printreg(SEED, 32);
    attack();
    return 0;
}

