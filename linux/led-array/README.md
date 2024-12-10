# DE10NANO LED Array Device Driver

## Building
A Makefile is included to cross-compile the device driver for ARM linux. Update the `KDIR` variable to point to your linux-socfpga repository directory.

Run `make` in this directory to build to kernel module.

## Device tree node

Use the following device tree node:
```devicetree
    array: array@ff220000 {
    compatible = "Howard,array";
    reg = <0xff220000 16>;
    };
```

## Usage

Run `sudo insmod led_array.ko` to load the driver on the FPGA. 

Navigate to the drivers attribute files by running `cd /sys/devices/platform/ff220000`. 

Before writing values to the registers, run `sudo -s` to write commands as the root user. 

To write values of your choosing, run `echo [value] > [filename]`.

## Register Map

The device driver has 1 system attribute files that can be communicated with. The attribute is thed LED pattern you would like to display on the FPGA. The attribute file is a 32 bit register mapped to the FPGA, but only the 8 least significant bits will be displayed on the LEDs.

| Offset | Name         | R/W | Purpose                    |
|--------|--------------|-----|----------------------------|
| 0x0    | LED Array    | R/W | LED Pattern Register       |