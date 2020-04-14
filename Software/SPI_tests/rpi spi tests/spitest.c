#pi@emonpiisohv:~/pyspitest $ cat spitest.c
#include <stdio.h>
#include <string.h>
#include <wiringPiSPI.h>

int main(void) {
int channel = 0;
int speed = 500000;
char data[12];

short tx[] = {
	0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4, 0xFFF5, 0xFFF6,
};
memcpy(data, tx, sizeof(tx));
wiringPiSPISetup (channel, speed);
wiringPiSPIDataRW (channel, data, sizeof(data));
int i = 0;
for (i = 0; i <sizeof(data); i++) {
	printf("%.2X ", data[i]);
}
}


/*
int main(int argc, char *argv[])
{
  if (argc > 1) printf("Your name is %s\n", argv[1]);
  else printf("Your name is not known\n");
}


#include <stdio.h>

//int wiringPiSPISetup (int 0, int 500000);

//echo wiringPiSPISetup;
printf("Hello world!\n");
//printf("Test");
*/
