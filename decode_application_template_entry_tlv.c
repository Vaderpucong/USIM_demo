//
// Created by pu on 2/16/21.
//

#include "decode_application_template_entry_tlv.h"


app_temp_entry_t* decode_application_template_entry_tlv(uint8_t* raw_data)
{
    assert(raw_data != NULL);

    uint8_t decode_index=0;

    if(0x61 == raw_data[decode_index++])
    {
        uint8_t application_template_size = raw_data[decode_index++];

        if(0x4F == raw_data[decode_index++])
        {
            app_temp_entry_t* app_temp_entry_p = (app_temp_entry_t*)malloc(sizeof(app_temp_entry_t));
            assert(app_temp_entry_p != NULL);
            memset(app_temp_entry_p , 0 , sizeof(app_temp_entry_t));

            app_temp_entry_p->aid_size = raw_data[decode_index++];
            memcpy(app_temp_entry_p->aid , &raw_data[decode_index] , app_temp_entry_p->aid_size);
            decode_index+=app_temp_entry_p->aid_size;

            if(decode_index < application_template_size && 0x50 == raw_data[decode_index++])
            {
                app_temp_entry_p->app_label_size = raw_data[decode_index++];
                app_temp_entry_p->app_label_value=(uint8_t*)malloc(app_temp_entry_p->app_label_size);
                assert(app_temp_entry_p->app_label_value != NULL);
                memcpy(app_temp_entry_p->app_label_value , &raw_data[decode_index++],app_temp_entry_p->app_label_size);
            }
            return app_temp_entry_p;
        }
        else
        {
            printf("decode application template entry failure.\n");
            return NULL;
        }
    }
    else
    {
        printf("decode application template entry failure.\n");
        return NULL;
    }
}

void release_application_template_entry(app_temp_entry_t** app_temp_entry_pp)
{
    assert(app_temp_entry_pp != NULL && *app_temp_entry_pp != NULL);
    if((*app_temp_entry_pp)->app_label_value) free((*app_temp_entry_pp)->app_label_value);
    free(*app_temp_entry_pp);
    *app_temp_entry_pp = NULL;
}