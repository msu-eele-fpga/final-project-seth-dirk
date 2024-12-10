# RGB LED Control VHDL Component

## Overview
The RGB LED control vhdl component consists of the process to drive a PWM signal to each color of the LED and establish the overall period as well. The base address is 0x001db060 as this was assigned to Seth inidividually earlier in the semester. 

## Register Map
The VHDL component has 4 registers that are established in the avalon wrapper. The map for each register is below.

| Offset | Name              | R/W | Purpose                    |
|--------|-------------------|-----|----------------------------|
| 0x0    | PWM Period        | R/W | Set the PWM Period         |
| 0x4    | Red Duty Cycle    | R/W | Set the red duty cycle     |
| 0x8    | Green Duty Cycle  | R/W | Set the green duty cycle   |
| 0xC    | Blue Duty Cycle   | R/W | Set the blue duty cycles   |

## Data Type Expectations
The period register is a 32 bit register with 26 fractional bits. With this fixed point configuration in mind, a 1 corresponds to a 1 ms PWM period. The duty cycle registers are 32 bits, but only the 20 least significant bits are considered for the conversion with 19 fractional bits. This fixed point conifguration was individually assigned to Seth earlier in the semester. A fixed point value of 1 corresponds to a 100% duty cycle.

## Top Level Routing
The period signal is internal. The red PWM signal is routed to GPIO_1(0). The green PWM signal is routed to GPIO_1(1). The blue PWM signal is routed to GPIO_1(2).