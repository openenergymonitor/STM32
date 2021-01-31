#include <SPI.h>

void setup() {
	Serial.begin(115200);
	
}

void loop() {
	Serial.println("Hello");
	
	SPI.beginTransaction(SPISettings(10000000, MSBFIRST, SPI_MODE0));
	SPI.transfer(0x88);
	SPI.endTransaction();
	
	delay(500);
}
