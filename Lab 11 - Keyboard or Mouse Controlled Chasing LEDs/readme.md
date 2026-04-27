# Lab 11 Keyboard and Mouse Controlled Chasing LEDs

## Overview

This project extends the basic Chasing LEDs experiment (Lab 6) by adding PS/2 keyboard or mouse control instead of physical switches and buttons. The system allows users to dynamically control the speed of the LED chasing effect and pause/resume the animation. The FPGA we use (Nexys A7-100T) only has 1 USB port that supports PS2 interface, so code I've provided can only handle 1 input either keyboard or mouse. Both devices are supported, but only one can be plugged in and BEFORE REPROGRAMMING because my init() function is called before the while loop since init() has some delays I didn't want interfering with the timing of the system.

Please refer to the following PDF file for detailed instructions and description of the lab:
- [Lab Instructions](Lab_11_Keyboard_or_Mouse_Controlled_Chasing_LEDs/images/Lab%2011%20-%20Keyboard%20or%20mouse%20controlled%20chasing%20LED.pdf)

## Video Demo



