#include "key_guess.h"
void* key_guess_0(void *args1){
    Step2Args *args = (Step2Args*) args1;
    double IS_B[1ULL << (WORD*2)];
    circuit_evaluation_2(IS_B);
    double IS_C[1ULL << (WORD*2)];
    circuit_evaluation_3(IS_C);
    uint64_t IS_D[1ULL << WORD];
    circuit_evaluation_4(IS_D);
    
    uint64_t len_b = (1UL<<WORD) * (1UL << (WORD*2));
    bitmap *B = bt_init(len_b);
    uint64_t len_c = (1UL << WORD) * (1UL << (2*WORD));
    bitmap *C = bt_init(len_c);

    for(uint64_t k1k0=args->start; k1k0<args->end;  k1k0+= 1){ 
        bt_zero(B);
        fht_step_2(args->A, B, k1k0, IS_B);
        //Compute for all k2 to get a speed up using G trick
        bt_zero(C);
        fht_step_3( B, C, IS_C );

        //Key Guess
        for(uint64_t k2=0UL; k2<(1<<WORD); k2++){
            //key guess
            for(uint64_t k3=0UL; k3<(1UL<<WORD); k3++){
                uint8_t S[(1 << WORD)];
                memset(S, 0x00, (1 << WORD));
                fht_step_4(C, S, k3, k2, IS_D);
    
                for(uint64_t k4=0UL; k4<(1UL<<WORD); k4++){
                    if(S[k4] != 0){ 
                        uint64_t k0 = k1k0 & 0xFF;
                        uint64_t k1 = (k1k0 >> WORD) & 0xFF;
                        uint64_t pos=k0;
                        pos=(pos<<WORD) | k1; 
                        pos=(pos<<WORD) | k2; 
                        pos=(pos<<WORD) | k3;
                        pos=(pos<<WORD) | k4;
                        bt_set_1(args->RemovedKeys, pos);
                    }
                }
            }
        }
    }
    bt_destroy(C);
    bt_destroy(B);
    pthread_exit(NULL);
}

void* key_guess_1(void *args1){
    Step2Args *args = (Step2Args*) args1;
    double IS_B[1ULL << (WORD*2)];
    circuit_evaluation_2(IS_B);
    double IS_C[1ULL << (WORD*2)];
    circuit_evaluation_3(IS_C);
    
    uint64_t len_b = (1UL<<WORD) * (1UL << (WORD*2));
    bitmap *B = bt_init(len_b);
    uint64_t len_c = (1UL << WORD) * (1UL << (2*WORD));
    bitmap *C = bt_init(len_c);

    //uint64_t count=0UL;
    for(uint64_t k1k0=args->start; k1k0<args->end;  k1k0+= 1){
        bt_zero(B);
        fht_step_2(args->A, B, k1k0, IS_B);
        
        //Compute for all k2 to get a speed up using G trick
        bt_zero(C);
        fht_step_3( B, C, IS_C );
        for(uint64_t k2=0UL; k2<(1UL<<WORD); k2++){
            for(uint64_t k3=0UL; k3<(1UL<<WORD); k3++){

                for(uint64_t k4=0; k4<(1UL<<WORD); k4++){
                    uint64_t k0 = k1k0 & 0xFF;
                    uint64_t k1 = (k1k0 >> WORD) & 0xFF;
                    uint64_t pos=k0;
                    pos=(pos<<WORD) | k1; 
                    pos=(pos<<WORD) | k2; 
                    pos=(pos<<WORD) | k3; 
                    pos=(pos<<WORD) | k4; 
                    if(bt_get(args->RemovedKeys, pos) == 0){
                        /* count++; */
                        uint64_t sum = 0UL;
                        for(uint64_t c=0UL; c<(1UL<<WORD); c++){
                            uint64_t pos1 = (k2*(1UL << (WORD*2))) + (k3*(1UL<<WORD)) + c;
                            if(bt_get(C, pos1) == 1){
                                sum = sum ^ SS4(c ^ k4);
                            }
                        }
                        if(sum != 0){
                            bt_set_1(args->RemovedKeys, pos);
                        }
                    }
                }
            }
        }
    }
    bt_destroy(C);
    bt_destroy(B);
    pthread_exit(NULL);
}
void* key_guess_2(void *args1){
    Step2Args *args = (Step2Args*) args1;
    double IS_B[1ULL << (WORD*2)];
    circuit_evaluation_2(IS_B);
    
    uint64_t len_b = (1UL<<WORD) * (1UL << (WORD*2));
    bitmap *B = bt_init(len_b);

    for(uint64_t k1k0=args->start; k1k0<args->end;  k1k0+= 1){
        bt_zero(B);
        fht_step_2(args->A, B, k1k0, IS_B);
        for(uint64_t k2=0UL; k2<(1UL<<WORD); k2++){

            for(uint64_t k3=0UL; k3<(1UL<<WORD); k3++){
                for(uint64_t k4=0; k4<(1UL<<WORD); k4++){
                    uint64_t k0 = k1k0 & 0xFF;
                    uint64_t k1 = (k1k0 >> WORD) & 0xFF;
                    uint64_t pos=k0;
                    pos=(pos<<WORD) | k1; 
                    pos=(pos<<WORD) | k2; 
                    pos=(pos<<WORD) | k3; 
                    pos=(pos<<WORD) | k4; 
                    if(bt_get(args->RemovedKeys, pos) == 0){
                        /* count++; */
                        uint64_t sum = 0UL;
                        for(uint64_t c3b=0UL; c3b<(1UL << (WORD*2)); c3b++){
                            uint64_t pos = (k2*(1UL<<(WORD*2)))+c3b;
                            if(bt_get(B, pos) == 1){
                                uint64_t c3 = (c3b >> WORD) & 0xFF;
                                uint64_t b = c3b & 0xFF;
                                uint64_t c = b ^ SS3(c3 ^ k3);
                                sum = sum ^ SS4(k4 ^ c);
                            }
                        }
                        if(sum != 0){
                            bt_set_1(args->RemovedKeys, pos);
                        }
                    }
                }
            }
        }
    }
    bt_destroy(B);
    pthread_exit(NULL);
}
void* key_guess_345(void *args1){
    Step2Args *args = (Step2Args*) args1;
    /* uint64_t count = 0UL; */
    for(uint64_t k1k0=args->start; k1k0<args->end;  k1k0+= 1){

        for(uint64_t k2=0UL; k2<(1UL<<WORD); k2++){
            for(uint64_t k3=0UL; k3<(1UL<<WORD); k3++){
                for(uint64_t k4=0; k4<(1UL<<WORD); k4++){
                    uint64_t k0 = k1k0 & 0xFF;
                    uint64_t k1 = (k1k0 >> WORD) & 0xFF;
                    uint64_t pos=k0;
                    pos=(pos<<WORD) | k1; 
                    pos=(pos<<WORD) | k2; 
                    pos=(pos<<WORD) | k3; 
                    pos=(pos<<WORD) | k4; 
                    if(bt_get(args->RemovedKeys, pos) == 0){
                        /* count++; */
                        uint64_t sum = 0UL;
                        for(uint64_t c3c2a=0UL; c3c2a<(1UL << (WORD*3)); c3c2a++){
                            uint64_t pos = (k1k0*(1UL<<(WORD*3)))+c3c2a;
                            if(bt_get(args->A, pos) == 1){
                                uint64_t c3 = (c3c2a >> (2*WORD)) & 0xFF;
                                uint64_t c2 = (c3c2a >> WORD) & 0xFF;
                                uint64_t a = c3c2a & 0xFF;
                                uint64_t b = a ^ SS2(c2 ^ k2);
                                uint64_t c = b ^ SS3(c3 ^ k3);
                                sum = sum ^ SS4(k4 ^ c);
                            }
                        }
                        if(sum != 0){
                            bt_set_1(args->RemovedKeys, pos);
                        }
                    }
                }
            }
        }
    }
    pthread_exit(NULL);
}

/***************************************************************************
 *This function performs the main key guessing operation executed in parallel with
 *multiple threads starting from line 6 in algorithm 3. It is important to note that
 *there are four distinct key guess functions utilized: key_guess_0 for delta set 1,
 *key_guess_1 for delta set 2, key_guess_2 for delta set 3, and key_guess_345 for the
 *remaining delta sets. These key guess functions vary in their approach to checking
 *keys k2, k3, and k4. With each additional delta set processed, a significant portion
 *of potential keys for k2, k3, and k4 have already been eliminated, making it more
 *efficient to verify the remaining keys rather than computing the Fast Hadamard
 *Transform (FHT).
 * ************************************************************************/
uint64_t filter_keys(bitmap *A, bitmap *RemovedKeys, uint64_t data_set_index){
    pthread_t thread_ids[THREADS];
    Step2Args  thread_args[THREADS];

    uint64_t cube_size = (1ULL << (WORD*2));
    uint64_t data_in_each_thread = (cube_size / THREADS);

    for(int i=0; i<THREADS; i++){
        thread_args[i].A = A;
        thread_args[i].RemovedKeys = RemovedKeys;
        thread_args[i].start = i * data_in_each_thread;
        thread_args[i].end = (i+1) * data_in_each_thread;
    }

    if(data_set_index == 0){
        for(int i=0; i < THREADS; i++){
            pthread_create(thread_ids + i,NULL,key_guess_0,(void*)(thread_args + i));
        }
    }
    else if(data_set_index == 1){
        for(int i=0; i < THREADS; i++){
            pthread_create(thread_ids + i,NULL,key_guess_1,(void*)(thread_args + i));
        }
    }
    else if(data_set_index == 2){
        for(int i=0; i < THREADS; i++){
            pthread_create(thread_ids + i,NULL,key_guess_2,(void*)(thread_args + i));
        }
    }
    else{
        for(int i=0; i < THREADS; i++){
            pthread_create(thread_ids + i,NULL,key_guess_345,(void*)(thread_args + i));
        }
    }

    for(int i=0; i < THREADS; i++){
        pthread_join(thread_ids[i], NULL);
    }
}

