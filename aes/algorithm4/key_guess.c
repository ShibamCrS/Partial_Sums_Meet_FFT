#include "key_guess.h"

void key_guess_0(void *args1, bitmap *chi){
    Args *args = (Args *) args1;
    //key guess
    for(uint64_t k1=0UL; k1<SIZE1; k1++){
        bt_zero(args->B);
        fht_step_2(args->A, args->B, k1, args->IS_B);
        //key guess
        bt_zero(args->C);
        fht_step_3( args->B, args->C, args->IS_C);
        //Key Guess
        for(uint64_t k2=0UL; k2<SIZE1; k2++){
            //key guess
            for(uint64_t k3=0UL; k3<SIZE1; k3++){
                uint8_t S[(1 << WORD)];
                memset(S, 0x00, (1 << WORD));
                fht_step_4(args->C, S, k3, k2, args->IS_D);
                for(uint64_t k4=0UL; k4<(1UL<<WORD); k4++){
                    if(S[k4] != 0){ 
                        uint64_t pos=k1;
                        pos=(pos<<WORD) | k2; 
                        pos=(pos<<WORD) | k3; 
                        pos=(pos<<WORD) | k4; 
                        bt_set_1(args->RK, pos);
                    }   
                }   
            }   
        }   
    }   
}
void key_guess_1(void *args1, bitmap *chi){
    Args *args = (Args *) args1;
    //key guess
    for(uint64_t k1=0UL; k1<SIZE1; k1++){
        bt_zero(args->B);
        fht_step_2(args->A, args->B, k1, args->IS_B);
        
        bt_zero(args->C);
        fht_step_3( args->B, args->C, args->IS_C);
        for(uint64_t k2=0UL; k2<SIZE1; k2++){
            for(uint64_t k3=0UL; k3<SIZE1; k3++){
                
                for(uint64_t k4=0; k4<SIZE1; k4++){
                    uint64_t pos=k1;
                    pos=(pos<<WORD) | k2;
                    pos=(pos<<WORD) | k3;
                    pos=(pos<<WORD) | k4;
                    if(bt_get(args->RK, pos) == 0){
                        uint64_t sum = 0UL;
                        for(uint64_t c=0UL; c<SIZE1; c++){
                            uint64_t pos1 = (k2*SIZE2) + (k3*SIZE1) + c;
                            if(bt_get(args->C, pos1) == 1){
                                sum = sum ^ SS4(c ^ k4);
                            }
                        }
                        if(sum != 0){
                            bt_set_1(args->RK, pos);
                        }
                    }
                }
            }   
        }   
    }   
}
void key_guess_2(void *args1, bitmap *chi){
    Args *args = (Args *) args1;
    //key guess
    for(uint64_t k1=0UL; k1<SIZE1; k1++){
        bt_zero(args->B);
        fht_step_2(args->A, args->B, k1, args->IS_B);
       
        for(uint64_t k2=0UL; k2<SIZE1; k2++){
            for(uint64_t k3=0UL; k3<SIZE1; k3++){
                for(uint64_t k4=0; k4<SIZE1; k4++){
                    uint64_t pos=k1;
                    pos=(pos<<WORD) | k2;
                    pos=(pos<<WORD) | k3;
                    pos=(pos<<WORD) | k4;
                    if(bt_get(args->RK, pos) == 0){
                        uint64_t sum = 0UL;
                        for(uint64_t c3=0UL; c3<SIZE1; c3++){
                            for(uint64_t b=0UL; b<SIZE1; b++){
                                uint64_t c3b = (c3 << WORD) + b;
                                uint64_t pos = (k2*SIZE2) + c3b;
                                if(bt_get(args->B, pos) == 1){
                                    uint64_t c = b ^ SS3(c3 ^ k3);
                                    sum = sum ^ SS4(k4 ^ c);
                                }
                            }
                        }
                        if(sum != 0){
                            bt_set_1(args->RK, pos);
                        }
                    }
                }
            }
        }   
    }   
}
void key_guess_345(void *args1, bitmap *chi){
    Args *args = (Args *) args1;
    //key guess
    for(uint64_t k1=0UL; k1<SIZE1; k1++){

        for(uint64_t k2=0UL; k2<SIZE1; k2++){
            for(uint64_t k3=0UL; k3<SIZE1; k3++){
                for(uint64_t k4=0; k4<SIZE1; k4++){
                    uint64_t pos=k1;
                    pos=(pos<<WORD) | k2;
                    pos=(pos<<WORD) | k3;
                    pos=(pos<<WORD) | k4;
                    if(bt_get(args->RK, pos) == 0){
                        uint64_t sum = 0UL;
                        for(uint64_t c3=0UL; c3<SIZE1; c3++){
                            for(uint64_t c2=0UL; c2<SIZE1; c2++){
                                for(uint64_t a=0UL; a<SIZE1; a++){
                                    uint64_t pos1 = k1*SIZE3+ a*SIZE2 + c3*SIZE1 + c2;
                                    if(bt_get(args->A, pos1) == 1){
                                        uint64_t b = a ^ SS2(c2 ^ k2);
                                        uint64_t c = b ^ SS3(c3 ^ k3);
                                        sum = sum ^ SS4(k4 ^ c);
                                    }
                                }
                            }
                        }
                        if(sum != 0){
                            bt_set_1(args->RK, pos);
                        }
                    }
                }
            }
        }
    }
}   
