#ifndef AES_H
#define AES_H

#include<immintrin.h>
#include<wmmintrin.h>
#include<xmmintrin.h>
#include<smmintrin.h>
#include<emmintrin.h>
#include<tmmintrin.h>
#include<malloc.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>


__m128i AES_128_ASSIST (__m128i temp1, __m128i temp2) {
 __m128i temp3;
 temp2 = _mm_shuffle_epi32 (temp2 ,0xff);
 temp3 = _mm_slli_si128 (temp1, 0x4);
 temp1 = _mm_xor_si128 (temp1, temp3);
 temp3 = _mm_slli_si128 (temp3, 0x4);
 temp1 = _mm_xor_si128 (temp1, temp3);
 temp3 = _mm_slli_si128 (temp3, 0x4);
 temp1 = _mm_xor_si128 (temp1, temp3);
 temp1 = _mm_xor_si128 (temp1, temp2);
 return temp1; 
}

void AES_128_Key_Expansion (unsigned char *userkey, __m128i *Key_Schedule) {
   __m128i temp1, temp2;

 temp1 = _mm_loadu_si128 ((__m128i*)userkey);
Key_Schedule[0] = temp1;
temp2 = _mm_aeskeygenassist_si128 (temp1,0x1);
temp1 = AES_128_ASSIST(temp1, temp2);
Key_Schedule[1] = temp1;
temp2 = _mm_aeskeygenassist_si128 (temp1,0x2);
temp1 = AES_128_ASSIST(temp1, temp2);
Key_Schedule[2] = temp1;
temp2 = _mm_aeskeygenassist_si128 (temp1,0x4);
temp1 = AES_128_ASSIST(temp1, temp2);
Key_Schedule[3] = temp1;
temp2 = _mm_aeskeygenassist_si128 (temp1,0x8);
temp1 = AES_128_ASSIST(temp1, temp2);
Key_Schedule[4] = temp1;
temp2 = _mm_aeskeygenassist_si128 (temp1,0x10);
temp1 = AES_128_ASSIST(temp1, temp2);
Key_Schedule[5] = temp1;
temp2 = _mm_aeskeygenassist_si128 (temp1,0x20);
temp1 = AES_128_ASSIST(temp1, temp2);
Key_Schedule[6] = temp1;
temp2 = _mm_aeskeygenassist_si128 (temp1,0x40);
temp1 = AES_128_ASSIST(temp1, temp2);
Key_Schedule[7] = temp1;
temp2 = _mm_aeskeygenassist_si128 (temp1,0x80);
temp1 = AES_128_ASSIST(temp1, temp2);
 Key_Schedule[8] = temp1;
 temp2 = _mm_aeskeygenassist_si128 (temp1,0x1b);
 temp1 = AES_128_ASSIST(temp1, temp2);
 Key_Schedule[9] = temp1;
 temp2 = _mm_aeskeygenassist_si128 (temp1,0x36);
 temp1 = AES_128_ASSIST(temp1, temp2);
 Key_Schedule[10] = temp1;
}
 __m128i  AES_128_Encrypt_wo_last_mc(__m128i ptext, __m128i *keySchedule, int rounds){
      int j;
      __m128i tmp;

      tmp = _mm_xor_si128 (ptext,keySchedule[0]);
      for(j=1; j <rounds; j++){
         tmp = _mm_aesenc_si128 (tmp,keySchedule[j]);
      }
      tmp = _mm_aesenclast_si128 (tmp,keySchedule[j]);
      /*_mm_storeu_si128 (&((__m128i*)out)[0],tmp);*/
      return(tmp);
}
void encryption(__m128i *roundKeys,uint8_t *plaintext,uint8_t *ciphertext,int rounds){
    __m128i P = _mm_loadu_si128 ((__m128i*)plaintext);
    
    __m128i C = AES_128_Encrypt_wo_last_mc(P,roundKeys, rounds);
    
    uint8_t *c = (uint8_t *)&C;
    
    for(int i=0;i<16;i++)
    {
        ciphertext[i] = c[i];
    }
}

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

void set_diagonal(uint64_t value, uint8_t *state){
    state[0] = (value & 0xFF);
    state[5] = ((value >> 8) & 0xFF);
    state[10] = ((value >> 16) & 0xFF);
    state[15] = ((value >> 24) & 0xFF);
}

//anti diagonal
/* 0  4  8 12 */
/* 1  5  9 13 */
/* 2  6 10 14 */
/* 3  7 11 15 */
uint64_t get_071013(uint8_t *c){
    uint64_t data = 0L; 
    
    data = data | c[7];
    data = (data << WORD) | c[10];
    data = (data << WORD) | c[13];
    data = (data << WORD) | c[0];
    
    return data;
}

void print_info(uint8_t *p, uint8_t *c, uint32_t data){
    printf("Plaintext : ");
    printreg(p, 16);
    printf("Ciphertext : ");
    printreg(c, 16);
    printf("Data : %04X\n",data);
    //printreg(&data, 4);
}

pthread_mutex_t lock[NROF_LOCK];
int bt_set_parity_threaded(bitmap *bt, uint64_t index){
    uint64_t word = (index/64); 
    uint8_t bit = (uint8_t)(index % 64);

    uint64_t size = (bt->nrof_bits)/64; //Number of words
    uint64_t words_per_mutex = size/NROF_LOCK;

    uint64_t lock_nr = word/words_per_mutex;
    /* printf("Lock_nr %ld\n", lock_nr); */
    pthread_mutex_lock(lock + lock_nr);
    bt->bits[word] ^= (1ULL << bit);
    pthread_mutex_unlock(lock + lock_nr);
}

void* encrypt_over_ranges(void *args){
    DataArgs *dataargs = (DataArgs*) args;
    uint8_t p[16];
    memcpy(p, dataargs->plain, 16);
    for(uint64_t i=dataargs->start; i < dataargs->end; i += 1){
        uint8_t c[16];
        set_diagonal(i, p);
        encryption(dataargs->rk, p, c, ROUNDS);
        uint64_t data = get_071013(c);
        /* pthread_mutex_lock(&lock_parity_1); */
        bt_set_parity_threaded(dataargs->ciphertexts, data);
        /* pthread_mutex_unlock(&lock_parity_1); */
        //print_info(dataargs->plain, c, data);
    }
    pthread_exit(NULL);
}

void data_collection_phase_threaded(bitmap *c, 
                        uint8_t *p, __m128i *rk){
    pthread_t thread_ids[NROF_THREADS_DATA]; 
    DataArgs  thread_args[NROF_THREADS_DATA];
    
    uint64_t cube_size = (1ULL << LOG_DATA);
	uint64_t data_in_each_thread = (cube_size / NROF_THREADS_DATA);

    for(int i=0; i<NROF_THREADS_DATA; i++){
        thread_args[i].rk = rk;
        thread_args[i].plain = p;
        thread_args[i].start = i * data_in_each_thread;
        thread_args[i].end = (i+1) * data_in_each_thread;
        thread_args[i].ciphertexts = c;
    }

    for(int i=0; i < NROF_THREADS_DATA; i++){
        pthread_create(thread_ids + i, NULL, encrypt_over_ranges, (void*) (thread_args + i));
    }
    for(int i=0; i < NROF_THREADS_DATA; i++){
        pthread_join(thread_ids[i], NULL);
    }
}

void data_collection_phase(bitmap *ciphertexts, uint8_t *p, __m128i *rk){
    bt_zero(ciphertexts);
    for(uint64_t i=0; i < (1ULL << LOG_DATA); i++){
        uint8_t c[16];
        set_diagonal(i, p); 
        encryption(rk, p, c, ROUNDS);
        uint64_t data = get_071013(c);
        bt_set_parity(ciphertexts, data);
        //print_info(dataargs->plain, c, data);
    }   
}
#endif
