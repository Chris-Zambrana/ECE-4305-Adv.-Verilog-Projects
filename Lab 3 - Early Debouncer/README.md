# Lab 3 - Early Debouncer

## Project Instructions and Description

### Overview
This project implements an early detection debouncing circuit based on section 5.5.2 of the course material. The early detection scheme responds immediately when the input changes from 0 to 1, then ignores the input for approximately 20 ms to avoid glitches. After this period, the FSM starts checking for the falling edge.

📄 **[Lab 3 - Early Debouncer Instructions (PDF)](images/Lab%203%20-%20Early%20Debouncer%20(1).pdf)**

## FSM & ASM Chart

![Lab 3 Early Debouncer ASM](images/Lab%203%20Early%20Debouncer%20ASM.drawio.png)
![FSM & ASM Chart Screenshot](images/Screenshot%202026-02-09%20235446.png)

## Oscilloscope Image
![Oscilloscope Image](images/Lab%203%20Oscilloscope%20Picture.jpg)

At cursor X1 (the measurement at the negative edge of the clock), the db signal instantly goes low at the same time the button (sw) changes. However, I captured the moment when the button (sw) changes before the 20-30ms delay is finished within the output (db). The Cursor measurement shows that the delay for the output is 25ms, staying low and then once that delay is done, then it starts reading the input from the sw and it instantly goes high. 

## Video Demo

<!-- Add your video demo link here -->
<!-- Example: [Video Demo](your-video-link) -->
