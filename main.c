//
// Created by DELL on 2021/2/15.
//

#include <stdio.h>
#include "usim.h"

int main(int argc , char** argv)
{
    usim_t usim={0};
    r_apdu_t r_apdu;

    init_usim(&usim ,send_apdu_usim);

    /* test case
     * select IMSI EF
     * read IMSI EF
     */
    uint8_t imsi[2]={0x6F,0x07};
    send_command_usim(&usim,0x00,0xA4,0x00,0x04,0x02,imsi,
                      INVALID_LE,&r_apdu);
    release_r_apdu(&r_apdu);

    send_command_usim(&usim,0x00,0xB0,0x00,0x00,INVALID_LC,NULL,
                      INVALID_LE,&r_apdu);
    printf("IMSI size is %u.\n",r_apdu.r_apdu_size);
    for(uint8_t i=0;i<r_apdu.r_apdu_size;++i)
    {
        printf("0X%02X-",r_apdu.r_apdu[i]);
    }
    printf("\n");
    release_r_apdu(&r_apdu);

    //get challenge
    send_command_usim(&usim,0x00,0x84,0x00,0x00,INVALID_LC,NULL,
                      0x04,&r_apdu);
    printf("Challenge size is %u.\n",r_apdu.r_apdu_size);
    for(uint8_t i=0;i<r_apdu.r_apdu_size;++i)
    {
        printf("0X%02X-",r_apdu.r_apdu[i]);
    }
    printf("\n");
    release_r_apdu(&r_apdu);


    release_usim(&usim);

    printf("Hello world.\n");
    return 0;
}
