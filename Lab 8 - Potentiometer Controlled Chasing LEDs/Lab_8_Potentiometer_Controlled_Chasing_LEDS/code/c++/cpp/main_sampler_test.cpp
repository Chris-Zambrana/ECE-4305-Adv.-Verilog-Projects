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
#include "speed_map.h"
#include <cmath>
using namespace std;

/**
 * Convert ADC Values into Speed and use the speed to control the chasing effect of the LEDs
 * @param adc_p pointer to xadc instance
 */

struct ControlSignals {
   int current_lvl;
   bool slider_sw;
};

ControlSignals sig_check(GpiCore *sw_p, XadcCore *adc_p) {
   ControlSignals s;
   int slider_sw_val;
   
   s.current_lvl = adc_p->read_raw(0) >> 4; // bits 1..5
   slider_sw_val = sw_p->read(0);                    // bit 0
   s.slider_sw = (slider_sw_val == 1) ? true : false; // convert to boolean

   return s;
}

void chasing_leds(GpoCore *led_p, SpeedMap *speed_p, GpiCore *sw_p, XadcCore *adc_p, int n) {
   int i, slider_sw, led_data, current_lvl, speed_delay, initial_led_data;
   static int boundary = 0; // 0 means moving left, 1 means moving right

   ControlSignals sw_state;

   initial_led_data = (boundary == 0) ? 0x0001 : 0x8000; // start at the rightmost or leftmost position
   led_p->write(initial_led_data); // move to the initial position

   // move to the leftmost position
   for (i = 1; i < n; i++) {
      sw_state = sig_check(sw_p, adc_p);
      slider_sw = sw_state.slider_sw;
      current_lvl = sw_state.current_lvl;
      
      if(slider_sw) {
            led_p->write(0x0001); // move to the rightmost position
            boundary = 0; // reset boundary to start moving left again
            return; 
      }
      else {

         speed_delay = speed_p->speed_calc(current_lvl); // calculate speed based on switch position
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
AdsrCore adsr(get_slot_addr(BRIDGE_BASE, S13_ADSR), &ddfs);
SpeedMap speed(0, 4095, 4145, 50); // min_lvl, max_lvl, min_speed, max_speed

int main() {
   //uint8_t id, ;

   while (1) {
      chasing_leds(&led, &speed, &sw, &adc, 16);
   } //while
} //main

