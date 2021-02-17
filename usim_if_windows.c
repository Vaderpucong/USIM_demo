//
// Created by DELL on 2021/2/15.
//

#include "usim_if_windows.h"

SCARDCONTEXT hContext;
SCARDHANDLE hCard;

int init_scard()
{
    LONG rv;
    int value, ret = -1;
    DWORD dwReaders, dwPref;
    char *mszReaders;

    rv = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &hContext);
    if (rv != SCARD_S_SUCCESS){
        printf("Failed at line %d\n", __LINE__);
        return 0;
    }

    dwReaders = SCARD_AUTOALLOCATE;
    rv = SCardListReaders(hContext, NULL, (LPSTR)&mszReaders, &dwReaders);
    if (rv != SCARD_S_SUCCESS){
        printf("Failed at line %d\n", __LINE__);
        return 0;
    }

    /* use first reader */
    printf("Using reader: %s\n", mszReaders);

    rv = SCardConnect(hContext, mszReaders,
                      SCARD_SHARE_DIRECT, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                      &hCard, &dwPref);
    if (rv != SCARD_S_SUCCESS){
        printf("Failed at line %d\n", __LINE__);
        return 0;
    }
    SCardFreeMemory(hContext, mszReaders);
    return 1;
}

void release_scard()
{
    SCardReleaseContext(hContext);
    SCardDisconnect	(hCard,SCARD_RESET_CARD);
}

int send_apdu_usim(uint8_t* apdu_p , uint8_t apdu_size)
{
    LONG rv=0;
    memset(&r_tpdu,0,sizeof(struct r_tpdu_s));
    r_tpdu.r_tpdu_size = sizeof(r_tpdu.r_tpdu);
    rv = SCardTransmit(hCard,SCARD_PCI_RAW,apdu_p,apdu_size,
                       NULL,r_tpdu.r_tpdu,(LPDWORD)&r_tpdu.r_tpdu_size);
    if (rv != SCARD_S_SUCCESS){
        printf("Failed at line %d\n", __LINE__);
        return 0;
    }

    return 1;
}