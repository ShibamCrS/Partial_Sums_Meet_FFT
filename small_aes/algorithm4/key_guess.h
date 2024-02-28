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
#include "guessK0.h"

void key_guess_0(void *args1);
void key_guess_1(void *args1);
void key_guess_2(void *args1);
void key_guess_345(void *args1);
#endif
