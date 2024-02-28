#ifndef FHT_STEPS
#define FHT_STEPS 

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#include "../utility/utility.h"
#include "../utility/bitmap.h"
#include "../utility/aes.h"

/*................. PRECOMPUTATION ............................*/
void circuit_evaluation_1(double *IS);
void circuit_evaluation_2(double *IS);
void circuit_evaluation_3(double *IS);
void circuit_evaluation_4(uint64_t *D);
/*..................................................................*/
void fht_step_1(bitmap *chi, bitmap *A, uint64_t c3, double *IS);
void fht_step_2(bitmap *A, bitmap *B, uint64_t k1, double *IS);
void fht_step_3(bitmap *B, bitmap *O, double *IS);
void fht_step_4(bitmap *C, uint8_t *SUM, uint64_t k3, uint64_t k2, uint64_t *D);
#endif
