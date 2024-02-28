#ifndef KEYGUESSK
#define KEYGUESSK

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../utility/utility.h"
#include "../utility/bitmap.h"
#include "../utility/smallAES.h"
#include "FHT_computations.h"
#include "key_guess.h"

struct Args{
    bitmap **ciphertexts;
    bitmap *A; 
    bitmap *B; 
    bitmap *C;
    bitmap *RK;
    uint64_t *IS_A;
    uint64_t *IS_B;
    uint64_t *IS_C;
    uint64_t *IS_D;
    uint64_t start;
    uint64_t end;

    uint64_t found_key;
    uint64_t Key;
};
typedef struct Args Args;

void prepare_chi(bitmap *c, bitmap *chi, uint64_t k0);
void *guess_over_k0(void *args1);
#endif
