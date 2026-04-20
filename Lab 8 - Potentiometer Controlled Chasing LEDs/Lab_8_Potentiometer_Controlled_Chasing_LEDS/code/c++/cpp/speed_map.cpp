#include "speed_map.h"
#include "chu_init.h"

/* Modular Design
    What can change?
    * initial speed, max speed, initial lvl, max lvl
        - different applications have different requirements for these parameters
        - Ex: in contrast to this design, a motor may want to stay still for initial lvl whereas 
            in this design, the initial speed is 3250 ms, which is not still but slow enough to be considered as the starting point of the speed range

    * slide/reset switch
        - the switch itself can change positions or even how the signal comes through can be as well
        - Ex: the switch could be on bit 10 instead of bit 0, or the signal can come through a button

    * output 
        - the output can be used to control something else other than the led, such as a motor 
        - Ex: the output can be used to control the speed of a motor

    Key Factors:
    *UART only outputs a message when speed changes
    *The Speed the user wants is determined by switches
    *The slider switch moves the lit LED to the rightmost position and stays there until the user switches it again
    *speed = ((max_speed - min_speed) / (max_lvl - min_lvl)) * (lvl) + min_speed;

    Order of Implementation:
    1. Initialize the necessary variables and components
     - min_speed = 3250 ms, max_speed = 50 ms, min_lvl = 0, max_lvl = 15
     - instantiate the switch and led
     - set the initial speed to be the slowest speed (min_speed)

    2. Get the speed value from the switch, 
    
    3. Calculate Speed using formula 
    
    4. have led_check() function run at that speed, and print it out through UART
    
    3. When LED hits leftmost or rightmost, then go into opposite direction
    
    4. Continously check the slide switch for a change 
    
    5. If speed changes, print out the new speed through UART (might have to use a flag to check if speed has changed)
    
    6.
*/  

SpeedMap::SpeedMap(int x1, int x2,int y1, int y2) {
    min_speed = y1;
    max_speed = y2;
    min_lvl = x1;
    max_lvl = x2;
    lvl = min_lvl;
    speed = min_speed;
}

SpeedMap::~SpeedMap() {
}

int SpeedMap::speed_calc(int current_lvl) {
    int temp = speed;
    lvl = current_lvl;
    if (max_lvl == min_lvl) {
        speed = min_speed;
        return speed;
    }
    speed = min_speed + ((max_speed - min_speed) * (lvl - min_lvl)) / (max_lvl - min_lvl);

    if (speed != temp) {
        uart_speed_check(speed);
    }

    return speed;
}

void SpeedMap::uart_speed_check(int speed) {
   uart.disp("current speed:");
   uart.disp(speed);
   uart.disp("ms"); // units
   uart.disp("\n\r");
}