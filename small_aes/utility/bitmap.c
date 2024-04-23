#include "bitmap.h"

int bt_init_ptr(bitmap *bt, uint64_t nrof_bits){
    // set the number of bits to store and allocate memory to store the bits 
    bt->nrof_bits = nrof_bits;
    bt->bits = calloc(nrof_bits/64 + 1, sizeof(uint64_t));

    if(bt->bits == NULL){
        printf("ERROR: Could not allocate memory for bits\n");
        return 0;
    }

    return 1;
}

bitmap* bt_init(uint64_t nrof_bits){
    /*
     * Initialise the bitmap and set all bits to 0
     */
    bitmap *bt;
    bt = calloc(1, sizeof(bitmap));
    if(bt == NULL){
        printf("ERROR: Could not allocate memory for bitmap\n");
        return NULL;
    }
    if(!bt_init_ptr(bt, nrof_bits)){
        return NULL;
    }
    return bt;
}

void bt_destroy(bitmap *bt){
    free(bt->bits);
    free(bt);
}

uint8_t bt_get(bitmap *bt, uint64_t index){
    uint64_t word = (index/64);
    uint8_t bit = (uint8_t)(index % 64);
    if(index > bt->nrof_bits){
       return 2;
    } 
    return (uint8_t)(bt->bits[word] >> bit) & 0x1;
}

int bt_set(bitmap *bt, uint64_t index, uint8_t value){
    /*
     *  Sets the bit on <index> to value
     *
     *  returns 1 on success returns 0 on failure
     */
    uint64_t word = (index/64);
    uint8_t bit = (uint8_t)(index % 64);
    uint64_t mask = ((uint64_t) 0x1ULL)  << bit;
    uint64_t value_mask = ((uint64_t) value & 0x1ULL)  << bit;
    if(index > bt->nrof_bits){
        return 0;
    }
    bt->bits[word] &= ~mask; // first set the bit to 0
    bt->bits[word] |= value_mask; // then set the bit to the value given
    return 1;
}

void bt_set_1(bitmap *bt, uint64_t index){
    uint64_t word = (index >> 6);
    uint8_t bit = (uint8_t)(index & 0b111111);
    
    bt->bits[word] |= (1ULL << bit);
}

void bt_set_parity(bitmap *bt, uint64_t index){
    uint64_t word = (index >> 6);
    uint8_t bit = (uint8_t)(index & 0b111111);
    
    bt->bits[word] ^= (1ULL << bit);
}

void bt_flip(bitmap *bt, uint64_t index){
    uint64_t word = (index/64);
    uint8_t bit = (uint8_t)(index % 64);
    uint64_t mask = ((uint64_t) 0x1)  << bit;
    bt->bits[word] ^= mask; // then set the bit to the value given
}

void bt_flip_all(bitmap *bt){
    uint64_t word_index;
    for(word_index=0; word_index < bt->nrof_bits/64; word_index++){
        bt->bits[word_index] = ~bt->bits[word_index];
    }
}

int bt_first_zero(bitmap *bt, uint64_t *index){
    uint64_t word_index, bit_index, word;
    word=0;
    // TODO we can remove one comparison with a sentinel block
    for(word_index=0; word_index < bt->nrof_bits/64; word_index++){
        word = bt->bits[word_index];
        if(~word){
            break;
        }
    }
    // now we are at the first block containing a 0
    for(bit_index=0; bit_index < 64; bit_index++){
        if( !((word >> bit_index) & 0x1) ){
            *index = word_index*64 + bit_index;
            return 1;
        }
    }
    return 0;
}

int bt_next_zero(bitmap *bt, uint64_t *index){
    uint64_t word_index, bit_index, word, first_index, first_bitindex;
    if(*index >= (bt->nrof_bits)){
        /* printf("ERROR: NZ index out of range %016llX (%16llX)\n", *index, bt->nrof_bits); */
        return 0;
    }
    first_index = (*index / 64);
    first_bitindex = (*index % 64);
    word = bt->bits[first_index];
    if(first_bitindex){
        word |= (0xFFFFFFFFFFFFFFFF >> (64-first_bitindex));
    }
    word = ~word;

    
    for(word_index=first_index; word_index < (bt->nrof_bits/64); word_index++){
        if(word){
            // now find the bit
            // returns the least significant 1 bit + 1
            *index = __builtin_ffsl(word) - 1 + word_index*64;
            return 1;
        }
        word = ~bt->bits[word_index+1];
    }
    
    //printf("Reached the end: %016llX (%lu)\n", word_index, bit_index);
    return 0;
}

uint64_t bt_popcount(uint64_t word){
    uint32_t word_low, word_high;
    word_low = word & 0xFFFFFFFF;
    word_high = (word >> 32) & 0xFFFFFFFF;
    return __builtin_popcount(word_low) + __builtin_popcount(word_high);
} 

void bt_zero(bitmap *bt){
    memset(bt->bits, 0, (bt->nrof_bits/64 + 1)*sizeof(uint64_t));
}

/*
uint64_t bt_popcount(uint64_t word){
    // Use this if the words are very sparse, note that by pruning you get one 
    // Very dense bitmap (the one after the pruning happened
    uint64_t c;
    for(c=0; word; c++){
        word &= word-1;
    }
    return c;
} 
*/

uint64_t bt_hamming_weight(bitmap *bt, uint64_t index_from, uint64_t index_to){
    uint64_t word_to, bit_to, word, i;
    uint64_t word_from, bit_from;
    uint64_t hamming_weight=0;
    
    if(index_to > bt->nrof_bits){
        index_to = bt->nrof_bits;
    }
    /*
    if(index_from > index_to){
        printf("FATAL ERROR: hamming_weight\n");
        exit(1);
    }
    */
    
    word_from = index_from/64;
    bit_from = index_from%64;

    word_to = index_to/64;
    bit_to = index_to%64;

    word = bt->bits[word_from];
    word = (bit_from >= 64) ? 0 : ((word >> bit_from) << bit_from);

    if(word_from == word_to){
        word = (bit_to == 0) ? 0 : (word << (64-bit_to));
        return bt_popcount(word);
    }

    hamming_weight += bt_popcount(word);

    for(i=word_from+1; i < word_to; i++){
        word = bt->bits[i];
        hamming_weight += bt_popcount(word);
    }
    word = (bit_to == 0) ? 0 : (bt->bits[word_to] << (64-bit_to));
    hamming_weight += bt_popcount(word); 

    return hamming_weight;
} 

int bt_save(bitmap *bt, char *fname){
    FILE *f;
    size_t status;
    f = fopen(fname, "wb"); 
    if(f == NULL){
        printf("ERROR: Could not open file for saving [%s]\n", fname);
        return 0;
    }
    // Write the bitmap
    status = fwrite(bt, sizeof(bitmap), 1, f);
    if(status != 1){
        printf("ERROR: could not write bitmap [%lu]\n", status);
        return 0;
    }
    // Write the bits in the bitmap
    status = fwrite(bt->bits, sizeof(uint64_t), bt->nrof_bits/64 + 1, f);
    if(status != bt->nrof_bits/64 + 1){
        printf("ERROR: could not write bits [%lu]\n", status);
        return 0;
    }
    fclose(f); 
    return 1;
}

bitmap* bt_load(char *fname){
    FILE *f;
    size_t status;
    bitmap *bt, *bt_tmp;
    f = fopen(fname, "rb"); 
    if(f == NULL){
        printf("ERROR: Could not open file for loading [%s]\n", fname);
        return NULL;
    }
    //bt = malloc(sizeof(bitmap));
    bt_tmp = malloc(sizeof(bitmap));

    // Load the bitmap
    //status = fread(bt, sizeof(bitmap), 1, f);
    status = fread(bt_tmp, sizeof(bitmap), 1, f);
    if(status != 1){
        printf("ERROR: Could not read bitmap [%lu]\n", status);
        return NULL;
    }

    // initialise the bitmap
    //printf("Load size: %ld \n", (unsigned long)bt_tmp->nrof_bits);
    bt = bt_init(bt_tmp->nrof_bits);
    // free the temporary bitmap var
    free(bt_tmp);

    if(bt == NULL){
        return NULL;
    }
    
    // load the bits
    status = fread(bt->bits, sizeof(uint64_t), bt->nrof_bits/64 + 1, f);
    if(status != bt->nrof_bits/64 + 1){
        printf("ERROR: Could not read bits [%lu]\n", status);
        return 0;
    }
    fclose(f); 
    return bt;
}

void bt_print(bitmap *bt){
    uint64_t i;
    for(i=0; i < bt->nrof_bits; i++){
        printf("%01u", (bt_get(bt, i) & 1));
        if(i > 300){
            printf(" ... ");
            for(i=bt->nrof_bits-8; i < bt->nrof_bits; i++){
                printf("%01u", (bt_get(bt, i) & 1));
            }
            break;
        }
        if( i % 8 == 7 ){
            printf(" ");
        }
    }
    printf("\n");
}

uint32_t bt_size(bitmap *bt){
    /* returns the number of bytes used to store this bitmap */
    return (bt->nrof_bits/64 + 1) * sizeof(uint64_t) + sizeof(bitmap);
}
