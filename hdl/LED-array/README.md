# LED Array component
This is a simple component that holds a register to control the LEDs on the FPGA. Any 8-bit value can be written to the register and the binary value will be displayed by the LED array on the de10nano. The base address for this component in 0x0002 0000.

## Data Type Expectation
The register is 32 bits long, but only the 8 least significant bits will be displayed on the LEDs. Keep this in mind when choosing values to write to the LEDs.

## Top Level Routing
The output of the led array component is the 8 least significant bits of the register, which is directly connected to the led signal at the top level.