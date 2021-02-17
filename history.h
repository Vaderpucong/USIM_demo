//
// Created by DELL on 2021/2/15.
//

#ifndef USIM_DEMO_HISTORY_H
#define USIM_DEMO_HISTORY_H

#include "commondef.h"

struct item_s{
    uint8_t * apdu;
    uint8_t * description; //describe apdu
}item;

typedef struct history_s{
    uint8_t pool_size;
    uint8_t latest_item; //pointer to a previous item
    struct item_s* items;
}history_pool_t;


history_pool_t* init_history_pool(uint8_t pool_size);
void release_history_pool(history_pool_t** history_pool);
void insert_history_item(history_pool_t* history_pool,
                         uint8_t* apdu_p,
                         uint8_t* description_p);
uint8_t* get_latest_item(history_pool_t* history_pool);
void display_history_pool(history_pool_t* history_pool);

#endif //USIM_DEMO_HISTORY_H
