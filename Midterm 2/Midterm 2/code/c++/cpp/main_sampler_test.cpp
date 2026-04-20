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
#include <cstring>

struct CompassCalibration {
   double x_off, y_off, z_off;
};

/* Unused for Demo, for tilt compensation
struct AccelerometerCalibration {
   double ax_off, ay_off, az_off;
};

struct AccelerometerVals{
   double x, y, z;
};
*/

struct RGB_Duty_Map
{
   double r_duty_val;
   double g_duty_val;
   double b_duty_val;
};

void compass_check(I2cCore *compass_p, SpiCore *accelerometer_p, SsegCore *sseg_p, PwmCore *pwm_p, GpoCore *led_p);
CompassCalibration compass_calibration(I2cCore *compass_p);

/* Unused for Demo, for tilt compensation
AccelerometerCalibration accelerometer_calibration(SpiCore *accelerometer_p);
AccelerometerVals accelerometer_check(SpiCore *spi_p);
*/

void master_display(SsegCore *sseg_p, PwmCore *pwm_p, double heading_deg, const char *direction);
void rgb_map(PwmCore *pwm_p, const char *direction);
         
const uint8_t DEV_ADDR = 0x30;
const int CAL_SAMPLES = 3; // number of samples to take for calibration
const int MEAS_SAMPLES = 1; // number of samples to take for measurement
const int DISP_NUM = 3; // number of digits to display on the seven segment display

void compass_check(I2cCore *compass_p, SpiCore *accelerometer_p, SsegCore *sseg_p, PwmCore *pwm_p, GpoCore *led_p) {
   uint8_t wbytes[6], rbytes[6];
   //int ack;
   int16_t xc[MEAS_SAMPLES], yc[MEAS_SAMPLES], zc[MEAS_SAMPLES], x_sum = 0, y_sum = 0, z_sum = 0;
   double ax_sum = 0, ay_sum = 0, az_sum = 0, ax_avg, ay_avg, az_avg, ax, ay, az, pitch_den, roll_den, x_avg, y_avg, z_avg, pitch, roll, 
      x_corr, y_corr, z_corr, x, y, z, xh, yh, heading_deg;
   int meas_done = 0;

   compass_p->set_freq(100000);  // set i2c clock to 100K Hz

   CompassCalibration compass_off;
   compass_off = compass_calibration(compass_p); // perform calibration to get the offset for each axis

   /*Unused for Demo, for tilt compensation
   AccelerometerVals accelerometer_vals[MEAS_SAMPLES];
   */

   /* Product ID Check
   wbytes[0] = 0x20; // id reg addr
   compass_p->write_transaction(DEV_ADDR, wbytes, 1, 1);
   compass_p->read_transaction(DEV_ADDR, rbytes, 1, 0);
   uart.disp("read compass id (should be 0x06): ");
   uart.disp(rbytes[0], 16);
   uart.disp("\n\r");
   */

   for(int i = 0; i < MEAS_SAMPLES; i++) {
   
      wbytes[0] = 0x07; // Control Reg 0 addr
      wbytes[1] = 0x01; // Start Data Aquisition 
      compass_p->write_transaction(DEV_ADDR, wbytes, 2, 0); 

      wbytes[0] = 0x06; // Status Reg addr
      compass_p->write_transaction(DEV_ADDR, wbytes, 1, 1); 
      while (!meas_done) {
         compass_p->read_transaction(DEV_ADDR, rbytes, 1, 0);
         if (rbytes[0] & 0x01) { // check if measurement done
            meas_done = 1;
         }
      }
      meas_done = 0; // reset measurement done flag for next measurement
      
      /*Unused for Demo, for tilt compensation

      // Get accelerometer measurements and calculate pitch and roll for each measurement sample
      accelerometer_vals[i] = accelerometer_check(accelerometer_p);

      */

      /* Default null field output is 32768, so we need to subtract 
         our raw measurement with the null field output to get the 
         actual magnetic field strength in each axis where 0 is the zero point
         instead of 32768
      */
      wbytes[0] = 0x00; // X LSB reg addr 
      compass_p->write_transaction(DEV_ADDR, wbytes, 1, 1);
      compass_p->read_transaction(DEV_ADDR, rbytes, 6, 0); // read X/Y/Z calibration data from X LSB reg to Z MSB reg (0x00 to 0x05)
      xc[i] = (((uint16_t)(rbytes[1]) << 8) + (uint16_t)(rbytes[0])) - 32768; // read raw measurement and convert to 16-bit int (by default 16-bit resolution)
      yc[i] = (((uint16_t)(rbytes[3]) << 8) + (uint16_t)(rbytes[2])) - 32768; // read raw measurement and convert to 16-bit int (by default 16-bit resolution)
      zc[i] = (((uint16_t)(rbytes[5]) << 8) + (uint16_t)(rbytes[4])) - 32768; // read raw measurement and convert to 16-bit int (by default 16-bit resolution)

      // Sum up the MEAS_SAMPLES samples for each axis to prepare for average calculation
      x_sum = x_sum + xc[i];
      y_sum = y_sum + yc[i];
      z_sum = z_sum + zc[i];

      /*Unused for Demo, for tilt compensation
      // Sum up the pitch and roll calculated from the MEAS_SAMPLES samples to prepare for average calculation
      ax_sum = ax_sum + accelerometer_vals[i].x;
      ay_sum = ay_sum + accelerometer_vals[i].y;
      az_sum = az_sum + accelerometer_vals[i].z;
      */
   }

   // Calculate the average for each axis
   x_avg = (double)x_sum / MEAS_SAMPLES;
   y_avg = (double)y_sum / MEAS_SAMPLES;
   z_avg = (double)z_sum / MEAS_SAMPLES;

   // Subtract the offset from the newly converted x/y/z values to get the calibrated magnetic field strength in each axis
   x_corr = x_avg - compass_off.x_off;
   y_corr = y_avg - compass_off.y_off;
   z_corr = z_avg - compass_off.z_off;

   /* Now divide by 2048 or multiply by 0.48828125 mG to convert the 
      measurement from counts to Gauss (G) unit or mG unit depending on 
      what operation you do
   */
   x = x_corr / 2048.0; // convert to G unit
   y = y_corr / 2048.0; // convert to G unit
   z = z_corr / 2048.0; // convert to G unit

   /* Unused for Demo, for tilt compensation

   // Calculate the average for accelerometer measurements for pitch and roll calculation
   ax_avg = ax_sum / MEAS_SAMPLES;
   ay_avg = ay_sum / MEAS_SAMPLES;
   az_avg = az_sum / MEAS_SAMPLES;

   ax = ax_avg - accelerometer_off.ax_off; // subtract the offset from the newly converted ax value 
   ay = ay_avg - accelerometer_off.ay_off; // subtract the offset from the newly converted ay value 
   az = az_avg - accelerometer_off.az_off; // subtract the offset from the newly converted az value 

   // Calculate pitch and roll 
   pitch_den = sqrt(ax * ax + az * az);
   roll_den = sqrt(ay * ay + az * az);

   roll = atan2(ax, roll_den);
   pitch = atan2(ay, pitch_den);

   uart.disp("Roll = ");
   uart.disp(roll * 180.0 / M_PI);
   uart.disp(" | ");
   uart.disp(" Pitch = ");
   uart.disp(pitch * 180.0 / M_PI);
   uart.disp("\n\r");

   // Tilt compensation formulas
   xh = x * cos(pitch) + z * sin(pitch);
   yh = x * sin(roll) * sin(pitch) + y * cos(roll) - z * sin(roll) * cos(pitch);

   */

   // Calculate heading in degree and direction based on tilt-compensated x/y values

   heading_deg = atan2(y, x) * 180.0 / M_PI; // if you want to calculate heading without tilt compensation, use this formula instead

    // Add an offset angle to align the compass heading with the FPGA board facing forward
    // Note1: depending on where you place the compass sensor on the FPGA board, you may need to adjust the offset angle to make sure the compass heading is aligned with the direction the FPGA board is facing
    // Ex: mine is place on the right side of the board at JB Pmod header and the compass point's toward the boundary between W and SW by default.
    // Note2: similar to that of a iPhone compass where the direction you get is based on where the front of the phone is facing
    // i.e. the direction you get from compass is based on where the front of the FPGA board is facing 
   
    heading_deg = heading_deg + 88.0; 
   
   if (heading_deg < 0) {
      heading_deg += 360.0; // convert to 0-360 degree range
   }

   if (heading_deg > 360.0) {
      heading_deg -= 360.0; // wrap around to 0-360 degree range
   }

   const char *direction;
   if ((heading_deg > 337.25) || (heading_deg < 22.5)) {
      direction = "N";
   } else if (heading_deg >= 292.5 && heading_deg <= 337.25) {
      direction = "NW";
   } else if (heading_deg >= 247.5 && heading_deg < 292.5) {
      direction = "W";
   } else if (heading_deg >= 202.5 && heading_deg < 247.5) {
      direction = "SW";
   } else if (heading_deg >= 157.5 && heading_deg < 202.5) {
      direction = "S";
   } else if (heading_deg >= 112.5 && heading_deg < 157.5) {
      direction = "SE";
   } else if (heading_deg >= 67.5 && heading_deg < 112.5) {
      direction = "E";
   } else {
      direction = "NE";
   }

   // display the heading in degree and direction on numerous interfaces
   master_display(sseg_p, pwm_p, heading_deg, direction); 

   // wait 1/3 of acquisition time (2.64ms) before next measurement, but master_display already covers this
   
}

CompassCalibration compass_calibration(I2cCore *compass_p) {
   uint8_t wbytes[6], rbytes[6];
   int16_t x_pos_sample[CAL_SAMPLES], y_pos_sample[CAL_SAMPLES], z_pos_sample[CAL_SAMPLES], x_neg_sample[CAL_SAMPLES], y_neg_sample[CAL_SAMPLES], z_neg_sample[CAL_SAMPLES];
   int32_t x_sum_pos = 0, y_sum_pos = 0, z_sum_pos = 0, x_sum_neg = 0, y_sum_neg = 0, z_sum_neg = 0;
   double x_off, y_off, z_off, x_avg_pos, y_avg_pos, z_avg_pos, x_avg_neg, y_avg_neg, z_avg_neg;
   int meas_done = 0;

   CompassCalibration compass_off;

   // SET Action Process (Find the max magnetic field strength in each axis on + direction)
   for(int i = 0; i < CAL_SAMPLES; i++) {

      // Recharge capacitor for SET action
      wbytes[0] = 0x07; // Control Reg 0 addr
      wbytes[1] = 0x80; // command to recharge the capacitor to prepare for the SET action
      compass_p->write_transaction(DEV_ADDR, wbytes, 2, 0); 
      sleep_ms(55); // wait for capacitor recharge (50ms min)

      // SET Action Process (Find the max magnetic field strength in each axis on + direction)
      wbytes[0] = 0x07; // Control Reg 0 addr
      wbytes[1] = 0x20; // Start SET action
      compass_p->write_transaction(DEV_ADDR, wbytes, 2, 0); 
      sleep_ms(2); // wait for SET action(1ms min)

      wbytes[0] = 0x07; // Control Reg 0 addr
      wbytes[1] = 0x01; // Start Data Aquisition 
      compass_p->write_transaction(DEV_ADDR, wbytes, 2, 0); 

      wbytes[0] = 0x06; // Status Reg addr
      compass_p->write_transaction(DEV_ADDR, wbytes, 1, 1); 
      while (!meas_done) {
         compass_p->read_transaction(DEV_ADDR, rbytes, 1, 0);
         if (rbytes[0] & 0x01) { // check if measurement done
            meas_done = 1;
         }
      }
      meas_done = 0; // reset measurement done flag for next measurement

      wbytes[0] = 0x00; // X LSB reg addr 
      compass_p->write_transaction(DEV_ADDR, wbytes, 1, 1);
      compass_p->read_transaction(DEV_ADDR, rbytes, 6, 0); // read X/Y/Z calibration data from X LSB reg to Z MSB reg (0x00 to 0x05)
      x_pos_sample[i] = (((uint16_t)(rbytes[1]) << 8) + (uint16_t)(rbytes[0])) - 32768; // default resolution is 16-bit (BW[1:0] == 00), so convert X calibration data to int and store as x_max
      y_pos_sample[i] = (((uint16_t)(rbytes[3]) << 8) + (uint16_t)(rbytes[2])) - 32768; // default resolution is 16-bit (BW[1:0] == 00), so convert Y calibration data to int and store as y_max
      z_pos_sample[i] = (((uint16_t)(rbytes[5]) << 8) + (uint16_t)(rbytes[4])) - 32768; // default resolution is 16-bit (BW[1:0] == 00), so convert Z calibration data to int and store as z_max

      sleep_ms(5); // wait before next calibration measurement

      // RESET Action Process (Find the min magnetic field strength in each axis on - direction)

      // Recharge capacitor for RESET action
      wbytes[0] = 0x07; // Control Reg 0 addr
      wbytes[1] = 0x80; // command to recharge the capacitor to prepare for the RESET action
      compass_p->write_transaction(DEV_ADDR, wbytes, 2, 0); 
      sleep_ms(55); // wait for capacitor recharge (50ms min)

      // RESET Action Process (Find the min magnetic field strength in each axis on - direction)
      wbytes[0] = 0x07; // Control Reg 0 addr
      wbytes[1] = 0x40; // Start RESET action
      compass_p->write_transaction(DEV_ADDR, wbytes, 2, 0); 
      sleep_ms(2); // wait for RESET action(1ms min)

      wbytes[0] = 0x07; // Control Reg 0 addr
      wbytes[1] = 0x01; // Start Data Aquisition 
      compass_p->write_transaction(DEV_ADDR, wbytes, 2, 0); 

      wbytes[0] = 0x06; // Status Reg addr
      compass_p->write_transaction(DEV_ADDR, wbytes, 1, 1); 
      while (!meas_done) {
         compass_p->read_transaction(DEV_ADDR, rbytes, 1, 0);
         if (rbytes[0] & 0x01) { // check if measurement done
            meas_done = 1;
         }
      }
      meas_done = 0; // reset measurement done flag for next measurement

      wbytes[0] = 0x00; // X LSB reg addr 
      compass_p->write_transaction(DEV_ADDR, wbytes, 1, 1);
      compass_p->read_transaction(DEV_ADDR, rbytes, 6, 0); // read X/Y/Z calibration data from X LSB reg to Z MSB reg (0x00 to 0x05)
      x_neg_sample[i] = (((uint16_t)(rbytes[1]) << 8) + (uint16_t)(rbytes[0])) - 32768; // default resolution is 16-bit (BW[1:0] == 00), so convert X calibration data to int and store as x_min
      y_neg_sample[i] = (((uint16_t)(rbytes[3]) << 8) + (uint16_t)(rbytes[2])) - 32768; // default resolution is 16-bit (BW[1:0] == 00), so convert Y calibration data to int and store as y_min
      z_neg_sample[i] = (((uint16_t)(rbytes[5]) << 8) + (uint16_t)(rbytes[4])) - 32768; // default resolution is 16-bit (BW[1:0] == 00), so convert Z calibration data to int and store as z_min

      // Sum up the CAL_SAMPLES samples for each axis in both + and - direction to prepare for average calculation
      x_sum_pos = x_sum_pos + x_pos_sample[i];
      y_sum_pos = y_sum_pos + y_pos_sample[i];
      z_sum_pos = z_sum_pos + z_pos_sample[i];
      x_sum_neg = x_sum_neg + x_neg_sample[i];
      y_sum_neg = y_sum_neg + y_neg_sample[i];
      z_sum_neg = z_sum_neg + z_neg_sample[i];

      sleep_ms(5); // wait before next calibration measurement
   }
   
   // Find the average value of the CAL_SAMPLES samples for each axis in both + and - direction 
   x_avg_pos = x_sum_pos / CAL_SAMPLES;
   y_avg_pos = y_sum_pos / CAL_SAMPLES;
   z_avg_pos = z_sum_pos / CAL_SAMPLES;
   x_avg_neg = x_sum_neg / CAL_SAMPLES;
   y_avg_neg = y_sum_neg / CAL_SAMPLES;
   z_avg_neg = z_sum_neg / CAL_SAMPLES;

   // Calculate the offset for each axis and store as x_off/y_off/z_off
   x_off = (x_avg_pos + x_avg_neg) / 2.0;
   y_off = (y_avg_pos + y_avg_neg) / 2.0;
   z_off = (z_avg_pos + z_avg_neg) / 2.0;

   compass_off.x_off = x_off;
   compass_off.y_off = y_off;
   compass_off.z_off = z_off;

   return compass_off;
}

/* Unused for Demo, for tilt compensation
AccelerometerVals accelerometer_check(SpiCore *spi_p) {
   const uint8_t RD_CMD = 0x0b;
   const uint8_t DATA_REG = 0x08;
   const double raw_max = 127.0 / 2.0;  //128 max 8-bit reading for +/-2g

   int8_t xraw, yraw, zraw;
   double x, y, z;

   AccelerometerVals accelerometer_vals;

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
   x = (double) xraw / raw_max;
   y = (double) yraw / raw_max;
   z = (double) zraw / raw_max;
   
   accelerometer_vals.x  = x;
   accelerometer_vals.y = y;
   accelerometer_vals.z = z; // assuming z is already in the correct units

   return accelerometer_vals;
}

AccelerometerCalibration accelerometer_calibration(SpiCore *accelerometer_p) {

   double xoff_sum = 0, yoff_sum = 0, zoff_sum = 0, xoff_avg, yoff_avg, zoff_avg;

   AccelerometerVals accelerometer_vals[CAL_SAMPLES];

   for (int i = 0; i < CAL_SAMPLES; i++) {
      // read accelerometer values and calculate the average to get the offset for each axis
      // similar to compass calibration

      accelerometer_vals[i] = accelerometer_check(accelerometer_p);

      xoff_sum = xoff_sum + accelerometer_vals[i].x;
      yoff_sum = yoff_sum + accelerometer_vals[i].y;
      zoff_sum = zoff_sum + accelerometer_vals[i].z;

   }

   xoff_avg = xoff_sum / CAL_SAMPLES;
   yoff_avg = yoff_sum / CAL_SAMPLES;
   zoff_avg = zoff_sum / CAL_SAMPLES;

   // Store the calculated offsets
   AccelerometerCalibration accelerometer_off;
   accelerometer_off.ax_off = xoff_avg;
   accelerometer_off.ay_off = yoff_avg;
   accelerometer_off.az_off = zoff_avg + 1;

   return accelerometer_off;
}
*/

void master_display(SsegCore *sseg_p, PwmCore *pwm_p, double heading_deg, const char *direction) {
   int i, integer_val;
   const char *color;

   /* UART DISPLAY */
   uart.disp("Heading (deg): ");
   uart.disp((int)heading_deg);
   uart.disp(" -> ");
   uart.disp(direction);
   uart.disp("\n\n\r");


   /* SEVEN SEGMENT DISPLAY SETUP*/

   //turn off all decimal points
   sseg_p->set_dp(0x00); // turn on the decimal point for the 6th digit to represent the decimal point in the displayed number

   // heading is a double value between 0 and 360, so we need to convert it to an integer value for the seven segment display
   integer_val = (int)(heading_deg); // convert to an integer value

   sseg_p->write_1ptn(0xff, 2); // turn off the leftmost position
   sseg_p->write_1ptn(0xff, 3); // turn off the second leftmost position

   // display the degree symbol on the 5th seven segment display
   sseg_p->write_1ptn(0x9c, 4); 

   // extract each digit and display on the seven segment display
   for (i = 5; i < DISP_NUM + 5; i++) {
      sseg_p->write_1ptn(sseg_p->h2s(integer_val % 10), i); // display the least significant digit at the rightmost position
      integer_val = integer_val / 10; // remove the least significant digit
   }

   /* DIRECTION DISPLAY */

   // Conditional statement to match the direction with corresponding display patterns
   if (strcmp(direction, "N") == 0) {
      sseg_p->write_1ptn(0xab, 0); // display "N" at the rightmost position of the seven segment display
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "purple"; // set color to red for N direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels based on the color

   } else if (strcmp(direction, "NE") == 0) {
      sseg_p->write_1ptn(0xab, 1); // display "N" at the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0x86, 0); // display "E" at the rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "orange"; // set color to orange for NE direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels based on the color

   } else if (strcmp(direction, "E") == 0) {
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0x86, 0); // display "E" at the rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "green"; // set color to green for E direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels based on the color

   } else if (strcmp(direction, "SE") == 0) {
      sseg_p->write_1ptn(0x92, 1); // display "S" at the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0x86, 0); // display "E" at the rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "yellow"; // set color to green for SE direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels based on the color

   } else if (strcmp(direction, "S") == 0) {
      sseg_p->write_1ptn(0x92, 0); // display "S" at the rightmost position of the seven segment display
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "light blue"; // set color to light blue for S direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels based on the color

   } else if (strcmp(direction, "SW") == 0) {
      sseg_p->write_1ptn(0x92, 1); // display "S" at the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0xd5, 0); // display "W" at the rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "blue"; // set color to blue for SW direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels based on the color

   } else if (strcmp(direction, "W") == 0) {
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0xd5, 0); // display "W" at the rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "red"; // set color to purple for W direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels based on the color

   } else if (strcmp(direction, "NW") == 0) {
      sseg_p->write_1ptn(0xab, 1); // display "N" at the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0xd5, 0); // display "W" at the rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "pink"; // set color to pink for NW direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels based on the color

   } else {
      sseg_p->write_1ptn(0xff, 0); // turn off the rightmost position of the seven segment display
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display

      /*RGB DISPLAY*/
      color = "off"; // set color to off for invalid direction
      rgb_map(pwm_p, color); // set the duty cycle for R/G/B channels to 0 to turn off the LED

   }

   sleep_ms(20); // small delay to allow the changes to take effect
}

void rgb_map(PwmCore *pwm_p, const char *color) {
   double duty_r, duty_g, duty_b;

   if (strcmp(color, "red") == 0) {
      duty_r = 1.0;
      duty_g = 0.0;
      duty_b = 0.0;

   } else if (strcmp(color, "orange") == 0) {
      duty_r = 1.0;
      duty_g = 0.15;
      duty_b = 0.0;

   } else if (strcmp(color, "yellow") == 0) {
      duty_r = 1.0;
      duty_g = 0.5;
      duty_b = 0.0;

   } else if (strcmp(color, "green") == 0) {
      duty_r = 0.0;
      duty_g = 1.0;
      duty_b = 0.0;

   } else if (strcmp(color, "light blue") == 0) {
      duty_r = 0.0;
      duty_g = 1.0;
      duty_b = 1.0;

   } else if (strcmp(color, "blue") == 0) {
      duty_r = 0.0;
      duty_g = 0.0;
      duty_b = 1.0;

   } else if (strcmp(color, "purple") == 0) {
      duty_r = 1.0;
      duty_g = 0.0;
      duty_b = 1.0;

   } else if (strcmp(color, "pink") == 0) {
      duty_r = 1.0;
      duty_g = 0.0;
      duty_b = 0.25;

   } else {
      duty_r = 0.0;
      duty_g = 0.0;
      duty_b = 0.0;
   }

   // set the duty cycle for red registers on both leds
   pwm_p->set_duty(duty_b, 0);
   pwm_p->set_duty(duty_b, 3);

   // set the duty cycle for green registers on both leds
   pwm_p->set_duty(duty_g, 1);
   pwm_p->set_duty(duty_g, 4);

   // set the duty cycle for blue registers on both leds
   pwm_p->set_duty(duty_r, 2);
   pwm_p->set_duty(duty_r, 5);
}


GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
I2cCore compass(get_slot_addr(BRIDGE_BASE, S10_I2C));
SpiCore spi(get_slot_addr(BRIDGE_BASE, S9_SPI));
PwmCore pwm(get_slot_addr(BRIDGE_BASE, S6_PWM));



int main() {
   //uint8_t id, ;
   sleep_ms(10); // wait for peripherals to be ready

   /*Unused for Demo, for tilt compensation
   AccelerometerCalibration accelerometer_offsets = accelerometer_calibration(&spi); // perform calibration to get the offset for each axis for accelerometer
   */

   while (1) {
      compass_check(&compass, &spi, &sseg, &pwm, &led); // perform compass check to get the heading and direction and display on various interfaces
   } 
} 

