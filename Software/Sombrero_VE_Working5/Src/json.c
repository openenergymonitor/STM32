#include "jsmn.h"
#include "usart.h"
//#include "rtc.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
/*
static const char *JSON_STRING =
    "{\"user\": \"johndoe\", \"admin\": false, \"uid\": 1000,\n  "
    "\"groups\": [\"users\", \"wheel\", \"audio\", \"video\"]}";
*/

//static const char *JSON_STRING = "{S:CT:2:state:1}"; // testing loveliness
//char *JSON_STRING = "{S:CT:2:state:1}"; // testing loveliness

//extern char log_buffer[];

typedef struct VTproperties_
{
  float vcal;
  bool state;
  int type;
  int assignedCT; // CT1, CT2 etc.
  //float instant_voltage;
  //int raw[];
  float voltageRMS;
  float max;
  float min;
} VTproperties_t;

typedef struct CTproperties_
{
  float ical;
  bool state;
  int type;
  int burden;
  int ratio;
  int assignedVT; // VT1, VT2 etc.
  //float instant_voltage;
  //int raw[];
  float currentRMS;
  float ApparentPower;
  float RealPower;
  float max;
  float min;
  double watth;
  float pfInsta;
  float pfAVG;
} CTproperties_t;

typedef struct IOproperties_
{
  int pin_physical;
  char pin_register;
  int pin_number;
  bool init;      // pin initialisation; 0 for input, 1 for output.
  int type;       // GPIO | LED | BUTTON
  bool interrupt; // If pin is input, treat as interrupt true/false.
  bool value;
} IOproperties_t;

static unsigned int uint2bcd(unsigned int ival) // https://www.microchip.com/forums/m271601.aspx
 {
 	return ((ival / 10) << 4) | (ival % 10);
 }

#define VTn 3
#define CTn 9
#define IOn 5 // number of IO in hardware.

static VTproperties_t VTproperties[VTn];
static CTproperties_t CTproperties[CTn];
//static IOproperties_t IOproperties[IOn];

void VTCommands(bool getset, int ch, char *command, char *property);
void CTCommands(bool getset, int ch, char *command, char *property);
void IOCommands(bool getset, int ch, char *command, char *property);
void error_handler(void);

/*static int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
  if (tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0)
  {
    return 0;
  }
  return -1;
}
*/

jsmn_parser p;
#define JSON_TOKENS 128
jsmntok_t tk[JSON_TOKENS]; /* We expect no more than 128 JSON tokens */
#define LOG_BUFFER_SIZE 20
char hwVersion[] = "alpha_0";
char fwVersion[] = "sombrero_0.1";

int device_mode = 4;
char mode1[] = "rPi";
char mode2[] = "ESP32";
char mode3[] = "standalone";
char mode4[] = "testing";

bool getset; // 0 = get, 1 = set.

int boot_number;

char json_response[40];
void json_parser(char *string)
{
  char *JSON_STRING = string;
  int r;
  int i;
  // // debug_printf("JSON parser test.\r\n");

  jsmn_init(&p);
  r = jsmn_parse(&p, JSON_STRING, strlen(JSON_STRING), tk, JSON_TOKENS);

  if (r < 0)
  {
    sprintf(log_buffer, "ERROR:JSON_PARSE %d\r\n", r);
    debug_printf(log_buffer);
  }

  char select[20];
  //----------------------------------------------------
  // LEVEL ONE
  //----------------------------------------------------

  //
  i = 1; // i = 1 for the first JSON object.
  sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
  if (!strcmp(select, "G"))
  {
    //debug_printf("testing-get");
    getset = 0;
    
  }
  else if (!strcmp(select, "S"))
  {
    //debug_printf("testing-set");
    getset = 1;
    
  }

  //----------------------------------------------------
  // LEVEL TWO
  //----------------------------------------------------
  i = 2;
  sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
  //--------------------------
  // GET Commands
  //--------------------------
  if (getset == 0)
  {
    if (!strcmp(select, "hello_STM32!"))
    {
      sprintf(json_response, "hello_rPi!");
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
      if (device_mode == 0)
      {
        sprintf(json_response, "G:device_mode:uninitialised");
      }
      else if (device_mode == 1)
      {
        sprintf(json_response, "G:device_mode:%s", mode1);
        // // debug_printf(log_buffer);
      }
      else if (device_mode == 2)
      {
        sprintf(json_response, "G:device_mode:%s", mode2);
        // // debug_printf(log_buffer);
      }
      else if (device_mode == 3)
      {
        sprintf(json_response, "G:device_mode:%s", mode3);
        // // debug_printf(log_buffer);
      }
      else if (device_mode == 4)
      {
        sprintf(json_response, "G:device_mode:%s", mode4);
        // // debug_printf(log_buffer);
      }
      else
        error_handler();
    }
    else if (!strcmp(select, "boot_reason"))
    {
      //sprintf(log_buffer, "boot_reason:%s", reset_cause_get_name(reset_cause));
      sprintf(json_response, "G:boot_reason:test\r\n");
      // // debug_printf(log_buffer);
    }
    else if (!strcmp(select, "number_reboots"))
    {
      sprintf(json_response, "G:number_reboots:%d", boot_number);
      // // debug_printf(log_buffer);
    }
    else if (!strcmp(select, "uptime"))
    {
      //      sprintf(log_buffer, "G:uptime:%d", HAL_GetTick());
      // debug_printf(log_buffer);
      // debug_printf("uptime_test\r\n");
    }
    else if (!strcmp(select, "RTC"))
    {
      

      sprintf(json_response, "G:RTC:%d", RTC_CalendarShowUnix());
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
    else if (!strcmp(select, "VT"))
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
          error_handler();
        }
      }
    }
    else if (!strcmp(select, "CT"))
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
        error_handler();
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
      error_handler();
    }
    //return 0;
  }
  //--------------------------
  // SET Commands
  //--------------------------
  else
  {
    if (!strcmp(select, "device_mode"))
    {
      i = 3;
      sprintf(select, "%.*s", tk[i].end - tk[i].start, JSON_STRING + tk[i].start);
      if (!strcmp(select, mode1))
      {
        device_mode = 1;
        sprintf(json_response, "S:device_mode:%s", mode1);
        // debug_printf(json_response);
      }
      else if (!strcmp(select, mode2))
      {
        device_mode = 2;
        sprintf(json_response, "S:device_mode:%s", mode2);
        // debug_printf(json_response);
      }
      else if (!strcmp(select, mode3))
      {
        device_mode = 3;
        sprintf(json_response, "S:device_mode:%s", mode3);
        // debug_printf(json_response);
      }
      else if (!strcmp(select, mode4))
      {
        device_mode = 4;
        sprintf(json_response, "S:device_mode:%s", mode4);
        // debug_printf(json_response);
      }
      else
      {
        // debug_printf("mode_error\r\n");
        error_handler();
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
        error_handler();
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
          error_handler();
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
      error_handler();
    }
  }

  // debug_printf("\r\n"); // ENDENDENDENDENDENDENDENDENDENDENDEND of the MAIN() //
}

/* reference ::::
typedef struct VTproperties_
{
  bool state;
  int type;
  float vcal;
  int assignedCT; // CT1, CT2 etc.
  float voltageRMS;
  float max;
  float min;
} VTproperties_t;
*/

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
  // debug_printf("CMD_ERROR\r\n");
}