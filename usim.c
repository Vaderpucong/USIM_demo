//
// Created by DELL on 2021/2/15.
//

#include "usim.h"
#include "decode_application_template_entry_tlv.h"

/***************************internal data*******************************/
const char* Command_APDUs_str[]={
        "SELECT FILE",
        "STATUS",
        "READ BINARY",
        "UPDATE BINARY",
        "READ RECORD",
        "UPDATE RECORD",
        "SEARCH RECORD",
        "INCREASE",
        "RETRIEVE DATA",
        "SET DATA",
        "VERIFY PIN",
        "CHANGE PIN",
        "DISABLE PIN",
        "ENABLE PIN",
        "UNBLOCK PIN",
        "DEACTIVATE FILE",
        "ACTIVATE FILE",
        "AUTHENTICATE",
        "AUTHENTICATE",
        "GET CHALLENGE",
        "TERMINAL CAPABILITY",
        "TERMINAL PROFILE",
        "ENVELOPE",
        "FETCH",
        "TERMINAL RESPONSE",
        "MANAGE CHANNEL",
        "MANAGE SECURE CHANNEL",
        "TRANSACT DATA",
        "SUSPEND UICC",
        "GET IDENTITY",
        "EXCHANGE CAPABILITIES",
        "GET RESPONSE",
};
uint8_t Command_APDUs_id[]={0xA4,0xF2,0xB0,0xD6,
                            0xB2,0xDC,0xA2,0x32,
                            0xCB,0xDB,0x20,0x24,
                            0x26,0x28,0x2C,0x04,
                            0x44,0x88,0x89,0x84,
                            0xAA,0x10,0xC2,0x12,
                            0x14,0x70,0x73,0x75,
                            0x76,0x78,0x7A,0xC0};



/***************************internal functions*******************************/
static apdu_t* _new_apdu_command(
        uint8_t cla,
        uint8_t ins,
        uint8_t p1,
        uint8_t p2,
        uint8_t lc,
        uint8_t* data,
        uint8_t le);
static int _send_apdu(usim_t* usim,r_apdu_t* r_apdu_p /* out */);
static void _decode_status_words(uint8_t sw1,uint8_t sw2);
static void _display_one_apdu(apdu_t* apdu_p);
static int _activate_usim_app(usim_t* usim);

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
    _display_one_apdu(_apdu_p);
    if(0 == usim->send_apdu_usim(GTE_APDU_HEAD(_apdu_p),_apdu_p->apdu_size))
        return 0;
    switch(r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-2]) //SW1
    {
        case 0x61: {
            //send GET RESPONSE command
            uint8_t _xx = GTE_APDU_LE(_apdu_p) < r_tpdu.r_tpdu[r_tpdu.r_tpdu_size - 1]?
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
            //for Case4
            if(_apdu_p->has_Lc == _HAS_LC &&
                _apdu_p->has_Le == _HAS_LE)
            {
                //decode status words
                _decode_status_words(r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-2],
                                     r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-1]);
                //_display_one_apdu(_apdu_p);
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
            _decode_status_words(r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-2],
                                 r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-1]);
            //_display_one_apdu(_apdu_p);
            assert(r_apdu_p != NULL);
            memset(r_apdu_p , 0 , sizeof(r_apdu_t));
            r_apdu_p->r_apdu_size = r_tpdu.r_tpdu_size - 2; //substrate SW1|SW2
            r_apdu_p->r_apdu = (uint8_t*)malloc(r_apdu_p->r_apdu_size);
            assert(r_apdu_p->r_apdu != NULL);
            memcpy(r_apdu_p->r_apdu, r_tpdu.r_tpdu , r_apdu_p->r_apdu_size);
            r_apdu_p->sw1 = r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-2];
            r_apdu_p->sw2 = r_tpdu.r_tpdu[r_tpdu.r_tpdu_size-1];
            break;
    }
    return 1;
}

static void _decode_status_words(uint8_t sw1,uint8_t sw2)
{
    switch(sw1)
    {
        //normal processing
        case 0x90:
        case 0x91:
        case 0x92:
            printf("Normal processing:");
            printf("Normal ending of the command.\n");
            break;
        //postponed processing
        case 0x93:
            printf("Postponed processing:");
            printf("SIM Application Toolkit is busy. "
                   "Command cannot be executed at present, further "
                   "normal commands are allowed.\n");
            break;
        //warnings
        case 0x62:
            printf("Warning:");
            switch(sw2)
            {
                case 0x00:
                    printf("No information given, "
                           "state of non-volatile memory unchanged.\n");
                    break;
                case 0x81:
                    printf("Part of returned data may be corrupted.\n");
                    break;
                case 0x82:
                    printf("End of file/record reached before "
                           "reading Le bytes or unsuccessful search.\n");
                    break;
                case 0x83:
                    printf("Selected file invalidated.\n");
                    break;
                case 0x85:
                    printf("Selected file in termination state.\n");
                    break;
                case 0xF1:
                    printf("More data available.\n");
                    break;
                case 0xF2:
                    printf("More data available and proactive command pending.\n");
                    break;
                case 0xF3:
                    printf("Response data available.\n");
                    break;
            }
            break;
        case 0x63:
            printf("Warning:");
            switch(sw2)
            {
                case 0xF1:
                    printf("More data expected.\n");
                    break;
                case 0xF2:
                    printf("More data expected and proactive command pending.\n");
                    break;
                default:
                    if(sw2>>4 == 0xCu)
                        printf("Command successful but after using an internal update retry routine %u times\n"
                               "or Verification failed, %u retries remaining" , sw2&0x0Fu,sw2&0x0Fu);
                    break;
            }
            break;
        //execution errors
        case 0x64:
            printf("Execution errors:");
            printf("No information given, state of non-volatile memory unchanged.\n");
            break;
        case 0x65:
            printf("Execution errors:");
            switch(sw2)
            {
                case 0x00:
                    printf("No information given, state of non-volatile memory changed.\n");
                    break;
                case 0x81:
                    printf("Memory problem.\n");
                    break;
            }
            break;
        //checking errors
        case 0x67:
            printf("Checking errors:");
            switch(sw2)
            {
                case 0x00:
                    printf("Wrong length.\n");
                    break;
                default:
                    printf("The interpretation of this status "
                           "word is command dependent.\n");
                    break;
            }
            break;
        case 0x6B:
            printf("Checking errors:");
            printf("Wrong parameter(s) P1-P2.\n");
            break;
        case 0x6D:
            printf("Checking errors:");
            printf("Instruction code not supported or invalid.\n");
            break;
        case 0x6E:
            printf("Checking errors:");
            printf("Class not supported.\n");
            break;
        case 0x6F:
            printf("Checking errors:");
            switch(sw2)
            {
                case 0x00:
                    printf("Technical problem, no precise diagnosis.\n");
                    break;
                default:
                    printf("The interpretation of this status word is command dependent.\n");
                    break;
            }
            break;
        //functions in CLA not supported
        case 0x68:
            printf("Functions in CLA not supported:");
            switch (sw2)
            {
                case 0x00:
                    printf("No information given.\n");
                    break;
                case 0x81:
                    printf("Logical channel not supported.\n");
                    break;
                case 0x82:
                    printf("Secure messaging not supported.\n");
                    break;
            }
        //command not allowed
        case 0x69:
            printf("Command not allowed:");
            switch (sw2)
            {
                case 0x00:
                    printf("No information given.\n");
                    break;
                case 0x81:
                    printf("Command incompatible with file structure.\n");
                    break;
                case 0x82:
                    printf("Security status not satisfied.\n");
                    break;
                case 0x83:
                    printf("Authentication/PIN method blocked.\n");
                    break;
                case 0x84:
                    printf("Referenced data invalidated.\n");
                    break;
                case 0x85:
                    printf("Conditions of use not satisfied.\n");
                    break;
                case 0x86:
                    printf("Command not allowed (no EF selected).\n");
                    break;
                case 0x89:
                    printf("Command not allowed - secure channel - security not satisfied.\n");
                    break;
            }
            break;
        //wrong parameters
        case 0x6A:
            printf("Wrong parameters:");
            switch(sw2)
            {
                case 0x80:
                    printf("Incorrect parameters in the data field.\n");
                    break;
                case 0x81:
                    printf("Function not supported.\n");
                    break;
                case 0x82:
                    printf("File not found.\n");
                    break;
                case 0x83:
                    printf("Record not found.\n");
                    break;
                case 0x84:
                    printf("Not enough memory space.\n");
                    break;
                case 0x86:
                    printf("Incorrect parameters P1 to P2.\n");
                    break;
                case 0x87:
                    printf("Lc inconsistent with P1 to P2.\n");
                    break;
                case 0x88:
                    printf("Referenced data not found.\n");
                    break;
            }
            break;
        //application errors
        case 0x98:
            printf("Application errors:");
            switch (sw2)
            {
                case 0x50:
                    printf("INCREASE cannot be performed, max value reached.\n");
                    break;
                case 0x62:
                    printf("Authentication error, incorrect MAC.\n");
                    break;
                case 0x63:
                    printf("Security session or association expired.\n");
                    break;
                case 0x64:
                    printf("Minimum UICC suspension time is too long or "
                           "Authentication error, security context not supported.\n");
                    break;
                case 0x65:
                    printf("Key freshness failure.\n");
                    break;
                case 0x66:
                    printf("Authentication error, no memory space available.\n");
                    break;
                case 0x67:
                    printf("Authentication error, no memory space available in EFMUK.\n");
                    break;
            }
    }
}

static void _display_one_apdu(apdu_t* apdu_p)
{
    printf("|0x%02X-0x%02X-0x%02X-0x%02X|",apdu_p->CLA,apdu_p->INC,apdu_p->P1,apdu_p->P2);
    uint8_t data_size=0;
    if(apdu_p->has_Lc){
        printf("0x%02X|0x",apdu_p->Lc_data_Le[0]);
        data_size = apdu_p->apdu_size - 4 -apdu_p->has_Lc - apdu_p->has_Le;
        for(uint8_t i=0;i<data_size;++i)
            printf("%02X",apdu_p->Lc_data_Le[1+i]);
        printf("|");
    }

    if(apdu_p->has_Le)
        printf("0x%02X|",apdu_p->Lc_data_Le[apdu_p->has_Lc+data_size]);
    printf("\n");
}

static int _activate_usim_app(usim_t* usim)
{
    r_apdu_t r_apdu;
    //SELECT EFdir
    uint8_t EFDIR[2]={0x2F,0x00};
    send_command_usim(usim,0x00,0xA4,0x00,0x04,0x02,EFDIR,
                      INVALID_LE,&r_apdu);
    release_r_apdu(&r_apdu);

    //SEARCH RECORD
    uint8_t usim_identity[]={0xA0,0x00,0x00,0x00,0x87,0x10,0x02};
    send_command_usim(usim,0x00,0xA2,0x01,0x04,sizeof(usim_identity),usim_identity,
                      0x01,&r_apdu);
    if(r_apdu.r_apdu_size == 0){
        printf("No find USIM App,activate USIM App failure.\n");
        return 0;
    }
    uint8_t usim_record_id = r_apdu.r_apdu[0];
    release_r_apdu(&r_apdu);

    //READ RECORD
    send_command_usim(usim,0x00,0xB2,usim_record_id,0x04,INVALID_LC,NULL,
                      0x00,&r_apdu);
    //DECODE APP TEMPLATE ENTRY
    app_temp_entry_t* app_temp_entry_p = decode_application_template_entry_tlv(r_apdu.r_apdu);
    if(app_temp_entry_p == NULL)
    {
        printf("%s-%d Error: decode AID TLV failure.\n",__FILE__,__LINE__);
        release_r_apdu(&r_apdu);
        return 0;
    }
    else
    {
        release_r_apdu(&r_apdu);
        if(0 == send_command_usim(usim,0x00,0xA4,0x04,0x04,
                          app_temp_entry_p->aid_size,app_temp_entry_p->aid,
                          INVALID_LE,&r_apdu))
        {
            printf("%s-%d Error: send command to usim failure.\n",__FILE__,__LINE__);
            release_application_template_entry(&app_temp_entry_p);
            release_r_apdu(&r_apdu);
            return 0;
        }
        if(r_apdu.sw1 == 0x90)
            printf("Activate USIM App successfully.\n");
        else
            printf("Activate USIM App unsuccessfully.\n");
        release_application_template_entry(&app_temp_entry_p);
        release_r_apdu(&r_apdu);
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
    _activate_usim_app(usim);
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
    printf("======================================.\n");

    return 1;
}

void release_r_apdu(r_apdu_t* r_apdu_p)
{
    assert(r_apdu_p != NULL);
    if(r_apdu_p->r_apdu) free(r_apdu_p->r_apdu);
    r_apdu_p->r_apdu = NULL;
    r_apdu_p->r_apdu_size = 0;
}