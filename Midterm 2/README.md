# Midterm 2 - Custom 3-axis Compass Pmod Implentation of FPGA

## Overview
FPGA-based interface for the Digilent Pmod CMPS2 3-axis magnetometer/compass using I2C, verified device communication by reading the product ID register and printing results over UART, and developed the software/data path needed to convert raw X/Y/Z register data into readable compass headings. The project included understanding the board pinout and pull-up configuration, handling the sensor’s midpoint-centered magnetic data format, applying saved calibration offsets to reduce fixed magnetic bias from the hardware/environment, using SET/RESET maintenance operations for sensor calibration to improve sensor reliability, and mapping corrected magnetometer readings into human-readable directions such as North, East, South, and West. The work also explored the limits of a magnetometer-only compass, including the need for tilt compensation and more advanced calibration for phone-like robustness but it was not used in the demo as it did not work under the class environment.

## Psuedocode
The Psuedocode below goes through all steps in order to achieve the final coordinate results that give the heading of the compass. The heading matches to the directions similarly to the image shown below, and this can be seen via the code at the very bottom of the compass_check function

![Compass_heading_image](Midterm%202/images/Compass_heading_image.jpg)
![Psuedocode_1](Midterm%202/images/Psuedocode_1.png)
![Psuedocode_2](Midterm%202/images/Psuedocode_2.png)

## Video Demo


## References 
Below is all the references used to complete the project, including data sheets and schematics of the PCB

![Compass_pmod_schematic](Midterm%202/images/Compass_pmod_schematic.pdf)
![Compass_pmod_datasheet](Midterm%202/images/Compass_pmod_datasheet.pdf)
![Compass_magnetometer_datasheet](Midterm%202/images/Compass_magnetometer_datasheet.pdf)



