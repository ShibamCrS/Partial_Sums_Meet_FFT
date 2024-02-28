#ifndef AES_H
#define AES_H

#include<immintrin.h>
#include<wmmintrin.h>
#include<xmmintrin.h>
#include<smmintrin.h>
#include<emmintrin.h>
#include<tmmintrin.h>
/* #include<malloc.h> */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <stdint.h>
#include "bitmap.h"
#include "utility.h"


__m128i AES_128_ASSIST (__m128i temp1, __m128i temp2);
void AES_128_Key_Expansion (unsigned char *userkey, __m128i *Key_Schedule);
__m128i  AES_128_Encrypt_wo_last_mc(__m128i ptext, __m128i *keySchedule, int rounds);
void encryption(__m128i *roundKeys,uint8_t *plaintext,uint8_t *ciphertext,int rounds);
/*********************************************************************************/

#define NROF_LOCK 128
struct DataArgs{
    __m128i *rk;
    uint8_t *plain;
    uint64_t start;
    uint64_t end;
    bitmap *ciphertexts;
};
typedef struct DataArgs DataArgs;

void set_diagonal(uint64_t value, uint8_t *state);
//anti diagonal
/* 0  4  8 12 */
/* 1  5  9 13 */
/* 2  6 10 14 */
/* 3  7 11 15 */
uint64_t get_071013(uint8_t *c);
void print_info(uint8_t *p, uint8_t *c, uint32_t data);
int bt_set_parity_threaded(bitmap *bt, uint64_t index);
void* encrypt_over_ranges(void *args);
void data_collection_phase_threaded(bitmap *c, uint8_t *p, __m128i *rk);
void data_collection_phase(bitmap *ciphertexts, uint8_t *p, __m128i *rk);


void inverseMixColumn(uint8_t *state);
//compute S4(x4 + S3(x3) + S2(x2) + S1(x1) + S0(x0))
uint8_t evaluate_F(uint64_t ctext, uint64_t k4);
uint8_t SS01(uint64_t ctext);
uint64_t SS0(uint64_t x);
uint64_t SS1(uint64_t x);
uint64_t SS2(uint64_t x);
uint64_t SS3(uint64_t x);
uint64_t SS4(uint64_t x);
#endif
