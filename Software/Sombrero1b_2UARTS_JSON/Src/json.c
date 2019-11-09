#include "jsmn.h"
#include <stdio.h>


#define JSON_TOKENS 128
jsmntok_t tk[JSON_TOKENS]; /* We expect no more than 128 JSON tokens */
jsmn_parser p;


void json_parser(char * JSON_STRING)
{
    int r;
    int i;
    jsmn_init(&p);
    r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), tk, JSON_TOKENS);

    if (r < 0)
    {
        debug_printf("Failed to parse JSON: %d\r\n", r);
    }
    else
    {
        char code_rxed[100];
        i = 0;                          // i = 0 is the first JSON object.
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