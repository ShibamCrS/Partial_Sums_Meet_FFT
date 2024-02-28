#ifndef BITMAP_H
#define BITMAP_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct bitmap{
    uint64_t nrof_bits;
    uint64_t *bits;
};
typedef struct bitmap bitmap;

int bt_init_ptr(bitmap *bt, uint64_t nrof_bits);
bitmap* bt_init(uint64_t nrof_bits);
void bt_destroy(bitmap *bt);
uint8_t bt_get(bitmap *bt, uint64_t index);
int bt_set(bitmap *bt, uint64_t index, uint8_t value);
int bt_set_1(bitmap *bt, uint64_t index);
int bt_set_parity(bitmap *bt, uint64_t index);
void bt_flip(bitmap *bt, uint64_t index);
void bt_flip_all(bitmap *bt);
int bt_first_zero(bitmap *bt, uint64_t *index);
int bt_next_zero(bitmap *bt, uint64_t *index);
uint64_t bt_popcount(uint64_t word);
uint64_t bt_zero(bitmap *bt);
uint64_t bt_hamming_weight(bitmap *bt, uint64_t index_from, uint64_t index_to);
int bt_save(bitmap *bt, char *fname);
bitmap* bt_load(char *fname);
void bt_print(bitmap *bt);
uint32_t bt_size(bitmap *bt);

#endif
