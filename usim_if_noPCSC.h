//
// Created by DELL on 2021/2/15.
//

#ifndef USIM_DEMO_USIM_IF_NOPCSC_H
#define USIM_DEMO_USIM_IF_NOPCSC_H

#include "commondef.h"

struct r_tpdu_t{
    uint8_t r_tpdu[256];
    uint16_t r_tpdu_size;
}r_tpdu;

int init_scard();
void release_scard();
int send_apdu_usim(uint8_t* apdu_p , uint8_t apdu_size);

#endif //USIM_DEMO_USIM_IF_NOPCSC_H
