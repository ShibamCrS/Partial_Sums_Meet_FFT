#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../utility/utility.h"
#include "../utility/bitmap.h"
#include "../utility/smallAES.h"

#include "FHT_computations.h"
#include "key_guess.h"
#include "attack.h"

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
 *This Function tests one delta set.
 *******************************************************************************
 */

void find_key_one_delta_set(int dataset_index, bitmap *RemovedKeys){
    struct timespec start, end;
    double time_meter;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    uint64_t len_a = (1UL<<(5*WORD));
    bitmap *A = bt_init(len_a);
    bt_zero(A);
    compute_first_step(A, dataset_index); //given in attack.c
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("First STEP DONE --- Elapsed time %f\n", time_meter);
    
    printf("Key Guess Start\n");


    /*
    --------------------MAIN ALGORITHM FOR KEY GUESS----------------
    */
    //Key Guess
    clock_gettime(CLOCK_MONOTONIC, &start);

    filter_keys(A, RemovedKeys, dataset_index); //given in key_guess.c

    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("Done For DataSet %d--- Elapsed time %f\n", dataset_index, time_meter);
    bt_destroy(A);
}

/********************************************************************************
 *This Function considers a set of all possible keys and run Algorithm 3 for each 
 *Delta set sequentially. If the integral property does not satisfy for some key
 *it marked that key as a wrong key
 *******************************************************************************/
void find_key(){
    bitmap *RemovedKeys = bt_init((1UL << (WORD*5)));
    for(int i=0; i<DATA_SETS; i++){
        find_key_one_delta_set(i, RemovedKeys);
    }
    uint64_t k0k1k2k3k4 = 0UL;
    printf("\n\n\n ATTACK COMPLETED \n");
    printf("------Recoverd Keys at Nibble k6_0, k6_13, k6_10, k6_7, k5_0---------\n");
    while(bt_next_zero(RemovedKeys, &k0k1k2k3k4)){
    printf("Key = %lx\n",k0k1k2k3k4);
    uint8_t key_6[16];
    memset(key_6, 0x00, 16);
    key_6[0]   = (k0k1k2k3k4 >> (WORD*4))  & 0x0F;
    key_6[13]  = (k0k1k2k3k4 >> (WORD*3)) & 0x0F;
    key_6[10]  = (k0k1k2k3k4 >> (WORD*2)) & 0x0F;
    key_6[7]   = (k0k1k2k3k4 >> (WORD*1)) & 0x0F;
    uint8_t k4 = k0k1k2k3k4 & 0x0F;
    printf("key_5_prime[0] : %02lX\n", k4);
    printf( "key_6 : ");
    printreg(key_6, 16);
    k0k1k2k3k4++;
    }
    printf("-------------------------------------------------------------------\n");
    bt_destroy(RemovedKeys);
}
void save_data(uint8_t *p, char *fname, uint8_t **rk){
    bitmap *ciphertexts = bt_init(1UL<<LOG_DATA);
    bt_zero(ciphertexts);
    //generate 2^32 ciphertexts from structured 2^32 structured plaintexts
    data_collection_phase_threaded(ciphertexts, p, rk); 
    bt_save(ciphertexts, fname);
    /* bt_print(ciphertexts); */
	//scanf("?");
    bt_destroy(ciphertexts);
    //printf("\n=============Saved=============== \n");
}

//6-round small AES oracle
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

//Main Attack Function
void attack(){
    struct timespec start, end;
    double time_meter;
    
    uint8_t *rk[ROUNDS+1];
    generate_random_round_keys(rk, ROUNDS);
    for(int i=0; i<ROUNDS+1; i++){
        printf("key_%d : ",i);
        printreg(rk[i], 16);
    }
    uint8_t rk5_prime[16];
    memcpy(rk5_prime, rk[5], 16);
    inverseMixColumn(rk5_prime);
    
    oracle_6r(rk);
    printf("\n=============Saved Data=============== \n");
    printf("Original Key: \n");
    printf("key_5_prime[0] : %02X\n",(unsigned char)rk5_prime[0]);
    printf("key_6 : ");
    printreg(rk[ROUNDS], 16);
    find_key();
    printf("Original Key: \n");
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

