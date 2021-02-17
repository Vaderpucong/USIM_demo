//
// Created by DELL on 2021/2/15.
//

#include "usim.h"

/***************************internal data*******************************/



/***************************internal functions*******************************/
/*
 * create a apdu command and return a pointer type of apdu_t
 * no need to pass data size ,because lc equal data size
 */
static apdu_t* _new_apdu_command(
        uint8_t cla,
        uint8_t ins,
        uint8_t p1,
        uint8_t p2,
        uint8_t lc,
        uint8_t* data,
        uint8_t le){

    if(data!=NULL && lc==INVALID_LC)//lc should equal to data size
        return NULL;
    uint8_t _index=0;
    apdu_t* _apdu_p = (apdu_t*)malloc(sizeof(apdu_t)+lc+(le==INVALID_LE?0:1));
    assert(_apdu_p != NULL);

    _apdu_p->CLA=cla;
    _apdu_p->INC=ins;
    _apdu_p->P1=p1;
    _apdu_p->P2=p2;
    _apdu_p->has_Lc = (lc==INVALID_LC?_NO_LC:_HAS_LC);
    _apdu_p->has_Le = (le==INVALID_LE?_NO_LE:_HAS_LE);
    if(data != NULL)
    {
        _apdu_p->Lc_data_Le[_index++] = lc;
        memcpy(_apdu_p->Lc_data_Le+1 , data , lc);
        _index+=lc;
    }
    if(le != INVALID_LE)
        _apdu_p->Lc_data_Le[_index++] = le;
    _apdu_p->apdu_size = _index+4; //add cla,ins,p1,p2

    return _apdu_p;
}

static int _send_apdu(usim_t* usim,r_apdu_t* r_apdu_p /* out */)
{
    apdu_t* _apdu_p = (apdu_t*)get_latest_item(usim->history_pool);
    if(0 == usim->send_apdu_usim(GTE_APDU_HEAD(_apdu_p),_apdu_p->apdu_size))
        return 0;
    switch(r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-2]) //SW1
    {
        case 0x61: {
            //send GET RESPONSE command
            uint8_t _xx = GTE_APDU_LE(_apdu_p)<r_tpdu.r_tpdu[r_tpdu.r_tpdu_size - 1]?
                         GTE_APDU_LE(_apdu_p):r_tpdu.r_tpdu[r_tpdu.r_tpdu_size - 1];

            insert_history_item(usim->history_pool,
                                (uint8_t *) _new_apdu_command(0x00, 0xC0,
                                                              0x00, 0x00,
                                                              INVALID_LC, NULL,
                                                              _xx/* SW2 */),
                                NULL);
            if (0 == _send_apdu(usim, r_apdu_p /* out */))
                return 0;
            break;
        }
        case 0x6C: {
            //resend previous command header
            apdu_t* _apdu_previous = (apdu_t*)get_latest_item(usim->history_pool);

            insert_history_item(usim->history_pool,
                                (uint8_t *) _new_apdu_command(_apdu_previous->CLA,
                                                              _apdu_previous->INC,
                                                              _apdu_previous->P1,
                                                              _apdu_previous->P2,
                                                              INVALID_LC,
                                                              NULL,
                                                              r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-1]/* SW2 */),
                                NULL);
            if (0 == _send_apdu(usim, r_apdu_p /* out */))
                return 0;
            break;
        }
        case 0x62:
        case 0x63:
        case 0x91:
        case 0x92:
            //Case4
            if(_apdu_p->has_Lc == _HAS_LC &&
                _apdu_p->has_Le == _HAS_LE)
            {
                //decode status words
                insert_history_item(usim->history_pool,
                                    (uint8_t *) _new_apdu_command(0x00, 0xC0,
                                                                  0x00, 0x00,
                                                                  INVALID_LC, NULL,
                                                                  0x00/* SW2 */),
                                    NULL);
                if (0 == _send_apdu(usim, r_apdu_p /* out */))
                    return 0;
                break;
            }
        default:
            //decode status words
            assert(r_apdu_p != NULL);
            memset(r_apdu_p , 0 , sizeof(r_apdu_t));
            r_apdu_p->r_apdu_size = r_tpdu.r_tpdu_size;
            r_apdu_p->r_apdu = (uint8_t*)malloc(r_tpdu.r_tpdu_size);
            assert(r_apdu_p->r_apdu != NULL);
            memcpy(r_apdu_p->r_apdu, r_tpdu.r_tpdu , r_apdu_p->r_apdu_size);
            break;
    }
    return 1;
}


/***************************global functions*****************************/
void init_usim(usim_t* usim ,
               int (* send_apdu_usim)(uint8_t*,uint8_t),...)
{
    if(0 == init_scard())
        return ;
    assert(usim!=NULL && send_apdu_usim!=NULL);
    usim->send_apdu_usim = send_apdu_usim;
    usim->history_pool = init_history_pool(10);
    assert(usim->history_pool != NULL);
}

void release_usim(usim_t* usim)
{
    assert(usim != NULL);
    usim->send_apdu_usim = NULL;
    release_history_pool(&usim->history_pool);
    release_scard();
}


int send_command_usim(usim_t* usim,
              uint8_t cla,
              uint8_t ins,
              uint8_t p1,
              uint8_t p2,
              uint8_t lc,
              uint8_t* data,
              uint8_t le,
              r_apdu_t* r_apdu /* out */)
{
    assert(usim != NULL);
    //derive apdu description
    insert_history_item(usim->history_pool,
                        (uint8_t*)_new_apdu_command(cla,ins,p1,p2,lc,data,le),
                        NULL);
    if(0 == _send_apdu(usim,r_apdu))
        return 0;

    return 1;
}

void release_r_apdu(r_apdu_t* r_apdu_p)
{
    assert(r_apdu_p != NULL);
    if(r_apdu_p->r_apdu) free(r_apdu_p->r_apdu);
    r_apdu_p->r_apdu_size = 0;
}