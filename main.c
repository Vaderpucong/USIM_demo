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

    uint8_t EFDIR[2]={0x2F,0x00};
    //Case 3
    send_command_usim(&usim,0x00,0xA4,0x00,0x04,0x02,EFDIR,
                      INVALID_LE,&r_apdu);
    release_r_apdu(&r_apdu);

    //SEARCH RECORD
    uint8_t substring[9]={0xA0,0x00,0x00,0x00,
                     0x87,0x10,0x02,0xFF,0x86};
    //Case 4
    send_command_usim(&usim,0x00,0xA2,0x01,0x04,sizeof(substring),substring,
                      0x10,&r_apdu);
    printf("SEARCH RECORD size is %u.\n",r_apdu.r_apdu_size);
    for(uint8_t i=0;i<r_apdu.r_apdu_size;++i)
    {
        printf("0X%02X-",r_apdu.r_apdu[i]);
    }
    printf("\n");
    release_r_apdu(&r_apdu);

    //select AID 0x00a4040410A0000000871002FF86FFFF89FFFFFFFF
    uint8_t aid[16]={0xA0,0x00,0x00,0x00,
                     0x87,0x10,0x02,0xFF,
                     0x86,0xFF,0xFF,0x89,
                     0xFF,0xFF,0xFF,0xFF};
    //Case 4
    send_command_usim(&usim,0x00,0xA4,0x04,0x04,0x10,aid,
                      0xFe,&r_apdu);
    printf("AID size is %u.\n",r_apdu.r_apdu_size);
    for(uint8_t i=0;i<r_apdu.r_apdu_size;++i)
    {
        printf("0X%02X-",r_apdu.r_apdu[i]);
    }
    printf("\n");
    release_r_apdu(&r_apdu);

    /* test case
     * select IMSI FE
     * read IMSI EF
     */
    uint8_t imsi[2]={0x6F,0x07};
    //Case 3
    send_command_usim(&usim,0x00,0xA4,0x00,0x04,0x02,imsi,
                      INVALID_LE,&r_apdu);
    release_r_apdu(&r_apdu);

    //Case 1
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
    //Case 2
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
