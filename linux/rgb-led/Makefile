ifneq ($(KERNELRELEASE),)
# kbuild part of makefile
obj-m := rgb_led.o

else
# normal makefile

# path to kernel directory
KDIR ?= ~/labs-and-homework-sethhoward17/linux/hello-world/linux-socfpga/

default:
	$(MAKE) -C $(KDIR) ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- M=$$PWD
	
clean:
	$(MAKE) -C $(KDIR) M=$$PWD clean
endif