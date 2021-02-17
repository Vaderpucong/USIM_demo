//
// Created by DELL on 2021/2/15.
//

#include "history.h"

history_pool_t* init_history_pool(uint8_t pool_size)
{
    history_pool_t* history_pool_p = (history_pool_t*)malloc(sizeof(history_pool_t));
    assert(history_pool_p!=NULL);
    memset(history_pool_p , 0 , sizeof(history_pool_t));
    history_pool_p->pool_size = pool_size;
    history_pool_p->latest_item = 0;
    history_pool_p->items = (struct item_s*)malloc(sizeof(struct item_s)*pool_size);
    assert(history_pool_p->items!=NULL);
    memset(history_pool_p->items , 0 , sizeof(struct item_s)*pool_size);
    return history_pool_p;
}

void release_history_pool(history_pool_t** history_pool)
{
    assert(history_pool!=NULL);
    if(*history_pool == NULL)
        return ;
    if((*history_pool)->items != NULL) {
        for (uint8_t i = 0; i < (*history_pool)->pool_size; ++i) {
            if ((*history_pool)->items[i].apdu) free((*history_pool)->items[i].apdu);
            if ((*history_pool)->items[i].description) free((*history_pool)->items[i].description);
        }
        free((*history_pool)->items);
    }
    free(*history_pool);
    *history_pool = NULL;
}

void insert_history_item(history_pool_t* history_pool,
                         uint8_t* apdu_p,
                         uint8_t* description_p)
{
    assert(history_pool!=NULL);
    assert(apdu_p!=NULL);
    history_pool->latest_item = (history_pool->latest_item+1)%history_pool->pool_size;
    if(history_pool->items[history_pool->latest_item].apdu)
        free(history_pool->items[history_pool->latest_item].apdu);
    history_pool->items[history_pool->latest_item].apdu=apdu_p;

    if(history_pool->items[history_pool->latest_item].description)
        free(history_pool->items[history_pool->latest_item].description);
    history_pool->items[history_pool->latest_item].description = description_p;
}

uint8_t* get_latest_item(history_pool_t* history_pool)
{
    return (history_pool->items[history_pool->latest_item].apdu);
}

void display_history_pool(history_pool_t* history_pool)
{

}