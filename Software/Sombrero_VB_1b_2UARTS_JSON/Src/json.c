#include "jsmn.h"
#include <stdio.h>
#include <string.h>
#include "usart.h"
#define JSON_TOKENS 128
jsmntok_t tk[JSON_TOKENS]; /* We expect no more than 128 JSON tokens */
jsmn_parser p;
extern char log_buffer[LOG_BUFFER_SIZE];

void json_parser(char *JSON_STRING)
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
        char code_rxed[100];
        i = 0;                          // i = 0 is the first JSON object index.
        char my_char_pattern[] = "CT1"; // example code pattern to look out for
        sprintf(code_rxed, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        if (strcmp(code_rxed, my_char_pattern) == 0)
        {
            debug_printf("JSON pattern MATCH!\r\n");
        }
        else
        {
            debug_printf("No matching JSON pattern.\r\n");
        }
    }
}