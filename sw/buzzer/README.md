# Buzzer Rotary Encoder user-space software info

## Overview
This code interacts with two devices: a rotary encoder and a buzzer, by reading and writing to specific registers. It enables the buzzer based on the state of the rotary encoder and adjusts the buzzer's frequency and volume, with volume controlled by the rotary encoder's position and frequency set by user input.

## Building
This file needs to be compiled with `arm-linux-gnueabihf-gcc` to create an executable

## Usage
Run `sudo insmod buzzer.ko` to load the buzzer driver on the FPGA. 
Run `sudo insmod rotary.ko` to load the rotary driver on the FPGA.

run the program with `sudo ./buzzer`

