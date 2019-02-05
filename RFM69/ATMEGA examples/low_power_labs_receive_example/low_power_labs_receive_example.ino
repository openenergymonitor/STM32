#include <RFM69.h>         //get it here: https://www.github.com/lowpowerlab/rfm69
#include <SPI.h>           //included with Arduino IDE install (www.arduino.cc)
RFM69 radio;

typedef struct {
  unsigned long nodeId; //store this nodeId
  unsigned long uptime; //uptime in ms
  //float         temp;   //temperature maybe?
} Payload;
Payload theData;

void setup() {
  Serial.begin(115200);
  delay(10);
  radio.initialize(RF69_433MHZ,1,210);
  radio.promiscuous(false);
  //radio.readAllRegs(); 
}

byte ackCount=0;
void loop() {
 
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    Serial.print(" [RX_RSSI:");Serial.print(radio.readRSSI());Serial.print("]");

    if (radio.DATALEN != sizeof(Payload))
      Serial.print("Invalid payload received, not matching Payload struct!");
    else
    {
      theData = *(Payload*)radio.DATA; //assume radio.DATA actually contains our struct and not something else
      Serial.print(" nodeId=");
      Serial.print(theData.nodeId);
      Serial.print(" uptime=");
      Serial.print(theData.uptime);
      //Serial.print(" temp=");
      //Serial.print(theData.temp);
    }
    Serial.println();
  }
}
/*
-- 
@openenergymon, @TrystanLea
openenergymonitor.org
megni.co.uk
07796843333
*/
