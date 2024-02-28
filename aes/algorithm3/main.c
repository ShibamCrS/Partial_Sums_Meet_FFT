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
#include "attack.h"

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
 *This Function tests one delta set.
 *******************************************************************************/
void find_key_one_delta_set(uint64_t dataset_index, bitmap *RemovedKeys, bitmap *A){
    struct timespec start, end;
    double time_meter;

    bt_zero(A);
    compute_first_step(A, dataset_index);
    
    printf("Key Guess Start\n");


    //Key Guess
    clock_gettime(CLOCK_MONOTONIC, &start);

    filter_keys(A, RemovedKeys, dataset_index);

    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("Done For DataSet %ld--- Elapsed time %f\n", dataset_index, time_meter);
}
/********************************************************************************
 *This Function considers a set of all possible keys and run Algorithm 3 for each
 *Delta set sequentially. If the integral property does not satisfy for some key
 *it marked that key as a wrong key
 *******************************************************************************/
void find_key(){
    struct timespec start, end;
    double time_meter;

    clock_gettime(CLOCK_MONOTONIC, &start);
    uint64_t len_a = (1UL<<(5*WORD));
    bitmap *A = bt_init(len_a);

    bitmap *RemovedKeys = bt_init((1UL << (WORD*5)));
    bt_zero(RemovedKeys);
    for(int i=0; i<DATA_SETS; i++){
        find_key_one_delta_set(i, RemovedKeys, A);
    }
    bt_destroy(A);
    uint64_t counter = 0UL;
    uint64_t k0k1k2k3k4 = 0UL;
    while(bt_next_zero(RemovedKeys, &k0k1k2k3k4)){
    printf("Key = %lx\n",k0k1k2k3k4);
    uint8_t key_6[16];
    memset(key_6, 0x00, 16);
    key_6[0]   = (k0k1k2k3k4 >> (WORD*4)) & 0xFF;
    key_6[13]  = (k0k1k2k3k4 >> (WORD*3)) & 0xFF;
    key_6[10]  = (k0k1k2k3k4 >> (WORD*2)) & 0xFF;
    key_6[7]   = (k0k1k2k3k4 >> (WORD*1)) & 0xFF;
    uint8_t k4 = k0k1k2k3k4 & 0xFF;
    printf("key_5_prime[0] : %02X\n", k4);
    printf( "key_6 : ");
    printreg(key_6, 16);
    k0k1k2k3k4++;
    printf("Counter = %ld\n", counter);
    counter++;
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("Full Calculation Done %f\n", time_meter);

    bt_destroy(RemovedKeys);
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
    for(uint64_t i=0; i<DATA_SETS; i++){
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

