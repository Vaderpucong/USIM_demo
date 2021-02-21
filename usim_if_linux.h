//
// Created by DELL on 2021/2/15.
//

#ifndef USIM_DEMO_USIM_IF_LINUX_H
#define USIM_DEMO_USIM_IF_LINUX_H

#include <PCSC/winscard.h>
#include "commondef.h"

struct r_tpdu_s{
    uint8_t r_tpdu[256+2];
    uint16_t r_tpdu_size;
}r_tpdu;

/*
typedef struct response_s{
    uint8_t SW1;
    uint8_t SW2;
    uint8_t data_len;
    uint8_t *data;
}RESPONSE_T, *RESPONSE_P;
*/

int init_scard();
void release_scard();
int send_apdu_usim(uint8_t* apdu_p , uint8_t apdu_size);

#endif //USIM_DEMO_USIM_IF_LINUX_H
