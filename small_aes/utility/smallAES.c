#include "smallAES.h"

uint8_t sBox[16] = { 0x6, 0xB, 0x5, 0x4, 0x2, 0xE, 0x7, 0xA, 0x9, 0xD, 0xF, 0xC, 0x3, 0x1, 0x0, 0x8 };

//Inverse S-box
uint8_t inv_sBox[16] = { 0xE, 0xD, 0x4, 0xC, 0x3, 0x2, 0x0, 0x6, 0xF, 0x8, 0x7, 0x1, 0xB, 0x9, 0x5, 0xA };
		
/*Multiplication*/
uint8_t multiplicationX(uint8_t byte);
uint8_t mult2[] = { 0x0, 0x2, 0x4, 0x6, 0x8, 0xA, 0xC, 0xE, 0x3, 0x1, 0x7, 0x5,
		0xB, 0x9, 0xF, 0xD };
uint8_t mult3[] = { 0x0, 0x3, 0x6, 0x5, 0xC, 0xF, 0xA, 0x9, 0xB, 0x8, 0xD, 0xE,
		0x7, 0x4, 0x1, 0x2 };
uint8_t multE[] = { 0x0, 0xE, 0xF, 0x1, 0xD, 0x3, 0x2, 0xC, 0x9, 0x7, 0x6, 0x8,
		0x4, 0xA, 0xB, 0x5 };
uint8_t multB[] = { 0x0, 0xB, 0x5, 0xE, 0xA, 0x1, 0xF, 0x4, 0x7, 0xC, 0x2, 0x9,
		0xD, 0x6, 0x8, 0x3 };
uint8_t multD[] = { 0x0, 0xD, 0x9, 0x4, 0x1, 0xC, 0x8, 0x5, 0x2, 0xF, 0xB, 0x6,
		0x3, 0xE, 0xA, 0x7 };
uint8_t mult9[] = { 0x0, 0x9, 0x1, 0x8, 0x2, 0xB, 0x3, 0xA, 0x4, 0xD, 0x5, 0xC,
		0x6, 0xF, 0x7, 0xE };

uint8_t multiplicationX(uint8_t byte){

  uint8_t bitTemp;

  bitTemp = (byte>>3) & 0x1;
  byte = (byte<<1) & 0xf;

  if(bitTemp==0)
    return byte;
  else
    return (byte^0x03);

}

void value_to_nibble_array(uint8_t *arr, uint32_t v, int arr_size){
    for(int i=0; i<arr_size; i++){
        arr[i] = (v >> (4*i)) & 0x0F;
    }

}


		
/*SBOX OPERATION---------------------------------------------*/
void subByte(uint8_t *state){
    for(int i=0; i<16; i++){
        state[i] = sBox[state[i]];
    }
}

void inverseSubByte(uint8_t *state){
    for(int i=0; i<16; i++){
        state[i] = inv_sBox[state[i]];
    }
}
/*------------------------------------------------------------*/

/*SHIFT ROW OPERATION---------------------------------------------
00 04 08 12
01 05 09 13
02 06 10 14
03 07 11 15

=>

00 04 08 12
05 09 13 01 
10 14 02 06 
15 03 07 11 
*/
void shiftRow(uint8_t *state){
    uint8_t p[16] = {0,5,10,15,  4,9,14,3,  8,13,2,7,  12,1,6,11};
    uint8_t temp[16];
    for(int i=0; i<16; i++){
        temp[i] = state[p[i]];
    }
    memcpy(state, temp, 16);
}

void inverseShiftRow(uint8_t *state){
    uint8_t p[16] = {0,13,10,7,  4,1,14,11,  8,5,2,15,  12,9,6,3};
    uint8_t temp[16];
    for(int i=0; i<16; i++){
        temp[i] = state[p[i]];
    }
    memcpy(state, temp, 16);
}
/*------------------------------------------------------------*/

/*MIX COLUMN OPERATION-----------------------------------------*/
void mixColumn(uint8_t *state){
    uint8_t temp[16];
    for(int i=0; i<4; i++){
        temp[4*i + 0] = mult2[state[4*i + 0]]^mult3[state[4*i + 1]]^state[4*i+2]^state[4*i+3];
        temp[4*i + 1] = state[4*i + 0]^mult2[state[4*i + 1]]^mult3[state[4*i+2]]^state[4*i+3];
        temp[4*i + 2] = state[4*i + 0]^state[4*i + 1]^mult2[state[4*i+2]]^mult3[state[4*i+3]];
        temp[4*i + 3] = mult3[state[4*i + 0]]^state[4*i + 1]^state[4*i+2]^mult2[state[4*i+3]];
    }
    memcpy(state, temp, 16);
}

/*
    0E 0B 0D 09
    09 0E 0B 0D
    0D 09 0E 0B
    0B 0D 09 0E
*/
void inverseMixColumn(uint8_t *state){
    uint8_t temp[16];
    for(int i=0; i<4; i++){
        temp[4*i + 0] = multE[state[4*i + 0]]^multB[state[4*i + 1]]^multD[state[4*i+2]]^mult9[state[4*i+3]];
        temp[4*i + 1] = mult9[state[4*i + 0]]^multE[state[4*i + 1]]^multB[state[4*i+2]]^multD[state[4*i+3]];
        temp[4*i + 2] = multD[state[4*i + 0]]^mult9[state[4*i + 1]]^multE[state[4*i+2]]^multB[state[4*i+3]];
        temp[4*i + 3] = multB[state[4*i + 0]]^multD[state[4*i + 1]]^mult9[state[4*i+2]]^multE[state[4*i+3]];
    }
    memcpy(state, temp, 16);
}

/*---------------------------------------------------------------*/


void addRoundKey(uint8_t *state, uint8_t *key) {
	int i;
	for (i=0; i<16; i++){
	    state[i] = state[i]^key[i];
	}

}

void encryption(uint8_t *p, uint8_t **rk, uint8_t *c){
    int i, j;
    uint8_t state[16];
    memcpy(state, p, 16);
    /*
    for(i=0; i<ROUNDS+1; i++){
        //generate_random_nibble(rk[i]);
        printf("key_%d : ",i);
        printreg(rk[i], 16);
    }
    */
    addRoundKey(state, rk[0]);
   
    for(i=0; i<ROUNDS-1; i++){
        subByte(state);
		shiftRow(state);
		mixColumn(state);
		addRoundKey(state, rk[i+1]);
    }
    //Final Round
	subByte(state);
	shiftRow(state);
	addRoundKey(state, rk[ROUNDS]);
	
	memcpy(c, state, 16);
}

void decryption(uint8_t *p, uint8_t **rk, uint8_t *c){
    int i, j;
    uint8_t state[16];
    memcpy(state, c, 16);
    
    addRoundKey(state, rk[ROUNDS]);
    
    for(i=ROUNDS-1; i>0; i--){
        inverseSubByte(state);
        inverseShiftRow(state);
        addRoundKey(state, rk[i]);
		inverseMixColumn(state);
		
    }
    //Final Round
	inverseSubByte(state);
	inverseShiftRow(state);
	addRoundKey(state, rk[0]);
	
	memcpy(p, state, 16);

}

/*................DATA COLLECTION................................*/
pthread_mutex_t lock[NROF_LOCK];
void set_diagonal(uint64_t value, uint8_t *state){
    state[0] = (value & 0x0F);
    state[5] = ((value >> 4) & 0x0F);
    state[10] = ((value >> 8) & 0x0F);
    state[15] = ((value >> 12) & 0x0F);
}

//anti diagonal
uint64_t get_071013(uint8_t *c){
    uint64_t data = 0L;
    
    data = data | c[7];
    data = (data << 4) | c[10];
    data = (data << 4) | c[13];
    data = (data << 4) | c[0];
    
    return data;
}
void bt_set_parity_threaded(bitmap *bt, uint64_t index){
    uint64_t word = (index/64); 
    uint8_t bit = (uint8_t)(index % 64);

    uint64_t size = (bt->nrof_bits)/64; //Number of words
    uint64_t words_per_mutex = size/NROF_LOCK;

    uint64_t lock_nr = word/words_per_mutex;
    /* printf("Lock_nr %ld\n", lock_nr); */
    pthread_mutex_lock(lock + lock_nr);
    bt->bits[word] ^= (1ULL << bit);
    pthread_mutex_unlock(lock + lock_nr);
}
void* encrypt_over_ranges(void *args){
    DataArgs *dataargs = (DataArgs*) args;
    uint8_t p[16];
    memcpy(p, dataargs->plain, 16);
    for(uint64_t i=dataargs->start; i < dataargs->end; i += 1){
        uint8_t c[16];
        set_diagonal(i, p);
        encryption(p, dataargs->rk, c);
        uint64_t data = get_071013(c);
        /* pthread_mutex_lock(&lock_parity_1); */
        bt_set_parity_threaded(dataargs->ciphertexts, data);
        /* pthread_mutex_unlock(&lock_parity_1); */
        //print_info(dataargs->plain, c, data);
    }
    pthread_exit(NULL);
}

void data_collection_phase_threaded(bitmap *c, 
                        uint8_t *p, uint8_t **rk){
    int threads = 8;
    pthread_t thread_ids[threads]; 
    DataArgs  thread_args[threads];
    
    uint64_t cube_size = (1ULL << LOG_DATA);
	uint64_t data_in_each_thread = (cube_size / threads);

    for(int i=0; i<threads; i++){
        thread_args[i].rk = rk;
        thread_args[i].plain = p;
        thread_args[i].start = i * data_in_each_thread;
        thread_args[i].end = (i+1) * data_in_each_thread;
        thread_args[i].ciphertexts = c;
    }

    for(int i=0; i < threads; i++){
        pthread_create(thread_ids + i, NULL, encrypt_over_ranges, (void*) (thread_args + i));
    }
    for(int i=0; i < threads; i++){
        pthread_join(thread_ids[i], NULL);
    }
}

void data_collection_phase(bitmap *ciphertexts, uint8_t *p, uint8_t **rk){
    bt_zero(ciphertexts);
    for(uint64_t i=0; i < (1ULL << LOG_DATA); i++){
        uint8_t c[16];
        set_diagonal(i, p);
        encryption(p, rk, c);
        uint64_t data = get_071013(c);
        bt_set_parity(ciphertexts, data);
        //print_info(dataargs->plain, c, data);
    }
}


//Utility Functions for precomputations
//compute S4(x4+S3(x3)+S2(x2)+S1(x1)+S0(x0))
uint8_t evaluate_F(uint64_t ctext, uint8_t k4){
    uint8_t v[4];
    value_to_nibble_array(v, ctext, 4);
    uint8_t x1 = multE[inv_sBox[v[0]]] ^ multB[inv_sBox[v[1]]];
    uint8_t x2 = x1 ^ multD[inv_sBox[v[2]]];
    uint8_t x3 = x2 ^ mult9[inv_sBox[v[3]]];
    uint8_t x = inv_sBox[x3 ^ k4];
    return x;
}
uint8_t SS01(uint8_t ctext){
    uint8_t v[2];
    value_to_nibble_array(v, ctext, 2);
	uint8_t x1 = multE[inv_sBox[v[0]]] ^ multB[inv_sBox[v[1]]];
	return x1;
}
uint8_t SS0(uint8_t x){
    uint8_t y = multE[inv_sBox[x]];
    return y;
}
uint8_t SS1(uint8_t x){
    uint8_t y = multB[inv_sBox[x]];
    return y;
}
uint8_t SS2(uint8_t x){
    uint8_t y = multD[inv_sBox[x]];
    return y;
}
uint8_t SS3(uint8_t x){
    uint8_t y = mult9[inv_sBox[x]];
    return y;
}
uint8_t SS4(uint8_t x){
    uint8_t y = inv_sBox[x];
    return y;
}

