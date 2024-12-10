# Buzzer Device Driver Info

## Building
A Makefile is included to cross-compile the device driver for ARM linux. Update the `KDIR` variable to point to your linux-socfpga repository directory.

Run `make` in this directory to build to kernel module.

## Device tree node

Use the following device tree node:
```devicetree
buzzer: buzzer@ff210000 {
	compatible = "Howard,buzzer";
	reg = <0xff210000 16>;
    };
```

## Usage

Run `sudo insmod buzzer.ko` to load the driver on the FPGA. 

Navigate to the drivers attribute files by running `cd /sys/devices/platform/ff210000`. 

Before writing values to the registers, run `sudo -s` to write commands as the root user. 

To write values of your choosing, run `echo [value] > [filename]`.

## Register Map

The device driver has 2 system attribute files that can be communicated with. The 2 attributes are buzzer duty cycle and, buzzer period. Each attribute file is a 32 bit register mapped to the FPGA. The period and duty cycles are instantiated in the hardware.

| Offset | Name         | R/W | Purpose                    |
|--------|--------------|-----|----------------------------|
| 0x0    | duty cycle   | R/W | pwm duty cycle             |
| 0x4    | period       | R/W | pwm period                 |
