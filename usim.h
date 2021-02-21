//
// Created by DELL on 2021/2/15.
//

#ifndef USIM_DEMO_USIM_H
#define USIM_DEMO_USIM_H

#include "commondef.h"
#include "history.h"

#if defined(_WIN64) | defined(_WIN32)
#include "usim_if_windows.h"
#elif defined(linux)
#include "usim_if_linux.h"
#else
//#include "usim_if_noPCSC.h"
#endif

typedef struct apdu{
    uint8_t apdu_size;
#define _NO_LC 0
#define _HAS_LC 1
    uint8_t has_Lc;
#define _NO_LE 0
#define _HAS_LE 1
    uint8_t has_Le;

    uint8_t CLA;
    uint8_t INC;
    uint8_t P1;
    uint8_t P2;
#define INVALID_LC 0x0u
#define INVALID_LE 0xFFu
    uint8_t Lc_data_Le[];
}apdu_t;

#define GTE_APDU_HEAD(t) ((uint8_t*)&(t->CLA))
#define GTE_APDU_LE(t) (t->has_Le==_HAS_LE?*(GTE_APDU_HEAD(t)+t->apdu_size-1):0x100u)

typedef struct usim_s{
    int (* send_apdu_usim)(uint8_t*,uint8_t);
    history_pool_t* history_pool;
}usim_t;

typedef struct r_apdu_s{
    uint8_t* r_apdu;
    uint16_t r_apdu_size;
    uint8_t sw1;
    uint8_t sw2;
}r_apdu_t;


void init_usim(usim_t* usim ,
               int (* send_apdu_usim)(uint8_t*,uint8_t),...);
void release_usim(usim_t* usim);
int send_command_usim(usim_t* usim,
                      uint8_t cla,
                      uint8_t ins,
                      uint8_t p1,
                      uint8_t p2,
                      uint8_t lc,
                      uint8_t* data,
                      uint8_t le,
                      r_apdu_t* r_apdu /* out */);
void release_r_apdu(r_apdu_t* r_apdu_p);


#endif //USIM_DEMO_USIM_H
