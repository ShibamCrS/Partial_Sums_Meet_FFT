#ifndef ATTACK
#define ATTACK 

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../utility/utility.h"
#include "../utility/bitmap.h"
#include "FHT_computations.h"

struct FHTArgs{
    bitmap *ciphertexts;
    bitmap *A;
    double *IS;
    uint64_t start;
    uint64_t end;
};
typedef struct FHTArgs FHTArgs;

void* FHT_over_ranges(void *args1);
void using_thraed(char *fname, bitmap *A); 
void compute_first_step(bitmap *A, uint64_t dataset_index);

#endif

