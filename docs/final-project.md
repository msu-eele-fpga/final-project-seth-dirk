# EELE 467 FPGAs Hardware Software Codesign Final Project
Montana State University Fall 2024

Seth Howard and Dirk Kaiser

## System Overview
The System we created for this final project included the required RGB LED controlled by potentiometers, as well as a buzzer controlled by user input and rotary encoder. The following block diagram outlines the project.

## Custom Component 1: Rotary Encoder
The first custom component of the project is a Rotary Encoder, implemented using VHDL to capture both the rotational state and the button press of the encoder. Detailed information about the VHDL implementation can be found in the [README](../hdl/rotary/README.md). A Linux device driver was then employed to read the encoder state and the button enable registers from the FPGA fabric. This enables interaction with the encoder data from a C program once the kernel module is loaded.

For this project, two C programs were developed to utilize the rotary encoder. The first program reads the encoder state and writes the information to a custom LED device, providing a visual indication of the encoder's rotation. The second program uses the encoder input to control the volume of a buzzer component (described later) by adjusting its frequency based on the user's setting.
## Custom Component 2: Buzzer

 ## Conclusion