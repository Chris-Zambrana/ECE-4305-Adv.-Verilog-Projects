# Lab 12 VGA Square Test-Pattern Generator

## Overview

This project is a VGA display controller that renders a colored square in the center of a 640×480 monitor.

Key features:

* square_pattern_gen: Generates the square pattern by checking if each pixel coordinate falls within square boundaries centered at (320, 240). Inside the square, it displays the selected color; outside, it inverts the color for contrast.

* vga_sync_demo: Handles VGA synchronization signals (hsync, vsync) for proper monitor timing and outputs RGB data during the active display area.

* frame_counter: Tracks pixel position (hc, vc) across the display and manages frame boundaries.

* top_vga: Ties everything together, using switches to control:
  * Bits 13:12 — Square size (16×16, 32×32, 64×64, or 128×128 pixels)
  * Bits 11:0 — Square color (12-bit RGB: 4 bits each for R, G, B)

Essentially, it's an FPGA-based VGA controller that lets you display a centered, resizable, colored square on a monitor using FPGA board switches.

Please refer to the following PDF file for detailed instructions and description of the lab:
- [Lab Instructions](Lab_12_Square_Test_Pattern_Generator/images/Lab%2012%20-%20Square%20Test-Pattern%20Generator.pdf)




