#ifndef JSON_H
#define JSON_H

#include "adc.h"
#include "jsmn.h"
#include "main.h"
#include "reset.h"
#include "usart.h"

extern int hunt_PF[CTn];

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


void VTCommands(bool getset, int ch, char *command, char *property);
void CTCommands(bool getset, int ch, char *command, char *property);
void IOCommands(bool getset, int ch, char *command, char *property);
void error_handler(void);

void json_parser(char *string);

extern char hwVersion[];
extern char fwVersion[];

//extern int mode;

const char mode_0[] = "standalone";
const char mode_1[] = "rPi";
const char mode_2[] = "ESP32";
const char mode_3[] = "dev";

bool getset; // 0 = get, 1 = set.

int boot_number;



#endif