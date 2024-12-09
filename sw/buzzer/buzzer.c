#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// offsets for rotary encoder
#define ENCODER_STATE_OFFSET 0x0
#define ENABLE_OFFSET 0x4
// offsets for buzzer
#define VOLUME_OFFSET 0x0
#define PERIOD_OFFSET 0x4

// vars for writing to devices
FILE *file;
size_t ret;
uint32_t val;

// vars for rotary encoder
uint32_t state;
uint32_t buzzer_en;

// vars for buzzer
uint32_t volume;
uint32_t freq;
uint32_t period_b;


void INThandler(int sig)
{
	char c;

	signal(sig, SIG_IGN);
	fprintf(stdout, "\n Are you sure you want to quit? [y/n] \n");
	c = getchar();
	if ( c == 'y' || c == 'Y')
	{

		file = fopen("/dev/buzzer" , "rb+" );

		// Turn off everything
		printf("Setting volume and frequency to zero....\n");
		val = 0x00;
		ret = fseek(file, VOLUME_OFFSET, SEEK_SET);
		ret = fwrite(&val, 4, 1, file);
		fflush(file);

		ret = fseek(file, PERIOD_OFFSET, SEEK_SET);
		ret = fwrite(&val, 4, 1, file);
		fflush(file);

		fclose(file);
		exit(0);
	}
}

int main () {

	// intial check to see if we can access both devices
	file = fopen("/dev/rotary", "rb+");
	if (file == NULL) {
		printf("failed to open rotary file\n");
		exit(1);
	}
	fclose(file);

	file = fopen("/dev/buzzer", "rb+");
	if (file == NULL) {
		printf("failed to open buzzer file\n");
		exit(1);
	}
	fclose(file);

	// Getting frequency from user
	printf("Please enter buzzer frequency: ");
	scanf("%d", &freq); // I know scanf is not the best way to do this, and I know I could implement some ways to make sure this is only integer values, but I don't have the time/will power to implement this. Remeber I am uninspired and lazy...
	if (freq < 16)
	{
		printf("Please enter a frequency greater than 16 Hz\n");
		exit(1);
	}
	// Converting the frequency to the correct 32.26 (n.f) number for the period register
	period_b = 1000/freq; // first change frequency to period in milliseconds
	period_b = period_b << 26; // mulitply by number of fractional bits so we get correct value for register

	signal(SIGINT, INThandler); // allow for exit with ^C
	while(1)
	{
		// the range of the volume register is 0 - 0x80000 (20.19 n.f for duty cycle which is proportional to buzzer volume)
		// this is 0 - 524288 in decimal
		// the range of the rotary encoder state  is 0 - 64 in decimal
		// this means we need a scaling factor of 8192 to convert from rotary state to volume value

		// first check if buzzer is enabled with rotary push button reg
		file = fopen("/dev/rotary","rb+");
		ret = fseek(file, ENABLE_OFFSET, SEEK_SET);
		ret = fread(&buzzer_en, 4, 1, file);
		printf("buzzer enable = 0x%x\n", buzzer_en);
		fclose(file);

		// now if we are enabled we should write to the period register 
		if (buzzer_en == 1)
		{
			file = fopen("/dev/buzzer", "rb+");
			ret = fseek(file, PERIOD_OFFSET, SEEK_SET);
			ret = fwrite(&period_b, 4, 1, file);
			fflush(file);
			fclose(file);

			// now volume from rotary encoder
			// the volume register is just duty cycly of the register
			// full 100% duty cycle would be 0x80000 in hex that is 524288 in decimal
			// the rotary encoder has a range of 0 to 63.
			// lets map that to 0 to 524288 for the volume duty cycle
			// 524288/63 = ~8322. This is our scaling value

			// First lets read the rotary encoder
			file = fopen("/dev/rotary","rb+");
			ret = fseek(file, ENCODER_STATE_OFFSET, SEEK_SET);
			ret = fread(&volume, 4, 1, file);
			printf("volume = %d\n",volume);
			fclose(file);
			// Multiply by scaling value
			volume = volume * 8322;

			// Write to buzzer volume register
			file = fopen("/dev/buzzer","rb+");
			ret = fseek(file, VOLUME_OFFSET, SEEK_SET);
			ret = fwrite(&volume, 4, 1, file);
			fflush(file);
			fclose(file);
		}
	}
	return 0;
}
