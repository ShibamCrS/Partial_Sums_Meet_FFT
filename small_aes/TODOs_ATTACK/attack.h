#ifndef ATTACK_H
#define ATTACK_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "../utility/utility.h"

struct PSArgs{
    bitmap **ciphertexts;
    uint64_t start;
    uint64_t end;
    uint64_t recovered_key;
    int found_key;
};
typedef struct PSArgs PSArgs;
void fht_step(bitmap *c, uint8_t *sum, uint64_t k4, uint64_t data_index);
#endif
