#include "guessK0.h"

uint8_t STOP=0;
void prepare_chi(bitmap *c, bitmap *chi, uint64_t k0){
    for(uint64_t c3c2c1=0UL; c3c2c1<SIZE3; c3c2c1++){
        for(uint64_t c0=0UL; c0<SIZE1; c0++ ){
            uint64_t c3c2c1c0 = (c3c2c1<<WORD) | c0;
            if(bt_get(c, c3c2c1c0) == 1){
                uint64_t c0_prime = SS0(c0 ^ k0);
                uint64_t c3c2c1c0_p = (c3c2c1<<WORD) | c0_prime;
                bt_set_parity(chi,c3c2c1c0_p);
            }
        }
    }
    /* bt_print(chi); */
}

/***************************************************************************
 *This function performs the main key guessing operation executed in parallel with
 *multiple threads starting from line 11 in algorithm 4. It is important to note that
 *there are four distinct key guess functions utilized: key_guess_0 for delta set 1,
 *key_guess_1 for delta set 2, key_guess_2 for delta set 3, and key_guess_345 for the
 *remaining delta sets. These key guess functions vary in their approach to checking
 *keys k2, k3, and k4. With each additional delta set processed, a significant portion
 *of potential keys for k2, k3, and k4 have already been eliminated, making it more
 *efficient to verify the remaining keys rather than computing the Fast Hadamard
 *Transform (FHT).
 * ************************************************************************/

void *guess_over_k0(void *args1){
    Args *args = (Args *) args1;
    bitmap *chi = bt_init(SIZE4);

     for(uint64_t k0=args->start; k0<args->end; k0++){ 
         if(STOP == 1){ 
             break; 
         } 
        bt_zero(args->RK);
        for(uint64_t i=0UL; i<DATA_SETS; i++){
            bt_zero(chi);
            prepare_chi(args->ciphertexts[i], chi, k0);
            
            //STEP1
            bt_zero(args->A);
            for(uint64_t c3=0UL; c3<SIZE1; c3++){
                fht_step_1(chi, args->A, c3, args->IS_A);
            }
            /* key_guess_0(args, chi); */

            if(i == 0){ 
                key_guess_0(args);
            }
            else if(i == 1){
                key_guess_1(args);
            }
            else if(i == 2){
                key_guess_2(args);
            }

            else {
                key_guess_345(args);
            }
        }
        uint64_t k1k2k3k4 = 0UL; 
        if(bt_first_zero(args->RK, &k1k2k3k4) == 1){
            /* bt_print(args->RK); */ 
            args->Key = (k0 << (4*WORD)) |  k1k2k3k4;
            printf("KEEEEY = %lX %04lX\n", k0, k1k2k3k4);
            
            args->found_key = 1;
            STOP = 1;
            break;
        }
        
    }
    /* for(int j=0; j<DATA_SETS; j++){ */
    /*     bt_destroy(args->ciphertexts[j]); */
    /* } */
    /* free(args->ciphertexts); */
    bt_destroy(chi);
    bt_destroy(args->A);
    bt_destroy(args->B);
    bt_destroy(args->C);
    bt_destroy(args->RK);
    /* free(args->IS_A); */
    /* free(args->IS_B); */
    /* free(args->IS_C); */
    /* free(args->IS_D); */
    pthread_exit(NULL);
}

