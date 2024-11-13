# Final Project Proposal - Seth Howard and Dirk Kaiser

## Hardware/Software Proposal 1
The first additional component would be an LED that moves left and right with the turn of a rotary encoder. The hardware would decode the output of the rotary encoder and put the output into a register. Hardware for driving the LEDs would be needed as well, which would consist of the LED register. The software would be a program to read the rotary encoder output register register and change the LED register accordingly.

## Hardware/software proposal 2
The second additional component would be a button (from the rotary encoder) that turns a buzzer on and off. The base pitch of the buzzer would be set by a user input and could be increased/decreased with the rotary encoder. The hardware portion would consist of a register for the output of the button press and the buzzer PWM controller. There would also be a register to store the user input for the base pitch in Hz, and the math to convert this pitch to the proper PWM signal. The software would enable the buzzer based off of the button and read the user input for pitch to store in the associated register. The program would also control the pitch of the buzzer by reading the output register of the rotary encoder and writing to the duty cycle register of the PWM controller.

## Division of labor 
Dirk will work on the hardware IP for the rotary encoder and button. Seth will work on the hardware IP for the buzzer. Dirk will work on the software for the buzzer enable and pitch control. Seth will work on the software for controlling the LEDs from the rotary encoder.
