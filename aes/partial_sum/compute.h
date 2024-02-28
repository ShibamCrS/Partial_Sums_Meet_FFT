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
            sum ^= rsbox[x3 ^ k4];
        }
    }
    return sum;
}
