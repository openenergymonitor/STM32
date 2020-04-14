// see wiringpi Blink example : http://wiringpi.com/examples/blink/
// $ sudo apt-get install wiringpi
// the compile command run on the pi is: gcc -Wall -o stm_reset_boot stm_reset_boot.c -lwiringPi
// run with $ sudo ./stm_reset_boot

#include <wiringPi.h>
int main (void)
{
  wiringPiSetup () ;
  pinMode (0, OUTPUT) ; // reset
  pinMode (7, INPUT) ; // BOOT0 set to input for safety
  pinMode (27, OUTPUT) ; // GPIO16
  pinMode (28, OUTPUT) ; // GPIO20
  digitalWrite (27, HIGH) ;
  digitalWrite (28, HIGH) ; delay(5) ;
  digitalWrite (0, LOW) ; delay(5) ;
  pinMode (0, INPUT) ; // reset
  return 0 ;
}