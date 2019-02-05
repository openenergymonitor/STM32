#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>           //included with Arduino IDE install (www.arduino.cc)   

int TRANSMITPERIOD = 2000; //transmit a packet to gateway so often (in ms)
byte sendSize=0;
boolean requestACK = false;

typedef struct {
  int nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  //float         temp;   //temperature maybe?
} Payload;
Payload theData;

RFM69 radio;

void setup() {
  Serial.begin(115200);
  radio.initialize(RF69_433MHZ,20,210); 
  // radio.readAllRegs(); 
}

long lastPeriod = -1;
void loop() {  
  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    //fill in the struct with new values
    theData.nodeId = 20;
    theData.uptime = millis();
    //theData.temp = 91.23; //it's hot!
    
    Serial.print("Sending struct (");
    Serial.print(sizeof(theData));
    Serial.print(" bytes) ");
    radio.send(1, (const void*)(&theData), sizeof(theData),false);
    Serial.println();
    lastPeriod=currPeriod;
  }
}
