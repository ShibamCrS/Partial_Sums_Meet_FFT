#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
/* #define FIXK0 */

#ifdef FIXK0
    #define THREADS 1
    #define THREADS_TODOS_ATTACK 1
#else
    #define THREADS 128
    #define THREADS_TODOS_ATTACK 8
#endif

#define THREADS_PARTIAL_SUMS 128
#define NROF_THREADS_DATA 32


#define DATA_PATH_6 "../data/data_6r_%ld.tmp"

#define ROUNDS 6
#define LOG_DATA 32
#define WORD 8
#define DATA_SETS 6
#define SIZE1 256UL  //2^WORD
#define SIZE2 65536UL //2^(2*WORD)
#define SIZE3 16777216UL
#define SIZE4 4294967296UL

/*
For parallel computation of FFTs
MSBX=2 denotes we compute 2^2 = 4 FFTs in parallel in the X-th step
SEPX=14 denotes we need 14 bit seperation
*/

#define SEP1 14
#define MSB1 2
#define LSB1 6

#define SEP2 14
#define MSB2 2
#define LSB2 6

#define SEP3 14
#define MSB3 2
#define LSB3 6

#define SEP4 7

//Printing Staffs
#include "bitmap.h"
#include "aes.h"
void printreg_to_file(const void *a, int nrof_byte, FILE *fp);
void printreg(const void *a, int nrof_byte);

#endif

