#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define LED_ARRAY_OFFSET 0x0
#define ROTARY_ENCODER_OFFSET 0x0

FILE *file;
size_t ret;	
uint32_t encoder;   //Hold the value from the encoder state register
uint32_t pattern;   //Hold the pattern to be written to the led array

void INThandler(int sig)
{
	char c;

	signal(sig, SIG_IGN);
	fprintf(stdout, "\n Are you sure you want to quit? [y/n] \n");
	c = getchar();
	if ( c == 'y' || c == 'Y')
	{

		file = fopen("/dev/led_array" , "rb+" );

		// Turn off everything
		printf("All LEDs off....\n");
		pattern = 0x00;
		ret = fseek(file, LED_ARRAY_OFFSET, SEEK_SET);
		ret = fwrite(&pattern, 4, 1, file);
		fflush(file);

		fclose(file);
		exit(0);
	}
}

int main () {

	signal(SIGINT, INThandler); // allow for exit with ^C
    while(1)
        {
        // Open the sysfs for the rotary encoder device driver
        file = fopen("/dev/rotary" , "rb+" );
        if (file == NULL) {
            printf("failed to open file\n");
            exit(1);
        }

        ret = fread(&encoder, 4, 1, file);
        printf("Encoder State = 0x%x\n", encoder);

        // If else statement to determine the LED pattern from the encoder state
        if (encoder < 8) 
        {
            pattern = 0x80;
        } 
        else if (encoder >= 8 && encoder < 16){
            pattern - 0xC0;
        }
        else if (encoder >= 16 && encoder < 24){
            pattern - 0xE0;
        }
        else if (encoder >= 24 && encoder < 32){
            pattern - 0xF0;
        }
        else if (encoder >= 32 && encoder < 40){
            pattern - 0xF8;
        }
        else if (encoder >= 40 && encoder < 48){
            pattern - 0xFC;
        }
        else if (encoder >= 48 && encoder < 56){
            pattern - 0xFE;
        }
        else if (encoder >= 56 && encoder < 64){
            pattern - 0xFF;
        }

        // Reset file position to 0
        ret = fseek(file, 0, SEEK_SET);
        printf("fseek ret = %d\n", ret);
        printf("errno =%s\n", strerror(errno));
        fclose(file);

        // Open the sysfs for the led_array device driver
        file = fopen("/dev/led_array" , "rb+" );
        if (file == NULL) {
            printf("failed to open file\n");
            exit(1);
        }

        // Write the pattern to the LEDs
        ret = fseek(file, LED_ARRAY_OFFSET, SEEK_SET);
        ret = fwrite(&pattern, 4, 1, file);
        fflush(file);
        fclose(file);
    }
    return 0;
}
