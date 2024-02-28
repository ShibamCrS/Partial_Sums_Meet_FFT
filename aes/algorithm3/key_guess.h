#ifndef KEYGUESS
#define KEYGUESS

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

struct Step2Args{
    bitmap *A;
    bitmap *RemovedKeys;
    uint64_t start;
    uint64_t end;
};
typedef struct Step2Args Step2Args;

void *key_guess_0(void *args1);
void *key_guess_1(void *args1);
void *key_guess_2(void *args1);
void *key_guess_345(void *args1);
uint64_t filter_keys(bitmap *A, bitmap *RemovedKeys, uint64_t data_set_index);
#endif  
    

