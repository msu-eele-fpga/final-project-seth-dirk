# Rotary Encoder Device Driver Info

## Building
A Makefile is included to cross-compile the device driver for ARM linux. Update the `KDIR` variable to point to your linux-socfpga repository directory.

Run `make` in this directory to build to kernel module.

## Device tree node

Use the following device tree node:
```devicetree
 rotary: rotary@ff230000 {
	compatible = "Kaiser,rotary";
	reg = <0xff230000 16>;
    };
```
## Usage
Run `sudo insmod rotary.ko` to load the driver on the FPGA. 

Navigate to the drivers attribute files by running `cd /sys/devices/platform/ff230000`. 

The rotary encoder is a read only device. in `platform/ff230000` the rotary encoder output and enable register can be read with `cat`.

## Register Map
The device driver has 2 system attribute files that can be communicated with. The 2 attributes are Rotary Encoder Output State and Enable. Each attribute file is a 32 bit register mapped to the FPGA.

| Offset | Name         | R/W | Purpose                    |
|--------|--------------|-----|----------------------------|
| 0x0    | output       | R   | rotary encoder state out   |
| 0x4    | enable       | R   | button enable state        |
