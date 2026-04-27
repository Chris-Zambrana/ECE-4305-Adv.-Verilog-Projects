/*****************************************************************//**
 * @file main_sampler_test.cpp
 *
 * @brief Basic test of nexys4 ddr mmio cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

// #define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "xadc_core.h"
#include "sseg_core.h"
#include "spi_core.h"
#include "i2c_core.h"
#include "ps2_core.h"
#include "ddfs_core.h"
#include "adsr_core.h"
#include <cmath>

const uint16_t MAX_NUM_DISP = 3; // Max number of digits to display speed on the seven segment display
static bool pause = false; // flag to indicate whether the chasing effect is currently paused or not

uint16_t ps2_check(Ps2Core *ps2_p, SsegCore *sseg_p, int id); // function to check for mouse or keyboard activity and update speed delay and pause state accordingly
void chasing_leds(Ps2Core *ps2_p, GpoCore *led_p, SsegCore *sseg_p, int id); // function to implement the chasing LED effect, which also calls ps2_check() to update speed and pause state
void sseg_disp(SsegCore *sseg_p, uint16_t speed); // function to display the current speed delay on the seven segment display, with an indication of whether the effect is paused or not

uint16_t ps2_check(Ps2Core *ps2_p, SsegCore *sseg_p, int id) {

   int lbtn, rbtn, xmov, ymov;
   uint8_t ch_id, num_speed_dig = 0;
   char ch;
   bool setting_mode = false; // flag to indicate whether we are in speed setting mode or not
   static uint16_t speed_delay = 999; // initial speed delay of 999 ms, which corresponds to the slowest speed for the chasing effect
   int temp, speed_calc = 0; 

   if (id == 2) {  // mouse
      if (ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov)) {

         if(lbtn) {
            pause = !pause; // toggle pause state when left button is clicked

            if(pause) {
               uart.disp("Paused. Press left mouse button to resume. \n\r");
            } else {
               uart.disp("Resumed. \n\r");
               uart.disp("Speed Delay: ");
               uart.disp(speed_delay);
               uart.disp("ms \n\r");
            }
         } 
         else if (rbtn && pause == false) { // check for right button press to enter speed setting mode, only if not currently paused
            speed_delay = 0; // reset current level to 0 before setting new value
            uart.disp("Entering speed setting mode. Please move the mouse right to decrease speed delay and move left to increase it. \n\r");
            uart.disp("Press the right mouse button again to exit speed setting mode. \n\r");
            setting_mode = true; // flag to indicate that we are in speed setting mode
            while(setting_mode) {
               if (ps2_p->get_mouse_activity(&lbtn, &rbtn, &xmov, &ymov)) {
                  if(rbtn) { // check for right button press to exit speed setting mode
                     setting_mode = false;
                     speed_delay = (uint16_t)speed_calc; // assign the calculated speed delay to the main variable after exiting setting mode
                  } else {
                     temp = (xmov * 4); // calculate using signed int to handle negative values
                     
                     temp = speed_calc + temp; // update the speed delay based on mouse movement

                     if(temp > 999) {
                        speed_calc = 999; // set speed delay to 999 if it exceeds the maximum
                     } else if (temp < 0) {
                        speed_calc = 0; // set speed delay to 0 if it goes below the minimum
                     } else {
                        speed_calc = temp; // update speed delay to the calculated value if it is within the valid range
                     }

                     uart.disp("\r\033[K");  // Move to start of line and clear to end
                     uart.disp("Speed Delay: ");

                     if(speed_calc < 100) {
                        uart.disp("0");
                        if(speed_calc < 10) {
                           uart.disp("0");
                        }
                     }
                     uart.disp(speed_calc);
                     uart.disp("ms");
                  }
               } 
            }

            uart.disp("\n\r");

         }
      }
   }   // end get_mouse_activitiy() 
   else { // keyboard
      if(ps2_p->get_kb_ch(&ch)) {
         ch_id = (uint8_t)ch;

         if(ch_id == 0xf0 && pause != true) { // check for F1 key press to enter speed setting mode only if not currently paused
            speed_delay = 0; // reset speed delay to 0 before setting new value
            uart.disp("Entering speed setting mode. Please enter up to ");
            uart.disp(MAX_NUM_DISP);
            uart.disp(" digits for speed delay (in ms). \n\r");
            uart.disp("Speed Delay: ");
            while(num_speed_dig < MAX_NUM_DISP) {
               if (ps2_p->get_kb_ch(&ch)) {
                     ch_id = (uint8_t)ch; // update ch_id with new keyboard input
                     if(ch_id >= 48 && ch_id <= 57) { // check if the input is a digit
                        speed_delay = speed_delay + (ch_id - 48) * pow(10, MAX_NUM_DISP - num_speed_dig - 1); // convert ASCII to integer and calculate the speed delay
                        uart.disp(ch_id-48); // display the entered digit);
                        num_speed_dig++; // increment the number of speed digits entered
                     }
               }
            }
            uart.disp("ms \n\r");
            
         } else if (ch_id == 80 || ch_id == 112) {
            pause = !pause; // toggle pause state when P or p key is pressed

            if(pause) {
               uart.disp("Paused. Press P to resume. \n\r");
            } else {
               uart.disp("Resumed. \n\r");
               uart.disp("Speed Delay: ");
               uart.disp(speed_delay);
               uart.disp("ms \n\r");
            }
         }
      } 
   }
   sseg_disp(sseg_p, speed_delay); // update seven segment display with current speed delay

   return speed_delay;
}

void chasing_leds(Ps2Core *ps2_p, GpoCore *led_p, SsegCore *sseg_p, int id) {
   uint16_t led_data, speed_delay;
   static int boundary = 0; // 0 means moving left, 1 means moving right

   led_data = (boundary == 0) ? 0x0001 : 0x8000; // start at the rightmost or leftmost position
   led_p->write(led_data); // move to the initial position

   // move to the leftmost position
   for (int i = 1; i < 16; i++) {

      speed_delay = ps2_check(ps2_p, sseg_p, id); // check for mouse or keyboard activity to update speed delay and pause state

      while(pause == true) {
         ps2_check(ps2_p, sseg_p, id); // continuously check for mouse or keyboard activity to check pause state while paused
      }

      sleep_ms(speed_delay); // function determining the delay, which determines the speed of the chasing effect

      if(boundary == 1){
         led_data = 0x8000 >> i; // shift the lit LED to the left
         led_p->write(led_data);

         if(led_data == 0x0001) {
            boundary = 0; // reached the leftmost position
         }
         else {
            boundary = 1; // keep moving right
         }
      } 
      else {
         led_data = 1 << i; // shift the lit LED to the left
         led_p->write(led_data);

         if(led_data == 0x8000) {
            boundary = 1; // reached the leftmost position
         }
         else {
            boundary = 0; // keep moving left
         }
      }
   }
}

void sseg_disp(SsegCore *sseg_p, uint16_t speed){

   sseg_p->write_1ptn(0xff, 6); // turn off the 7th Seven Segment Display

   if(pause == true) {
      sseg_p->write_1ptn(0x8c, 7); // Make 8th Seven Segment display "P" to indicate paused state
      sseg_p->set_dp(0x80); // turn on the decimal point for the 8th digit to represent the decimal point in the displayed number
   } else {
      sseg_p->write_1ptn(0xff, 7); // turn off the 8th Seven Segment to indicate unpaused state
      sseg_p->set_dp(0x00); // turn off the decimal point for the 8th digit to represent the decimal point in the displayed number
   }
   
   sseg_p->write_1ptn(0x92, 5); // Make 6th Seven Segment display "S" for SP
   sseg_p->write_1ptn(0x8c, 4); // Make 5th Seven Segment display "P" for SP
   sseg_p->write_1ptn(0xb7, 3); // Make 6th Seven Segment display ":" for SP

   // extract each digit and display on the seven segment display
   for (int i = 0; i < MAX_NUM_DISP; i++) {
      sseg_p->write_1ptn(sseg_p->h2s(speed % 10), i); // display the least significant digit at the rightmost position
      speed = speed / 10; // remove the least significant digit
   }
}

GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
XadcCore adc(get_slot_addr(BRIDGE_BASE, S5_XDAC));
PwmCore pwm(get_slot_addr(BRIDGE_BASE, S6_PWM));
DebounceCore btn(get_slot_addr(BRIDGE_BASE, S7_BTN));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
SpiCore spi(get_slot_addr(BRIDGE_BASE, S9_SPI));
I2cCore adt7420(get_slot_addr(BRIDGE_BASE, S10_I2C));
Ps2Core ps2(get_slot_addr(BRIDGE_BASE, S11_PS2));
DdfsCore ddfs(get_slot_addr(BRIDGE_BASE, S12_DDFS));


int main() {
   int id = ps2.init();
   while (1) {
      chasing_leds(&ps2, &led, &sseg, id);
   } 
} 
