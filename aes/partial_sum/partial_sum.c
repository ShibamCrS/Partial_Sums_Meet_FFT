#include "partial_sum.h"

int STOP = 0;
uint8_t get_x1( uint8_t c0, uint8_t c1, uint8_t k0, uint8_t k1 ){
    uint8_t x1 = 0;
    x1 = SS0(c0 ^ k0) ^ SS1(c1 ^ k1);
    return x1;
}

uint8_t get_x2( uint8_t x1, uint8_t c2, uint8_t k2 ){
    uint8_t x2 = 0;
    x2 = x1 ^ SS2(c2 ^ k2);
    return x2;
}
uint8_t get_x3( uint8_t x2, uint8_t c3, uint8_t k3 ){
    uint8_t x3 = 0;
    x3 = x2 ^ SS3(c3 ^ k3);
    return x3;
}
void set_counter_x1(bitmap *ciphertexts, bitmap *vec_x1, uint64_t ds_index, uint64_t k1k0){
    for(uint64_t i=0UL; i < (1UL<<LOG_DATA); i++){ 
        int flag = bt_get(ciphertexts, i); 
        if (flag == 1){ 
            //c3 c2 c1 c0
            uint8_t c0 = i & 0xFF;
            uint8_t c1 = (i >> WORD) & 0xFF;
            uint64_t c3c2 = (i >> (WORD*2)) & 0xFFFF;
            uint64_t k1 = (k1k0 >> WORD) & 0xFF;
            uint64_t k0 = k1k0 & 0xFF;
            uint8_t x1 = get_x1(c0, c1, k0, k1);
            //c3 c2 x1
            uint64_t data = 0UL;
            data = data | c3c2;
            data = (data << WORD ) | x1;
            uint64_t pos = (ds_index*(1UL<<(WORD*3))) + data;
            bt_set_parity(vec_x1, pos);
        }
    }   
}
void set_counter_x2(bitmap *vec_x1, bitmap *vec_x2, uint64_t ds_index, uint64_t k2){
    for(uint64_t i=0UL; i < (1UL << (WORD*3)); i++){
        uint64_t pos = (ds_index*(1UL<<(WORD*3))) + i;
        int flag = bt_get(vec_x1, pos); 
        if (flag == 1){ 
            //c3 c2 x1
            uint8_t x1 = i & 0xFF;
            uint8_t c2 = (i >> WORD) & 0xFF;
            uint64_t c3 = (i >> (2*WORD)) & 0xFF;
            uint8_t x2 = get_x2(x1, c2, k2);
            //c3 x2
            uint64_t data = 0UL;
            data = data | c3; 
            data = (data << WORD ) | x2; 
            uint64_t pos_1 = (ds_index*(1UL<<(WORD*2))) + data;
            bt_set_parity(vec_x2, pos_1);
        }
    }   
}

void set_counter_x3(bitmap *vec_x2, bitmap *vec_x3, uint64_t ds_index, uint64_t k3){
    for(uint64_t i=0; i < (1UL << (2*WORD)); i += 1){ 
        uint64_t pos_1 = (ds_index*(1UL<<(WORD*2))) + i;
        int flag = bt_get(vec_x2, pos_1); 
        if (flag == 1){ 
            //c3 x2
            uint8_t x2 = i & 0xFF;
            uint8_t c3 = (i >> WORD) & 0xFF;
            uint8_t x3 = get_x3(x2, c3, k3);
            //x3
            uint64_t data = 0UL; 
            data = data | x3; 
            uint64_t pos_2 = (ds_index*(1UL<<WORD)) + data;
            bt_set_parity(vec_x3, pos_2);
        }
    }   
}
uint8_t total_sum(bitmap *vec_x3, uint64_t ds_index, uint64_t k4){
    uint8_t sum = 0UL;
    for(uint64_t i=0; i < (1UL << WORD); i++){ 
        uint64_t pos_2 = (ds_index*(1UL<<WORD)) + i;
        int flag = bt_get(vec_x3, pos_2); 
        if (flag == 1){ 
            //x3
            uint8_t x3 = i & 0xFF;
            sum ^= SS4(x3 ^ k4);
        }
    }
    return sum;
}
void *key_guess_over_ranges(void *args1){
    struct timespec start, end;
    double time_meter;
    PSArgs *args = (PSArgs *) args1;

    uint64_t len_x1 = DATA_SETS*(1ULL << (3*WORD));
    uint64_t len_x2 = DATA_SETS*(1ULL << (2*WORD));
    uint64_t len_x3 = DATA_SETS*(1ULL << WORD);
    
    bitmap *vec_x1 = bt_init(len_x1);
    bitmap *vec_x2 = bt_init(len_x2);
    bitmap *vec_x3 = bt_init(len_x3);

    uint8_t sum[DATA_SETS];

    for(uint64_t k1=args->start; k1<args->end; k1++){
        uint64_t k1k0 = (k1 << WORD) | args->k0;
        if(STOP == 1){ 
            break;
        }

        /* printf("Start key %ld--- \n", k1k0); */
        /* clock_gettime(CLOCK_MONOTONIC, &start); */
        uint8_t valid_k1k0 = 0;
        bt_zero(vec_x1);
        for(uint64_t i=0UL; i<DATA_SETS; i++){
            set_counter_x1(args->ciphertexts[i], vec_x1, i, k1k0);
        }

        for(uint64_t k2=0UL; k2<(1UL << WORD); k2++){
            uint8_t valid_k2 = 0;
            bt_zero(vec_x2);
            for(uint64_t i=0UL; i<DATA_SETS; i++){
                set_counter_x2(vec_x1, vec_x2, i, k2);
            }
            for(uint64_t k3=0UL; k3<(1UL << WORD); k3++){
                uint8_t valid_k3 = 0;
                bt_zero(vec_x3);
                for(uint64_t i=0UL; i<DATA_SETS; i++){
                    set_counter_x3(vec_x2, vec_x3, i, k3);
                }
                for(uint64_t k4=0UL; k4<(1UL << WORD); k4++){
                    uint8_t final_sum = 0;
                    for(uint64_t i=0UL; i<DATA_SETS; i++){
                        sum[i] = total_sum(vec_x3, i, k4);
                        if(sum[i] != 0){
                            final_sum = 1;
                            break;
                        }
                    }
                    //printf("Final Sum = %d\n", final_sum);
                    if(final_sum == 0){
                        uint64_t k4k3k2k1k0 = k4;
                        k4k3k2k1k0 = (k4k3k2k1k0 << WORD) | k3;
                        k4k3k2k1k0 = (k4k3k2k1k0 << WORD) | k2;
                        k4k3k2k1k0 = (k4k3k2k1k0 << (2*WORD)) | k1k0;
                        args->recovered_key = k4k3k2k1k0;
                        args->found_key = 1;
                        printf("Key inside thread = %lx\n",k4k3k2k1k0);
                        STOP = 1;
                        valid_k3 = 1;
                        valid_k2 = 1;
                        valid_k1k0 = 1;
                        break;
                    }
                }
                if(valid_k3 == 1){
                    break;
                }
            }
            if(valid_k2 == 1){
                break;
            }
        }
        if(valid_k1k0 == 1){
            break;
        }
        /* clock_gettime(CLOCK_MONOTONIC, &end); */
        /* time_meter = ((double)(end.tv_sec - start.tv_sec)); */
        /* printf("Full Calculation Done for key %ld--- Elapsed time %f\n", k1k0, time_meter); */
        /* fflush(stdout); */
    } 
    bt_destroy(vec_x1);
    bt_destroy(vec_x2);
    bt_destroy(vec_x3);
    pthread_exit(NULL);
}

uint64_t Key_Guess_Using_Thread(bitmap **c, uint64_t k0){
    bitmap **ciphertexts = (bitmap **)malloc(DATA_SETS*sizeof(bitmap*));
    for(uint64_t j=0; j<DATA_SETS; j++){
        char fname[256];
        sprintf(fname, DATA_PATH_6, j);
        ciphertexts[j] = bt_load(fname);
    }

    pthread_t thread_ids[THREADS_PARTIAL_SUMS];
    PSArgs  thread_args[THREADS_PARTIAL_SUMS];

    uint64_t cube_size = (1ULL << WORD);
    uint64_t data_in_each_thread = (cube_size / THREADS_PARTIAL_SUMS);
    uint64_t data_in_last_thread = cube_size % THREADS_PARTIAL_SUMS; 
    for(int i=0; i<THREADS_PARTIAL_SUMS; i++){
        thread_args[i].ciphertexts = ciphertexts;
        thread_args[i].found_key = 0;
        thread_args[i].start = i * data_in_each_thread;
        if(i == THREADS-1){
            if(data_in_last_thread > 0){
                thread_args[i].end = i*data_in_each_thread + data_in_last_thread;
            }
            else{
                thread_args[i].end = (i+1)*data_in_each_thread;
            }
        }
        else{
            thread_args[i].end = (i+1)*data_in_each_thread;
        }
        //printf("Start = %lu\n",thread_args[i].start);
        //printf("End = %lu\n",thread_args[i].end);
        thread_args[i].k0 =  k0;
    }

    for(int i=0; i < THREADS_PARTIAL_SUMS; i++){
        pthread_create(thread_ids + i,NULL,key_guess_over_ranges,(void*)(thread_args + i));
    }
    for(int i=0; i < THREADS_PARTIAL_SUMS; i++){
        pthread_join(thread_ids[i], NULL);
    }
    for(int j=0; j<DATA_SETS; j++){
        bt_destroy(ciphertexts[j]);
    }
    free(ciphertexts);
    for(int i=0; i<THREADS_PARTIAL_SUMS; i++){
        if(thread_args[i].found_key==1){
            return (thread_args[i].recovered_key);
        }
    }
}

void find_key(){
    bitmap *ciphertexts[DATA_SETS];
    for(int i=0; i<DATA_SETS; i++){
        char fname[256];
        sprintf(fname, DATA_PATH_6, i); 
        ciphertexts[i] = bt_load(fname);
    }

    struct timespec start, end, start1, end1;
    double time_meter, time_meter1;

    printf("Key Guess Start\n");


    /*
    --------------------MAIN ALGORITM FOR KEY GUESS----------------
    */
    //Key Guess
    uint64_t k4k3k2k1k0; 
    clock_gettime(CLOCK_MONOTONIC, &start);
    #ifdef FIXK0
    for(uint64_t k0=0x9F; k0<(0x9F + 1); k0++){
    #else
    for(uint64_t k0=0UL; k0<256UL; k0++){
    #endif

        clock_gettime(CLOCK_MONOTONIC, &start1);
        printf("Start For K0 = %02lX \n", k0);

        k4k3k2k1k0 = Key_Guess_Using_Thread(ciphertexts, k0);

        clock_gettime(CLOCK_MONOTONIC, &end1);
        time_meter1 = ((double)(end1.tv_sec - start1.tv_sec));
        printf("Calculation Done --- Elapsed time %f\n", time_meter1);

        if(STOP == 1){
            break;
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    time_meter = ((double)(end.tv_sec - start.tv_sec));
    printf("Full Calculation Done --- Elapsed time %f\n", time_meter);
    printf("Key = %lx\n",k4k3k2k1k0);

    uint8_t key_6[16];
    memset(key_6, 0x00, 16);
    key_6[0]   = k4k3k2k1k0  & 0xFF;
    key_6[13]  = (k4k3k2k1k0 >> (WORD*1)) & 0xFF;
    key_6[10]  = (k4k3k2k1k0 >> (WORD*2)) & 0xFF;
    key_6[7]   = (k4k3k2k1k0 >> (WORD*3)) & 0xFF;
    uint8_t k4 = (k4k3k2k1k0 >> (WORD*4)) & 0xFF;
    printf("key_5_prime[0] : %02X\n", k4);
    printf( "key_6 : ");
    printreg(key_6, 16);
    for(int i=0; i<DATA_SETS; i++){
        bt_destroy(ciphertexts[i]);
    }
}

