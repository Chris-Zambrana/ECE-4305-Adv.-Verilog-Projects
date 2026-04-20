# Lab 7 Blinking LEDs Core

## Overview

Please refer to the following PDF file for detailed instructions and description of the lab:
- [Lab Instructions](Lab_7_Blinking_LEDS_Core/images/Lab%207%20-%20Blinking%20LEDs%20Core.pdf)

##  Schematic

![Lab 7 Schematic](Lab_7_Blinking_LEDS_Core/images/ECE_4305_Lab_7_Block_Diagram.png)

##  Core Register Mapping

![Lab 7 Core Register Mapping](Lab_7_Blinking_LEDS_Core/images/ECE_4305_Lab_7_Register_Mapping.png)

## Simulation Waveform

![Simulation Waveform](Lab_7_Blinking_LEDS_Core/images/ECE_4305_Lab_7_Waveform.png)

The data in blinking_led_reg is interpreted as miliseconds, so the values in each register are in miliseconds and then are later converted to nanoseconds because our FPGA runs at a 100MHz clock frequency 

