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

#define THREADS 8
#define DATA_PATH_6 "../data/data_6r_%d.tmp"

#define LOG_DATA 16
#define ROUNDS 6
#define WORD 4
#define DATA_SETS 6
#define SIZE1 16UL  //2^WORD
#define SIZE2 256UL //2^(2*WORD)
#define SIZE3 4096UL
#define SIZE4 65536UL
/*
For parallel computation of FFTs
MSBX=3 denotes we compute 2^3 = 8 FFTs in parallel in the X-th step
SEPX=4 denotes we need 4 bit seperation
*/
#define SEP1 4
#define MSB1 3
#define LSB1 1

#define SEP2 4
#define MSB2 3
#define LSB2 1

#define SEP3 4
#define MSB3 3
#define LSB3 1

#define SEP4 4

//Printing Staffs
#include "bitmap.h"
#include "smallAES.h"
void printreg_to_file(const void *a, int nrof_byte, FILE *fp);
void printreg(const void *a, int nrof_byte);

#endif

