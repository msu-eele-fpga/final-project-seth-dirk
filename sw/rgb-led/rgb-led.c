#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

// offsets for rgb led
#define PERIOD_OFFSET 0x0
#define RED_DUTY_CYCLE_OFFSET 0x4
#define GREEN_DUTY_CYCLE_OFFSET 0x08
#define BLUE_DUTY_CYCLE_OFFSET 0x0C
// offsets for adc
#define CH_0_OFFSET 0x0
#define CH_1_OFFSET 0x4
#define CH_2_OFFSET 0x8

FILE *file;
size_t ret;	
uint32_t val;


void INThandler(int sig)
{
	char c;

	signal(sig, SIG_IGN);
	fprintf(stdout, "\n Are you sure you want to quit? [y/n] \n");
	c = getchar();
	if ( c == 'y' || c == 'Y')
	{

		file = fopen("/dev/rgb_led" , "rb+" );

		// Turn off everything
		printf("All duty cycles to zero....\n");
		val = 0x00;
		ret = fseek(file, RED_DUTY_CYCLE_OFFSET, SEEK_SET);
		ret = fwrite(&val, 4, 1, file);
		fflush(file);

		ret = fseek(file, GREEN_DUTY_CYCLE_OFFSET, SEEK_SET);
		ret = fwrite(&val, 4, 1, file);
		fflush(file);

		ret = fseek(file, BLUE_DUTY_CYCLE_OFFSET, SEEK_SET);
		ret = fwrite(&val, 4, 1, file);
		fflush(file);

		fclose(file);
		exit(0);
	}
}

int main () {

	// intial check to see if we can access both devices
	file = fopen("/dev/adc", "rb+");
	if (file == NULL) {
		printf("failed to open adc file\n");
		exit(1);
	}
	fclose(file);
	file = fopen("/dev/rgb_led", "rb+");
	if (file == NULL) {
		printf("failed to open rgb file\n");
		exit(1);
	}
	fclose(file);

	// Test reading the registers sequentially
	printf("\n************************************\n*");
	printf("* read initial register values\n");
	printf("************************************\n\n");

	// first read rgb led
	file = fopen("/dev/rgb_led", "rb+");

	ret = fread(&val, 4, 1, file);
	printf("period = 0x%x\n", val);

	ret = fread(&val, 4, 1, file);
	printf("red duty cycle = 0x%x\n", val);

	ret = fread(&val, 4, 1, file);
	printf("green duty cycle = 0x%x\n", val);

	ret = fread(&val, 4, 1, file);
	printf("blue duty cycle = 0x%x\n", val);

	// Reset file position to 0
	ret = fseek(file, 0, SEEK_SET);
	printf("fseek ret = %d\n", ret);
	printf("errno =%s\n", strerror(errno));

	// close the rgb led
	fclose(file);

	file = fopen("/dev/adc", "rb+");
	printf("File opened");
	// read adc now
	ret = fread(&val, 4, 1, file);
	printf("adc channel 0 = 0x%x\n", val);
	ret = fread(&val, 4, 1, file);
	printf("adc channel 1 = 0x%x\n", val);
	ret = fread(&val, 4, 1, file);
	printf("adc channel 1 = 0x%x\n", val);

	// Reset file position to 0
	ret = fseek(file, 0, SEEK_SET);
	printf("fseek ret = %d\n", ret);
	printf("errno =%s\n", strerror(errno));
	// close file when done
	fclose(file);

	signal(SIGINT, INThandler); // allow for exit with ^C
	while(1)
	{
		sleep(1);
	}
	return 0;
}
