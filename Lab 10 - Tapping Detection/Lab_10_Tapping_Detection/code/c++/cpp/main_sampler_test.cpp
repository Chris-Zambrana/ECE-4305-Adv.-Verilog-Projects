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

double y_calc(double x, double x0, double x1, double y0, double y1);

/*
 * read temperature from adt7420
 * @param adt7420_p pointer to adt7420 instance
 */
void tapping_detection(SpiCore *spi_p, GpoCore *led_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t DATA_REG = 0x08;
   const float raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw, yraw, zraw;
   float x, y, z, magnitude, led_mag;
   int i, led_map;

   spi_p->set_freq(400000);
   spi_p->set_mode(0, 0);

   // read 8-bit x/y/z g values once
   spi_p->assert_ss(0);    // activate
   spi_p->transfer(RD_CMD);  // for read operation
   spi_p->transfer(DATA_REG);  //
   xraw = spi_p->transfer(0x00);
   yraw = spi_p->transfer(0x00);
   zraw = spi_p->transfer(0x00);
   spi_p->deassert_ss(0);
   x = (float) xraw / raw_max;
   y = (float) yraw / raw_max;
   z = (float) zraw / raw_max;
   magnitude = sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2));

   /* !! default value of magnitude on table is .978g-.945g, so set threshold to 1.0g to detect tapping on table !! 
      
      Theoretical max magnitude is about 3g, but experimentally it is 2.1g  
      Theoretical min magnitude is 0g, but experimentally it is 0.02g

      For Mag >.978, .07g
      For Mag <.945, .06g
   */

   if (magnitude < .945 || magnitude > .978) {
      uart.disp("Tap Detected! Magnitude (g): ");
      uart.disp(magnitude, 3);
      uart.disp("\n\r");

      // Since magnitude can be above or below 1.0g, it's hard to determine how to map magnitude to LED levels. 
      // So we will take the absolute value of the difference between magnitude and 1.0g to have a consistent way to map magnitude to LED levels. 
      led_mag = fabs(magnitude - 1.0);
      uart.disp("Magnitude Lvl: ");
      uart.disp(led_mag, 3);
      uart.disp("\n\r");

      // Map magnitude to LED levels (0-16) using linear interpolation.
      led_map = (int) round(y_calc(led_mag, 0.0, .8, 0.0, 16.0));

      // Display LED level (aka how many LEDs light up)
      uart.disp("LED Lvl: ");
      uart.disp(led_map, 3);\
      uart.disp("\n\r");

      // Light up LEDs according to the mapped LED level. 
      // sleep_ms(20) is added between lighting up each LED to make it more visually appealing.
      for (i = 0; i < led_map; i++) {
         led_p->write(1, i);
         sleep_ms(20);
      }
      sleep_ms(250);
      led_p->write(0);
   } 

   /* Test for max and min magnitudes experimentally to get accurate led outputs
   if (magnitude < .02 || magnitude > 2.1) {
      uart.disp("Max or Min Magnitude Detected: ");
      uart.disp(magnitude, 3);
      uart.disp("\n\r");

   } 
   */

}

// linerar interpolation function to map ADC reading to duty cycle for RGB LEDs
double y_calc(double x, double x0, double x1, double y0, double y1) {
   double y;
   if (x1 == x0) { // avoid division by zero
        return y0;
    }
    y = y0 + ((y1 - y0) * (x - x0)) / (x1 - x0);

    return y;
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


int main() {
   //uint8_t id, ;

   while (1) {
      tapping_detection(&spi, &led);
   } //while
} //main

