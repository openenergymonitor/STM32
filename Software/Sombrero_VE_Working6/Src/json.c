#include "jsmn.h"
#include "json.h"
#include "usart.h"
#include "adc.h"
#include "rtc.h"
#include "reset.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

const char crm_version[] = "v1"; // version tracking for cmd/response system.
//static const char *JSON_STRING = "{S:CT:2:state:1}"; // testing 
//static const char *JSON_STRING = "{G:boot_reason}"; // testing 

#define VTn 3
#define IOn 5 // number of IO in hardware?
static VTproperties_t VTproperties[VTn];
static CTproperties_t CTproperties[CTn];
//static IOproperties_t IOproperties[IOn];

// JSMN Parser Variables
jsmn_parser p;
#define JSON_TOKENS 128
jsmntok_t tk[JSON_TOKENS]; /* We expect no more than 128 JSON tokens */
char json_response[40];

//------------------------------------
// COMMAND/RESPONSE MODEL
//------------------------------------
/* The json_parser() function takes a char array typically received over UART,
* parses it for json-like elements, and finds a suitable response.
*
* Sequentially:::
*
* 0. Send command string to stm32, for example "{G:hello_STM32}" (excluding quotations).
*
* 1. A pointer to a char array is passed to json_parser().
*
* 2. JSMN api detects the JSON elements, and assignes a token to the JSON element.
*   - this means the first element is token 1, the second token 2, and so on.
*   - these token numbers represent 'levels' in the hierarchy of the command/response scheme.
*  
* 3. Level by level, the JSON element is compared for a suitable next step, resulting in one of two things:
*   3a, finding a known command, and therefore a known response.
*   3b, not finding a knows response, in which case a standard error is returned.
*
* 4. The resulting response string is loaded into the json_response[] char array.
*
* 5. The sender receives a response string prefixed by "{STM32:" (excl. quotations), 
*    for example "{STM32:hello_rPi}".
*
//-----------------------------
// ALL COMMANDS LISTED BELOW
//-----------------------------
// SUMMARY:
***LEVEL ONE (token index = 1)***
G = GET (retrieve a value from the stm32).
S = SET (set a value in the stm32).

***LEVEL TWO (token index = 2)***
// specifics
***LEVEL THREE (token index = 3)***
// specifics
***LEVEL FOUR (token index = 4)***
// specifics


      // COMMAND LIST BEGIN // 
  ALL COMMANDS BRACKETTED WITH CURLY BRACES (e.g {G:hello_STM32} )
Command                       Response Example(s)
G:hello_STM32                 :hello_rPi
G:hw_version                  :v0.8
G:fw_version                  :v1.3
G:device_mode                 :standalone 
-                             :rPi
-                             :ESP32
-                             :dev
G:boot_reason                 : [reseponses listed reset.h in reset_cause_get_name()]
G:boots                       : [ToDo]
G:uptime                      :1234 [millis since start]


reset_cause_t reset_cause = reset_cause_get();
printf("The system reset cause is \"%s\"\n", reset_cause_get_name(reset_cause));



S:device_mode:0               :standalone
S:device_mode:1               :rPi
S:device_mode:2               :ESP32
S:device_mode:3               :dev

S:huntPF:1-9                  :PF hunting (Method2) for CT channels 1 to 9.

*/

// ----------------------
// BGIN json_parser()
// ----------------------


void json_parser(char *string)
{
  static const char mode_0_string[] = "standalone";
  static const char mode_1_string[] = "rPi";
  static const char mode_2_string[] = "ESP32";
  static const char mode_3_string[] = "dev";

  
  char *JSON_STRING = string;
  int r;
  int i;

  jsmn_init(&p);
  r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), tk, JSON_TOKENS);

  if (r < 0)
  {
    while (!usart_tx_ready) {__NOP();} // force wait while usart Tx finishes.
    sprintf(log_buffer, "ERROR:json parse error.%d\r\n", r);
    debug_printf(log_buffer);
  }

  char select[20];

  //----------------------------------------------------
  // LEVEL ONE
  //----------------------------------------------------
  i = 1; // i = 1 for the first JSON object.
  sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
  
  //----------------------------------------------------
  //----------------------------------------------------
  //----------------------------------------------------
  //    LEVEL TWO : GET Commands
  //----------------------------------------------------
  //----------------------------------------------------
  //----------------------------------------------------
  if (!strcmp(select, "G"))
  {
    i = 2;
    sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
    if (!strcmp(select, "hello_STM32"))
    {
      sprintf(json_response, "hello_rPi");
      //// debug_printf("hello_rPi!");
    }
    else if (!strcmp(select, "hw_version"))
    {
      sprintf(json_response, "G:hw_version:%s", hwVersion);
      //// debug_printf(log_buffer);
    }
    else if (!strcmp(select, "fw_version"))
    {
      sprintf(json_response, "G:fw_version:%s", fwVersion);
      //// debug_printf(log_buffer);
    }
    else if (!strcmp(select, "device_mode"))
    {
     if (_mode == 0)
      {
        sprintf(json_response, "G:device_mode:%s", mode_0_string);
        // // debug_printf(log_buffer);
      }
      else if (_mode == 1)
      {
        sprintf(json_response, "G:device_mode:%s", mode_1_string);
        // // debug_printf(log_buffer);
      }
      else if (_mode == 2)
      {
        sprintf(json_response, "G:device_mode:%s", mode_2_string);
        // // debug_printf(log_buffer);
      }
      else if (_mode == 3)
      {
        sprintf(json_response, "G:device_mode:%s", mode_3_string);
        // // debug_printf(log_buffer);
      }
      else {
        error_handler(); return;
      }
    }
    else if (!strcmp(select, "boot_reason"))
    {
      sprintf(json_response, "boot_reason:%s", reset_cause_get_name(reset_cause_store));
      // debug_printf(json_response);
    }
    else if (!strcmp(select, "boots"))
    {
      sprintf(json_response, "boots:%d", boot_number);
    }
    else if (!strcmp(select, "uptime"))
    {
      sprintf(json_response, "uptime:%ld", HAL_GetTick());
    }
    else if (!strcmp(select, "RTC"))
    {
      sprintf(json_response, "RTC:%lld", RTC_CalendarShowUnix());
      //printf(log_buffer);
      // debug_printf("G:rtc_time:123456789\r\n"); //return in millis?
    }
    else if (!strcmp(select, "system"))
    {
      // implement system list print out.
      //sprintf(log_buffer, "system_list:%d", HAL_GetTick());
      //printf_rpi(log_buffer);
      //return 1;
    }
    else if (!strcmp(select, "VT")) // return the VT config.
    {
      i = 3;
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      //printf("%s\r\n", select);
      if (!strcmp(select, "channel"))
      {
        i = 4;
        sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        int x = atoi(select);
        if (x > 0 && x <= VTn)
        {
          i = 5;
          sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
          VTCommands(getset, x, select, 0);
        }
        else
        {
          // debug_printf("unrecognised channel\r\n");
          error_handler(); return;
        }
      }
    }
    else if (!strcmp(select, "CT")) // return the CT channel config.
    {

      i = 3;
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      int x = atoi(select);
      if (x > 0 && x <= CTn)
      {
        i = 4;
        sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        CTCommands(getset, x, select, 0);
      }
      else
      {
        // debug_printf("unrecognised channel\r\n");
        error_handler(); return;
      }
    }
    else if (!strcmp(select, "I/O")) // GPIO Get request handler.
    {
      i = 3;
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      if (!strcmp(select, "channel"))
      {
        i = 4;
        sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        int x = atoi(select);

        i = 5;
        sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        IOCommands(getset, x, select, 0);
      }
    }
    else
    {
      error_handler(); return;
    }
    //return 0;
  }
  //----------------------------------------------------
  //----------------------------------------------------
  //----------------------------------------------------
  //    LEVEL TWO : SET Commands
  //----------------------------------------------------
  //----------------------------------------------------
  //----------------------------------------------------
  else if (!strcmp(select, "S"))
  {
    i = 2;
    sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
    if (!strcmp(select, "device_mode"))
    {
      i = 3;
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      if (!strcmp(select, mode_1_string))
      {
        _mode = 0;
        sprintf(json_response, "S:device_mode:%s", mode_0_string);
        // debug_printf(json_response);
      }
      else if (!strcmp(select, mode_1_string))
      {
        _mode = 1;
        sprintf(json_response, "S:device_mode:%s", mode_1_string);
        // debug_printf(json_response);
      }
      else if (!strcmp(select, mode_2_string))
      {
        _mode = 2;
        sprintf(json_response, "S:device_mode:%s", mode_2_string);
        // debug_printf(json_response);
      }
      else if (!strcmp(select, mode_3_string))
      {
        _mode = 3;
        sprintf(json_response, "S:device_mode:%s", mode_3_string);
        // debug_printf(json_response);
      }
      else
      {
        // debug_printf("mode_error\r\n");
        error_handler(); return;
      }
    }
    else if (!strcmp(select, "RTC"))
    {
      i = 3;
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      //debug_printf("test§");
      time_t timehere = atoi(select); // convert input string to unix time integer.
      struct tm *ptm = localtime(&timehere); // create a 'tm struct' from the integer, breaking time into day, month, year etc..

      //sprintf(log_buffer, "The time from the unix epoch string is: %02d:%02d:%02d\n", ptm->tm_hour, ptm->tm_min, ptm->tm_sec); debug_printf(log_buffer);

      // stm32 date & time structs, see stm32f3xx_hal_rtc.h
      RTC_DateTypeDef sdatestructure;
      RTC_TimeTypeDef stimestructure;
      sdatestructure.Year = uint2bcd(ptm->tm_year-100);
      sdatestructure.Month = uint2bcd(ptm->tm_mon+1);
      sdatestructure.Date = uint2bcd(ptm->tm_mday);
      sdatestructure.WeekDay = uint2bcd(ptm->tm_wday);
      stimestructure.Hours = uint2bcd(ptm->tm_hour);
      stimestructure.Minutes = uint2bcd(ptm->tm_min);
      stimestructure.Seconds = uint2bcd(ptm->tm_sec);
      stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
      stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
      stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

      RTC_CalendarConfig(sdatestructure, stimestructure); // send the structs to the RTC config function, setting the RTC time, which will hold for as long as the device is powered.

      sprintf(json_response, "S:RTC:%lld", timehere); // load the response to send back, confirming signal received.
      return;
    }
    else if (!strcmp(select, "huntPF"))
    {
      i++; // shift up the JSON token index.
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      int x = atoi(select) - 1;
      if (x >= CTn) {error_handler(); return; }
      hunt_PF[x] = 1; // set as 1 to start huntPF().
      sprintf(json_response, "huntPF:%i", x+1);
    }
    else if (!strcmp(select, "CT"))
    {

      i = 3;
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      //printf("%s\r\n", select);
      int x = atoi(select);
      //printf("%d\r\n", x);
      //return 0;
      if (x > 0 && x <= CTn)
      {
        i = 4;
        char _command[20];
        sprintf(_command, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);

        i = 5;
        char _property[20];
        sprintf(_property, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        //printf("%s\r\n", _property);
        CTCommands(getset, x, _command, _property);
      }
      else
      {
        error_handler(); return;
      }
    }
    else if (!strcmp(select, "VT"))
    {
      i = 3;
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      //printf("%s\r\n", select);
      if (!strcmp(select, "channel"))
      {
        i = 4;
        sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
        //printf("%s\r\n", select);
        int x = atoi(select);
        //printf("%d\r\n", x);
        //return 0;
        if (x > 0 && x <= VTn)
        {
          i = 5;
          char _command[20];
          sprintf(_command, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);

          i = 6;
          char _property[20];
          sprintf(_property, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
          //printf("%s\r\n", _property);
          VTCommands(getset, x, _command, _property);
        }
        else
        {
          error_handler(); return;
        }
      }
    }
    else if (!strcmp(select, "I/O"))
    {
      // return 1;
    }
    else if (!strcmp(select, "MBUS"))
    {
      // return 1;
    }
    else
    {
      error_handler(); return;
    }
  }
  else {
    error_handler(); return;
  }
  // debug_printf("\r\n"); // ENDENDENDENDENDENDENDENDENDENDENDEND of the MAIN() //
} 
// ----------------------
// END json_parser()
// ----------------------





// ----------------------
// CMD/RESPONSE Helpers
// ----------------------

void VTCommands(bool getset, int ch, char *command, char *property)
{
  VTproperties_t *VTproperty = &VTproperties[ch]; // example usage VTproperty->vcal = 100.123;

  if (getset == 0)
  { // GET COMMANDS
    if (!strcmp(command, "state"))
    {
      sprintf(json_response, "G:%s:%d", command, VTproperty->state);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "type"))
    {
      sprintf(json_response, "G:%s:%d", command, VTproperty->type);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "vcal"))
    {
      sprintf(json_response, "G:%s:%.3f", command, VTproperty->vcal);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "voltageRMS"))
    {
      sprintf(json_response, "G:%s:%.3f", command, VTproperty->voltageRMS);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "max"))
    {
      sprintf(json_response, "G:%s:%.3f", command, VTproperty->max);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "min"))
    {
      sprintf(json_response, "G:%s:%.3f", command, VTproperty->min);
      // debug_printf(json_response);
    }
  }

  if (getset == 1)
  { // SET COMMANDS
    if (!strcmp(command, "state"))
    {
      int x = atoi(property);
      VTproperty->state = x;
      sprintf(json_response, "S:%s:%d", command, VTproperty->state);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "type"))
    {
      int x = atoi(property);
      VTproperty->type = x;
      sprintf(json_response, "S:%s:%d", command, VTproperty->type);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "vcal"))
    {
      float f = strtof(property, NULL);
      VTproperty->vcal = f;
      sprintf(json_response, "S:%s:%.3f", command, VTproperty->vcal);
      // debug_printf(json_response);
    }
  }
}

/* reference 
typedef struct CTproperties_
{
  float ical;
  bool state;
  int type;
  int burden;
  int ratio;
  int assignedVT; // VT1, VT2 etc.
    double watth;
  float pfAVG;
  float currentRMS;
  float ApparentPower;
  float RealPower;
  float max;
  float min;

} CTproperties_t;
*/

void CTCommands(bool getset, int ch, char *command, char *property)
{

  CTproperties_t *CTproperty = &CTproperties[ch]; // example struct usage CTproperty->ical = 100.123;

  if (getset == 0)
  { // GET COMMANDS
    if (!strcmp(command, "state"))
    {
      sprintf(json_response, "G:%s:%d", command, CTproperty->state);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "type"))
    {
      sprintf(json_response, "G:%s:%d", command, CTproperty->type);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "ical"))
    {
      sprintf(json_response, "G:%s:%.3f", command, CTproperty->ical);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "currentRMS"))
    {
      sprintf(json_response, "G:%s:%.3f", command, CTproperty->currentRMS);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "max"))
    {
      sprintf(json_response, "G:%s:%.3f", command, CTproperty->max);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "min"))
    {
      sprintf(json_response, "G:%s:%.3f", command, CTproperty->min);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "burden"))
    {
      sprintf(json_response, "G:%s:%d", command, CTproperty->burden);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "ratio"))
    {
      sprintf(json_response, "G:%s:%d", command, CTproperty->ratio);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "assignedVT"))
    {
      sprintf(json_response, "G:%s:%d", command, CTproperty->assignedVT);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "watth"))
    {
      sprintf(json_response, "G:%s:%.2f", command, CTproperty->watth);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "pfAVG"))
    {
      sprintf(json_response, "G:%s:%.2f", command, CTproperty->pfAVG);
      // debug_printf(json_response);
    }
  }

  if (getset == 1)
  { // SET COMMANDS
    if (!strcmp(command, "state"))
    {

      int x = atoi(property);
      CTproperty->state = x;
      sprintf(json_response, "S:%s:%d", command, CTproperty->state);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "type"))
    {
      int x = atoi(property);
      CTproperty->type = x;
      sprintf(json_response, "S:%s:%d", command, CTproperty->type);
      // debug_printf(json_response);
    }
    if (!strcmp(command, "ical"))
    {
      float f = strtof(property, NULL);
      CTproperty->ical = f;
      sprintf(json_response, "S:%s:%.3f", command, CTproperty->ical);
      // debug_printf(json_response);
    }
  }
}

void IOCommands(bool getset, int ch, char *command, char *property)
{

//  IOproperties_t *IOproperty = &IOproperties[ch];

  if (getset == 0)
  { // GET COMMANDS
  }

  if (getset == 1)
  { // SET COMMANDS
  }
}

void error_handler(void)
{
  sprintf(json_response, "CMD_ERROR");
}