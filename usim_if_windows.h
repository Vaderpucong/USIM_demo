//
// Created by DELL on 2021/2/15.
//

#ifndef USIM_DEMO_USIM_IF_WINDOWS_H
#define USIM_DEMO_USIM_IF_WINDOWS_H

#include <winscard.h>

struct r_tpdu_t{
    uint8_t r_tpdu[255];
    uint16_t r_tpdu_size;
}r_tpdu;

int init_scard();
void release_scard();

#endif //USIM_DEMO_USIM_IF_WINDOWS_H
