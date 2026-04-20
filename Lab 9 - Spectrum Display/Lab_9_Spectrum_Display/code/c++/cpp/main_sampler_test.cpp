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

struct RGB_Duty_Map;
double red_map(double adc_reading);
double green_map(double adc_reading);
double blue_map(double adc_reading);
double y_calc(double x, double x0, double x1, double y0, double y1);
void sseg_disp(SsegCore *sseg_p, double adc_reading);


struct RGB_Duty_Map
{
   double r_duty_val;
   double g_duty_val;
   double b_duty_val;
};

RGB_Duty_Map rgb_map(double adc_reading) {
   RGB_Duty_Map rgb_vals;
   
   rgb_vals.r_duty_val = red_map(adc_reading);
   rgb_vals.g_duty_val = green_map(adc_reading);
   rgb_vals.b_duty_val = blue_map(adc_reading);

   return rgb_vals;
}

double red_map(double adc_reading) {
   double r_duty;
   if ((adc_reading >= 0.0 && adc_reading <= 0.16667) || (adc_reading >= 0.83335)) {
      r_duty = 1.0; // red at full brightness
   } else if(adc_reading > 0.16667 && adc_reading < 0.33334) {
      r_duty = y_calc(adc_reading, 0.16667, 0.33334, 1.0, 0.0); // linear interpolation for red duty cycle
   } else if (adc_reading >= 0.33334 && adc_reading <= 0.66668) {
      r_duty = 0.0; // red off
   } else if(adc_reading > 0.66668 && adc_reading < 0.83335) {
      r_duty = y_calc(adc_reading, 0.66668, 0.83335, 0.0, 1.0); // linear interpolation for red duty cycle
   } else {
      r_duty = 0.0; // default case, should not happen
   }
   return r_duty;
}

double blue_map(double adc_reading) {
   double b_duty;
   if ((adc_reading >= 0.0 && adc_reading <= 0.33334) || (adc_reading >= 1.0)) {
      b_duty = 0.0; // blue off
   } else if (adc_reading > 0.33334 && adc_reading < 0.5) {
      b_duty = y_calc(adc_reading, 0.33334, 0.5, 0.0, 1.0); // linear interpolation for blue duty cycle
   } else if(adc_reading >= 0.5 && adc_reading <= 0.83335) {
      b_duty = 1.0; // blue at full brightness
   } else if(adc_reading > 0.83335 && adc_reading < 1.0) {
      b_duty = y_calc(adc_reading, 0.83335, 1.0, 1.0, 0.0); // linear interpolation for blue duty cycle
   } else {
      b_duty = 0.0; // default case, should not happen
   }
   return b_duty;
}

double green_map(double adc_reading) {
   double g_duty;
   if (adc_reading > 0.0 && adc_reading < 0.16667) {
      g_duty = y_calc(adc_reading, 0.0, 0.16667, 0.0, 1.0); // linear interpolation for green duty cycle
   } else if (adc_reading >= 0.16667 && adc_reading <= 0.5) {
      g_duty = 1.0; // green on at full brightness
   } else if(adc_reading > 0.5 && adc_reading < 0.66668) {
      g_duty = y_calc(adc_reading, 0.5, 0.66668, 1.0, 0.0); // linear interpolation for green duty cycle
   } else if((adc_reading >= 0.66668) || (adc_reading <= 0.0)) {
      g_duty = 0.0; // green off
   } else {
      g_duty = 0.0; // default case, should not happen
   }
   return g_duty;
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


void pwm_spectrum_display(PwmCore *pwm_p, XadcCore *adc_p, SsegCore *sseg_p) {
   double duty_r, duty_g, duty_b, adc_reading, adc_rounded;
   pwm_p->set_freq(50); // set frequency to 50 Hz
   adc_reading = adc_p->read_adc_in(0); // read from ADC channel 0

   // round to 5 decimal places for better readability
   // & bc our mapping functions are designed for 5 decimal places
   // & bc we'll display all 5 decimal place on the 7-seg display
   adc_rounded = round(adc_reading * 100000.0) / 100000.0; 
   sseg_disp(sseg_p, adc_rounded); // display the ADC reading on the 7-segment display

   RGB_Duty_Map rgb = rgb_map(adc_rounded); // get the duty cycle values for R, G, B based on ADC reading
   duty_r = rgb.r_duty_val;
   duty_g = rgb.g_duty_val;
   duty_b = rgb.b_duty_val;

   // set the duty cycle for red registers on both leds
   pwm_p->set_duty(duty_b, 0);
   pwm_p->set_duty(duty_b, 3);

   // set the duty cycle for green registers on both leds
   pwm_p->set_duty(duty_g, 1);
   pwm_p->set_duty(duty_g, 4);

   // set the duty cycle for blue registers on both leds
   pwm_p->set_duty(duty_r, 2);
   pwm_p->set_duty(duty_r, 5);

   sleep_ms(50); // small delay to allow the changes to take effect

}

void sseg_disp(SsegCore *sseg_p, double adc_reading) {
   int i, integer_val;

   //turn off all decimal points
   sseg_p->set_dp(0x20); // turn on the decimal point for the 6th digit to represent the decimal point in the displayed number

   // adc is a double value between 0 and 1, so we need to convert it to an integer value for the seven segment display
   integer_val = (int)(adc_reading * 100000); // convert to an integer value 

   sseg_p->write_1ptn(0xff, 7); // turn off the leftmost position
   sseg_p->write_1ptn(0xff, 6); // turn off the second leftmost position

   // extract each digit and display on the seven segment display
   for (i = 0; i < 6; i++) {
      sseg_p->write_1ptn(sseg_p->h2s(integer_val % 10), i); // display the least significant digit at the rightmost position
      integer_val = integer_val / 10; // remove the least significant digit
   }

   sleep_ms(50); // small delay to allow the changes to take effect
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
      pwm_spectrum_display(&pwm, &adc, &sseg);
   } //while
} //main

