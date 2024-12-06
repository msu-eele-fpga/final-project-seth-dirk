#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#define LED_ARRAY_OFFSET 0x0
#define ROTARY_ENCODER_OFFSET 0x0

int main () {
	FILE *file;
	size_t ret;	
	uint32_t encoder;   //Hold the value from the encoder state register
    uint32_t pattern;   //Hold the pattern to be written to the led array

    // Open the sysfs for the rotary encoder device driver
	file = fopen("/dev/encoder" , "rb+" );
	if (file == NULL) {
		printf("failed to open file\n");
		exit(1);
	}

	// Read the encoder state register
	printf("\n************************************\n*");
	printf("* read the encoder state register value\n");
	printf("************************************\n\n");

	ret = fread(&encoder, 4, 1, file);
	printf("Encoder State = 0x%x\n", encoder);

	// Reset file position to 0
	ret = fseek(file, 0, SEEK_SET);
	printf("fseek ret = %d\n", ret);
	printf("errno =%s\n", strerror(errno));
    fclose(file);

    // Write a pattern to the LED array based off of the encoder state
	printf("\n************************************\n*");
	printf("* write value to the LED array\n");
	printf("************************************\n\n");

    // Open the sysfs for the led_array device driver
    file = fopen("/dev/led_array" , "rb+" );
	if (file == NULL) {
		printf("failed to open file\n");
		exit(1);
	}

    // Case/switch statement to set the LED pattern from the encoder state
    switch(encoder)
    {
        case 0x1:
            pattern = 0x80;
            break;
        case 0x2:
            pattern = 0xC0;
            break;
        case 0x3:
            pattern = 0xE0;
            break;
        case 0x4:
            pattern = 0xF0;
            break;
        case 0x5:
            pattern = 0xF8;
            break;
        case 0x6:
            pattern = 0xFC;
            break;
        case 0x7:
            pattern = 0xFE;
            break;
        case 0x8:
            pattern = 0xFF;
            break;
        default:
            pattern = 0x00;
            break;
    }

	// Write the pattern to the LEDs
	printf("writing pattern to LEDs....\n");
    ret = fseek(file, LED_ARRAY_OFFSET, SEEK_SET);
	ret = fwrite(&pattern, 4, 1, file);
	fflush(file);
	fclose(file);
	return 0;
}