#include "jsmn.h"
#include "json.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include <stdbool.h>

extern char hwVersion[] = "hw05";
extern char fwVersion[] = "json_uart_test";
bool getset = 0; // 0 = get, 1 = set.
extern char log_buffer[LOG_BUFFER_SIZE];
char log_buffer[100];
extern boot_number;
#define true 1
#define false 0

#define JSON_TOKENS 128
jsmntok_t tk[JSON_TOKENS]; /* We expect no more than 128 JSON tokens */
jsmn_parser p;
extern char log_buffer[LOG_BUFFER_SIZE];

bool json_parser(char *JSON_STRING)
{
    int r;
    int i;
    jsmn_init(&p);
    r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), tk, JSON_TOKENS);

    if (r < 0)
    {
        sprintf(log_buffer, "Failed to parse JSON: %d\r\n", r);
        debug_printf(log_buffer);
    }
    else
    {

        char select[100];
        //----------------------------------------------------
        // LEVEL ONE
        //----------------------------------------------------

        //
        i = 0; // i = 0 is the first JSON object index?
        sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        if (!strcmp(select, "G"))
        {
            getset = 0;
        }
        else if (!strcmp(select, "S"))
        {
            getset = 1;
        }

        //----------------------------------------------------
        // LEVEL TWO
        //----------------------------------------------------
        i = 1; // i = 0 is the first JSON object index?
        sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        //--------------------------
        // GET Commands
        //--------------------------
        if (getset == 0)
        {
            if (!strcmp(select, "hello"))
            {
                printf_rpi("hello");
                return 1;
            }
            else if (!strcmp(select, "hw_version"))
            {
                sprintf(log_buffer, "hw_version:%s", hw_version)
                    printf_rpi(log_buffer);
                return 1;
            }
            else if (!strcmp(select, "fw_version"))
            {
                sprintf(log_buffer, "fw_version:%s", fw_version)
                    printf_rpi(log_buffer);
                return 1;
            }
            else if (!strcmp(select, "mode"))
            {
                sprintf(log_buffer, "device_mode:%s", device_mode)
                    printf_rpi(log_buffer);
                return 1;
            }
            else if (!strcmp(select, "boot_reason"))
            {
                sprintf(log_buffer, "boot_reason:%s", reset_cause_get_name(reset_cause));
                printf_rpi(log_buffer);
                return 1;
            }
            else if (!strcmp(select, "number_reboots"))
            {
                sprintf(log_buffer, "number_reboots:%d", boot_number);
                printf_rpi(log_buffer);
                return 1;
            }
            else if (!strcmp(select, "uptime"))
            {
                sprintf(log_buffer, "uptime:%d", HAL_GetTick());
                printf_rpi(log_buffer);
                return 1;
            }
            else if (!strcmp(select, "rtc_time"))
            {
                sprintf(log_buffer, "rtc_time:%d", rtc_time_get());
                printf_rpi(log_buffer);
                return 1;
            }
            else if (!strcmp(select, "system"))
            {
                // implement system list print out.
                //sprintf(log_buffer, "system_list:%d", HAL_GetTick());
                //printf_rpi(log_buffer);
                return 1;
            }
            else if (!strcmp(select, "CT"))
            {
                // implement 3rd stage jsmn tkn, look for number, then 4th stage the get command.

                return 1;
            }
            else if (!strcmp(select, "VT"))
            {
                // implement 3rd stage jsmn tkn, look for number, then 4th stage the get command.
                return 1;
            }
            else if (!strcmp(select, "I/O"))
            {
                // implement 3rd stage jsmn tkn, look for number, then 4th stage the get command.
                return 1;
            }
            return 0;
        }
        //--------------------------
        // SET Commands
        //--------------------------
        else
        {
            if (!strcmp(select, "mode"))
            {
                return 1;
            }
            else if (!strcmp(select, "rtc_time"))
            {
                return 1;
            }
            else if (!strcmp(select, "CT"))
            {
                return 1;
            }
            else if (!strcmp(select, "VT"))
            {
                return 1;
            }
            else if (!strcmp(select, "I/O"))
            {
                return 1;
            }
            else if (!strcmp(select, "MBUS"))
            {
                return 1;
            }
            return 0;
        }
    }
}
