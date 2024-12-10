# Potentiometer-RGB LED user-space software info

## Over view
This code interacts with two devices: an ADC and an RGB LED, by reading and writing to specific registers via file operations. The program continuously reads values from three ADC channels (representing red, green, and blue duty cycles) and writes scaled versions of these values to the RGB LED registers, controlling the LED colors based on ADC input. It also handles shutdown on a keyboard interrupt (Ctrl+C), ensuring all LED duty cycles are set to zero before exiting.

## Building
This file needs to be compiled with `arm-linux-gnueabihf-gcc` to create an executable

## Usage
Run `sudo insmod rgb-led.ko` to load the led driver on the FPGA. 
Run `sudo insmod adc.ko` to load the adc driver on the FPGA.

run the program with `sudo ./rgb-led`

