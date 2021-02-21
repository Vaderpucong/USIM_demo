/*
 * Description:
 *  Write this file to demonstrate Data Link Layer protocol
 *  In our lab,wo not use pcsclite library. we use FPGA to
 *  finish this work.
 *
 * Author Vaderpucong
 */

#define COMMAND_SIZE_ADDR   0x00
#define COMMAND_DATA_ADDR   0x01
#define ENABLE_USIM_ADDR    0x02

#define READ_DATA_SZIE_ADDR 0x03
#define READ_DATA_ADDR      0x04

#include "usim_if_noPCSC.h"

void fpga_write(uint8_t* data , uint32_t len)
{
    //Demo
    //1.write data size
    //2.write data
}

int fpga_read(uint8_t* data , uint32_t* len)
{
    //Demo
    //1.read data size
    //2.read data
}

int init_scard()
{
    //Demo
    return 1;
}

void release_scard()
{
    //Demo
}

int send_apdu_usim(uint8_t* apdu_p , uint8_t apdu_size)
{
    uint8_t enable_usim = 0;//random value
    uint32_t  data_size = 0;
    uint8_t *data=NULL;

    uint8_t response_data[260]={0};
    uint32_t res_size=0;

    uint8_t hasLe=0;
    uint8_t hasLc=0;

    //send command header size to usim
    fpga_write(apdu_p , 5/*in Byte*/); //CLS INS P1 P2 P3

    //get response(data size and data)
    fpga_read(response_data, &res_size);

re_decode:

    if(res_size > 3) //INS [Le Bytes of data] SW1 SW2
    {
        hasLe = 0;
        if(apdu_size > 4)
        {
            if(hasLc == 1)
            {
                if((apdu_size-5-apdu_p[4]) == 1) hasLe = 1;
                else hasLe = 0;
            }
            else
                hasLe = 1;
        }
        if(hasLe && (apdu_p[apdu_size-1] != 0)
        && (res_size-2-1)>apdu_p[apdu_size-1]) //if apdu_size > Le , return Le bytes of data
            r_tpdu_size = apdu_p[apdu_size-1];
        else
            r_tpdu_size = res_size-2-1;
        memset(r_tpdu , 0 , sizeof(r_tpdu));
        memcpy(r_tpdu,response_data+1,r_tpdu_size);
    }
    else if(res_size == 1 && response_data[0]==apdu_p[1]) // return INS
    {
        hasLc = 1;

        //send data part of APDU
        fpga_write(apdu_p+5 , apdu_p[4]/*in Byte*/);

        //get response(data size and data)
        fpga_read(response_data, &res_size);

        goto re_decode;
    }
    else if(res_size == 2) //only SW1 SW2
    {
        r_tpdu_size = res_size;
        memset(r_tpdu , 0 , sizeof(r_tpdu));
        memcpy(r_tpdu,response_data,r_tpdu_size);
    }
    else
    {
        printf("Error:undefined action.\n");
        return 0;
    }


    return 1;
}