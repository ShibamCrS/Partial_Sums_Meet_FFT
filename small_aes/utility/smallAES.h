#ifndef SMALL_AES_H
#define SMALL_AES_H
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "utility.h"
#include "bitmap.h"
#define ROUNDS 6

void value_to_nibble_array(uint8_t *arr, uint32_t v, int arr_size);
uint8_t evaluate_F(uint64_t ctext, uint8_t k4);
uint8_t SS01(uint8_t ctext);
uint8_t SS0(uint8_t x);
uint8_t SS1(uint8_t x);
uint8_t SS2(uint8_t x);
uint8_t SS3(uint8_t x);
uint8_t SS4(uint8_t x);

void subByte(uint8_t *state);
void inverseSubByte(uint8_t *state);
void shiftRow(uint8_t *state);
void inverseShiftRow(uint8_t *state);
void mixColumn(uint8_t *state);
void inverseMixColumn(uint8_t *state);
void addRoundKey(uint8_t *state, uint8_t *key);
void encryption(uint8_t *p, uint8_t **rk, uint8_t *c);
void decryption(uint8_t *p, uint8_t **rk, uint8_t *c);

/*................DATA COLLECTION................................*/
#define NROF_LOCK 16
struct DataArgs{
    uint8_t **rk;
    uint8_t *plain;
    uint64_t start;
    uint64_t end;
    bitmap *ciphertexts;
};
typedef struct DataArgs DataArgs;

void set_diagonal(uint64_t value, uint8_t *state);
//anti diagonal
uint64_t get_071013(uint8_t *c);
void print_info(uint8_t *p, uint8_t *c, uint32_t data);
void bt_set_parity_threaded(bitmap *bt, uint64_t index);
void* encrypt_over_ranges(void *args);
void data_collection_phase_threaded(bitmap *c, uint8_t *p, uint8_t **rk);
void data_collection_phase(bitmap *ciphertexts, uint8_t *p, uint8_t **rk);

#endif
