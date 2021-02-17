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
        printf("Failed at line %d with %s (0x%lX)\n", __LINE__, pcsc_stringify_error(rv), rv); \
        return 0;
    }

    dwReaders = SCARD_AUTOALLOCATE;
    rv = SCardListReaders(hContext, NULL, (LPSTR)&mszReaders, &dwReaders);
    if (rv != SCARD_S_SUCCESS){
        printf("Failed at line %d with %s (0x%lX)\n", __LINE__, pcsc_stringify_error(rv), rv); \
        return 0;
    }

    /* use first reader */
    printf("Using reader: %s\n", mszReaders);

    rv = SCardConnect(hContext, mszReaders,
                      SCARD_SHARE_DIRECT, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
                      &hCard, &dwPref);
    if (rv != SCARD_S_SUCCESS){
        printf("Failed at line %d with %s (0x%lX)\n", __LINE__, pcsc_stringify_error(rv), rv); \
        return 0;
    }
    SCardFreeMemory(hContext, mszReaders);
    return 1;
}

void release_scard()
{
    SCardReleaseContext(hContext);
}