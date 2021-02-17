//
// Created by pu on 2/16/21.
//

#ifndef USIM_DEMO_DECODE_APPLICATION_TEMPLATE_ENTRY_TLV_H
#define USIM_DEMO_DECODE_APPLICATION_TEMPLATE_ENTRY_TLV_H

#include "commondef.h"

typedef struct app_temp_entry_s{
    uint8_t aid_size;
    uint8_t aid[16];
    uint8_t app_label_size;
    uint8_t* app_label_value;
}app_temp_entry_t;

app_temp_entry_t* decode_application_template_entry_tlv(uint8_t* raw_data);
void release_application_template_entry(app_temp_entry_t** app_temp_entry_pp);

#endif //USIM_DEMO_DECODE_APPLICATION_TEMPLATE_ENTRY_TLV_H
