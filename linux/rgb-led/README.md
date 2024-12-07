# RGB LED Controller Device Driver Info

Run `sudo insmod rgb_led.ko` to load the driver on the FPGA. 

Navigate to the drivers attribute files by running `cd /sys/devices/platform/ff3bd060`. 

Before writing values to the registers, run `sudo -s` to write commands as the root user. 

To write values of your choosing, run `echo [value] > [filename]`.

## Registers
The device driver has 4 system attribute files that can be communicated with. The 4 attributes are the red duty cycle, green duty cycle, blue duty cycle, and the PWM period. Each attribute file is a 32 bit register mapped to the FPGA, but the period and duty cycles are instantiated with unique sizes in the hardware. This was part of an assignment earlier in the semester, and I just left the data sizes alone for the final project. The duty cycle registers are 20 bits long, with 19 fractional bits. The period register is 32 bits long with 26 fractional bits. Keep this in mind when choosing values to write into each attribute file.