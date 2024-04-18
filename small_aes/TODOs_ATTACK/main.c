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
void find_key(){
    //Loading 6 sets of ciphertexts of size 2^16
	bitmap *ciphertexts[DATA_SETS];
    for(int i=0; i<DATA_SETS; i++){
        char fname[256];
        sprintf(fname, DATA_PATH_6, i);
        ciphertexts[i] = bt_load(fname);
    }
    struct timespec start, end;
    double time_meter;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for(uint64_t k4=0UL; k4<16; k4++){ 
        uint8_t sum[(1UL << LOG_DATA )*DATA_SETS];
        memset(sum, 0x00, (1UL << LOG_DATA )*DATA_SETS);
        int valid_k4 = 0;
        printf("\n=============Start K4 %d=============== \n",k4);
        for(uint64_t i=0UL; i<DATA_SETS; i++){
            fht_step(ciphertexts[i], sum, k4, i);
        }
        for(uint64_t k=0UL; k<(1UL << LOG_DATA); k++){
            int final_flag = 1;
            for(uint64_t i=0UL; i<DATA_SETS; i++){
                if(sum[i*(1UL << LOG_DATA) + k] != 0){
                    final_flag = 0;
                    break;
                }
            }
            if(final_flag == 1){
                valid_k4 = 1;

                uint8_t key_6[16];
                memset(key_6, 0x00, 16);
                key_6[0]   = (k & 0x0F);
                key_6[13]  = (k >> 4) & 0x0F;
                key_6[10]  = (k >> 8) & 0x0F;
                key_6[7]   = (k >> 12)& 0x0F;
                printf("key_5_prime[0] : %02X\n", k4);
                printf( "key_6 : ");
                printreg(key_6, 16);
                break;
            }
        }
        if(valid_k4 == 1){
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));

    for(int i=0; i<DATA_SETS; i++){
        bt_destroy(ciphertexts[i]);
    }
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
