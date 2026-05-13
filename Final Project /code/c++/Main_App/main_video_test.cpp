/*****************************************************************//**
 * @file main_video_test.cpp
 *
 * @brief Basic test of 4 basic i/o cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

//#define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"
#include "servo_pwm_core.h"
#include "sensor_uart_core.h"
#include "adsr_core.h"
#include "ddfs_core.h"
#include "i2c_core.h"
#include "spi_core.h"
#include "ps2_core.h"
#include "xadc_core.h"
#include "sseg_core.h"
#include "vga_core.h"
#include <cmath> 
#include <cstring>

struct DispColors {
   int fill_color = 0x010; // default fill color to darker green
   int outline_color = 0x020; // default outline color to slightly brighter green for better visibility
   int osd_color = 0x0f0; // default osd color to green
   int sweep_color = osd_color; // default sweep color to match osd color because they both have 12 bit registers for color setting
};

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


void keyboard_check(Ps2Core *ps2_p, OsdCore *osd_p, FrameCore *frame_p, RadarCore *radar_p, int id);
void radar_init(FrameCore *frame_p, OsdCore *osd_p, RadarCore *radar_p);
void scanning_check(I2cCore *compass_p, SpiCore *accelerometer_p, OsdCore *osd_p, ServoPwmCore *pwm_servo_p, SsegCore *sseg_p);
void servo_check(ServoPwmCore *pwm_servo_p, OsdCore *osd_p, SsegCore *sseg_p);
void compass_check(I2cCore *compass_p, SpiCore *accelerometer_p, SsegCore *sseg_p);
CompassCalibration compass_calibration(I2cCore *compass_p);
void compass_sseg(SsegCore *sseg_p, const char *direction);
/* Unused for Demo, for tilt compensation
AccelerometerCalibration accelerometer_calibration(SpiCore *accelerometer_p);
AccelerometerVals accelerometer_check(SpiCore *spi_p);
*/

void ultrasonic_check(SensorUartCore *sensor_p, OsdCore *osd_p, PwmCore *pwm_p, SsegCore *sseg_p, RadarCore *radar_p);
DispColors disp_color_map();

//Global Macros
static const uint8_t MANUAL_MODE = 2; // constant to represent manual mode selection for easier readability

// Global Variables
static uint8_t servo_speed = 1;
static int angle = 0;
static uint8_t operation_mode = 0;
static bool pause = false; // flag to indicate whether operation is paused or not
static int distance = 255; // variable to hold the current distance reading from the ultrasonic sensor, initialized to an out-of-range value
static char disp_color = 'g'; // variable to hold the current color selection for the radar display, initialized to green
static uint8_t past_operation_mode = operation_mode; // variable to hold the previous mode of operation for mode change detection

// Compass Macros
const uint8_t DEV_ADDR = 0x30;
const int CAL_SAMPLES = 2; // number of samples to take for calibration
const int MEAS_SAMPLES = 1; // number of samples to take for measurement
const int DISP_NUM = 3; // number of digits to display on the seven segment display

// Ultrasonic Sensor Macros
static const int SSEG_DIST_NUM = 3; // number of digits to display for distance on the seven segment display

// Radar Macros
static const int LINE_EXTEND = 10; // length to extend the lines beyond the outer circle
static const int MAX_DISTANCE = 200; // maximum distance from the center to the outer circle
static const int RADAR_XCENTER = 320; // x-coordinate of the center of the radar
static const int RADAR_YCENTER = 240; // y-coordinate of the center of the radar



void keyboard_check(Ps2Core *ps2_p, OsdCore *osd_p, FrameCore *frame_p, RadarCore *radar_p, int id) {
   char ch;
   bool valid_input = false; 

   if (id == 1) {  // keyboard
      if(ps2_p->get_kb_ch(&ch)) {

         if(ch == 'M' || ch == 'm') { // check for F1 key press to enter speed setting mode
            uart.disp("Select mode of operation:\n\r");
            uart.disp("0 - 180 Swivel Detection\n\r");
            uart.disp("1 - 360 Swivel Detection\n\r");
            uart.disp("2 - Manual Detection (Compass)\n\r");
            uart.disp("Enter mode: ");
            while(!valid_input) {
               if (ps2_p->get_kb_ch(&ch)) {
                  uart.disp(ch);

                  if(ch >= 0x30 && ch <= 0x32) { // check if the input is a valid mode selection
                     operation_mode = ch - 0x30;
                     osd_p->set_color(0x0f0, 0x000); // set text color to green for valid input confirmation
                     radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous mode's display
                     valid_input = true;
                  } else {
                     osd_p->set_color(0xf00, 0x000); // set text color to red for error message
                     osd_p->wr_char(6, 0, 'X', 0);
                     uart.disp("\n\rInvalid input. Please enter 0, 1, or 2: ");
                     valid_input = false;
                  }
               }     
            }       
         } else if (ch == 'S' || ch == 's') { // check for S key press to enter speed setting mode
            if(operation_mode == MANUAL_MODE) {
               uart.disp("Speed setting disabled in manual mode. \n\r");
            } else {
               uart.disp("Enter speed of swivel (Valid Speeds = 1-7): ");
               while(!valid_input) {
                  if (ps2_p->get_kb_ch(&ch)) {
                     uart.disp(ch);

                     if(ch >= 0x31 && ch <= 0x37) { // check if the input is a number between 1 and 7
                        servo_speed = ch - 0x30; // convert ASCII character to integer value for speed setting
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous speed setting
                        valid_input = true;
                     } else {
                        osd_p->set_color(0xf00, 0x000); // set text color to red for error message
                        osd_p->wr_char(78, 1, 'X', 0);
                        uart.disp("\n\rInvalid input. Please enter a number from 1-7: ");
                        valid_input = false;
                     }
                  }     
               }
            }
         
         } else if(ch == 'c' || ch == 'C') {
            uart.disp("Select a color:\n\r");
            uart.disp("o or O - Orange\n\r");
            uart.disp("y or Y - Yellow\n\r");
            uart.disp("g or G - Green\n\r");
            uart.disp("b or B - Blue\n\r");
            uart.disp("p or P - Pink\n\r");
            uart.disp("Enter color: ");
            while(!valid_input) {
               if (ps2_p->get_kb_ch(&ch)) {
                  uart.disp(ch);

                  switch(ch) {
                     case ('o'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     case ('O'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     } 
                     case ('y'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     case ('Y'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     case ('g'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     case ('G'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     case ('b'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     case ('B'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     case ('p'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     case ('P'): {
                        disp_color = ch; // update color variable to reflect the current color selection
                        radar_init(frame_p, osd_p, radar_p); // re-initialize radar display to clear previous color
                        valid_input = true;
                        break;
                     }
                     default: {
                        osd_p->set_color(0xf00, 0x000); // set text color to red for error message
                        uart.disp("\n\rInvalid input. Please enter o, y, g, b, or p: ");
                        valid_input = false;
                     }
                  }
               }     
            }
         } else if (ch == 'p' || ch == 'P') {
            pause = !pause; // toggle pause state when P or p key is pressed

            if(pause) {
               while(pause) {
                  if (ps2_p->get_kb_ch(&ch)) {
                     if(ch == 'p' || ch == 'P') {
                        pause = false; // toggle pause state when P or p key is pressed again
                        uart.disp("Resumed. \n\r");
                     }
                  }
               }
            }
         } 
      } 
   } else {
      uart.disp("Keyboard input disabled. Please connect a keyboard. \n\r");
   }
}

void radar_init(FrameCore *frame_p, OsdCore *osd_p, RadarCore *radar_p) {
	static uint8_t ring_num = 4; // number of rings to display, 0 for no rings, 1 for 1 ring, etc. Map mode: 0 for 180 radar, 1 for 360 radar
   int xcoordinate_45, ycoordinate_45, xcoordinate_135, ycoordinate_135, xcoordinate_225, ycoordinate_225, xcoordinate_315, ycoordinate_315;

   DispColors disp_colors = disp_color_map(); // get the color values for the radar display based on the current color selection

   osd_p->clr_screen();
   osd_p->set_color(disp_colors.osd_color, 0x000); // dark gray/green

   frame_p->clr_screen(0x000);  // black background
   
   radar_p->set_mode(operation_mode); // set radar mode based on user selection
   radar_p->set_color(disp_colors.sweep_color); // set radar sweep color based on current color selection

   xcoordinate_45 = (int)(RADAR_XCENTER + (MAX_DISTANCE + LINE_EXTEND) * cos(45 * M_PI / 180));
   ycoordinate_45 = (int)(RADAR_YCENTER - (MAX_DISTANCE + LINE_EXTEND) * sin(45 * M_PI / 180));

   xcoordinate_135 = (int)(RADAR_XCENTER + (MAX_DISTANCE + LINE_EXTEND) * cos(135 * M_PI / 180));
   ycoordinate_135 = (int)(RADAR_YCENTER - (MAX_DISTANCE + LINE_EXTEND) * sin(135 * M_PI / 180));

   xcoordinate_225 = (int)(RADAR_XCENTER + (MAX_DISTANCE + LINE_EXTEND) * cos(225 * M_PI / 180));
   ycoordinate_225 = (int)(RADAR_YCENTER - (MAX_DISTANCE + LINE_EXTEND) * sin(225 * M_PI / 180));

   xcoordinate_315 = (int)(RADAR_XCENTER + (MAX_DISTANCE + LINE_EXTEND) * cos(315 * M_PI / 180));
   ycoordinate_315 = (int)(RADAR_YCENTER - (MAX_DISTANCE + LINE_EXTEND) * sin(315 * M_PI / 180));

   if(operation_mode == 0) {
      frame_p->fill_half_circle(RADAR_XCENTER, RADAR_YCENTER, MAX_DISTANCE, disp_colors.fill_color); // dark green filled half circle for radar background

      for(int i = 1; i <= ring_num; i++) {
         frame_p->plot_half_circle(RADAR_XCENTER, RADAR_YCENTER, 50*i, disp_colors.outline_color); // green half circles in the middle of the screen
      }

      frame_p->plot_line(RADAR_XCENTER, (RADAR_YCENTER-MAX_DISTANCE) - LINE_EXTEND, RADAR_XCENTER, RADAR_YCENTER, disp_colors.outline_color); // vertical line from top to bottom of half circle (90°)
      frame_p->plot_line((RADAR_XCENTER-MAX_DISTANCE) - LINE_EXTEND, RADAR_YCENTER, (RADAR_XCENTER+MAX_DISTANCE) + LINE_EXTEND, RADAR_YCENTER, disp_colors.outline_color); // horizontal line from left to right of half circle (0° & 180°)

      frame_p->plot_line(RADAR_XCENTER, RADAR_YCENTER, xcoordinate_135, ycoordinate_135, disp_colors.outline_color); // line at 135°
      frame_p->plot_line(RADAR_XCENTER, RADAR_YCENTER, xcoordinate_45, ycoordinate_45, disp_colors.outline_color); // line at 45°

   } else {
      frame_p->fill_circle(RADAR_XCENTER, RADAR_YCENTER, MAX_DISTANCE, disp_colors.fill_color); // dark green filled circle for radar background
   
      for(int i = 1; i <= ring_num; i++) {
         frame_p->plot_circle(RADAR_XCENTER, RADAR_YCENTER, 50*i, disp_colors.outline_color); // green circles in the middle of the screen
      }

      frame_p->plot_line(RADAR_XCENTER, (RADAR_YCENTER-MAX_DISTANCE) - LINE_EXTEND, RADAR_XCENTER, (RADAR_YCENTER+MAX_DISTANCE) + LINE_EXTEND, disp_colors.outline_color); // vertical line from top to bottom of outer circle (90° & 270°)
      frame_p->plot_line((RADAR_XCENTER-MAX_DISTANCE) - LINE_EXTEND, RADAR_YCENTER, (RADAR_XCENTER+MAX_DISTANCE) + LINE_EXTEND, RADAR_YCENTER, disp_colors.outline_color); // horizontal line from left to right of outer circle (0° & 180°)

      frame_p->plot_line(xcoordinate_45, ycoordinate_45, xcoordinate_225, ycoordinate_225, disp_colors.outline_color); // line at 45° and 225°
      frame_p->plot_line(xcoordinate_135, ycoordinate_135, xcoordinate_315, ycoordinate_315, disp_colors.outline_color); // line at 135° and 315°
      
      // 225°
      osd_p->wr_char(18, 24, '2', 0);
      osd_p->wr_char(19, 24, '2', 0);
      osd_p->wr_char(20, 24, '5', 0);
      
      // 270°
      osd_p->wr_char(39, 28, '2', 0);
      osd_p->wr_char(40, 28, '7', 0);
      osd_p->wr_char(41, 28, '0', 0);
      
      // 315°
      osd_p->wr_char(59, 24, '3', 0);
      osd_p->wr_char(60, 24, '1', 0);
      osd_p->wr_char(61, 24, '5', 0);


   }

   // Add heading labels for 360 & 180-degree radar
   // 0°
   osd_p->wr_char(67, 14, '0', 0);

   // 45°
   osd_p->wr_char(59, 5, '4', 0);
   osd_p->wr_char(60, 5, '5', 0);

   // 90°
   osd_p->wr_char(39, 1, '9', 0);
   osd_p->wr_char(40, 1, '0', 0);

   // 135°
   osd_p->wr_char(18, 5, '1', 0);
   osd_p->wr_char(19, 5, '3', 0);
   osd_p->wr_char(20, 5, '5', 0);

   // 180°
   osd_p->wr_char(10, 14, '1', 0);
   osd_p->wr_char(11, 14, '8', 0);
   osd_p->wr_char(12, 14, '0', 0);

   //Add text labels for mode, distance, and speed
   
   osd_p->wr_char(0, 0, 'M', 0);
   osd_p->wr_char(1, 0, 'O', 0);
   osd_p->wr_char(2, 0, 'D', 0);
   osd_p->wr_char(3, 0, 'E', 0);
   osd_p->wr_char(4, 0, ':', 0);
   osd_p->wr_char(5, 0, ' ', 0);
   osd_p->wr_char(6, 0, '0' + operation_mode, 0); // display current mode (0, 1, or 2)

   osd_p->wr_char(64, 0, 'D', 0);
   osd_p->wr_char(65, 0, 'I', 0);
   osd_p->wr_char(66, 0, 'S', 0);
   osd_p->wr_char(67, 0, 'T', 0);
   osd_p->wr_char(68, 0, 'A', 0);
   osd_p->wr_char(69, 0, 'N', 0);
   osd_p->wr_char(70, 0, 'C', 0);
   osd_p->wr_char(71, 0, 'E', 0);
   osd_p->wr_char(72, 0, ':', 0);
   osd_p->wr_char(73, 0, ' ', 0);
   osd_p->wr_char(77, 0, 'i', 0);
   osd_p->wr_char(78, 0, 'n', 0);

   osd_p->wr_char(71, 1, 'S', 0);
   osd_p->wr_char(72, 1, 'P', 0);
   osd_p->wr_char(73, 1, 'E', 0);
   osd_p->wr_char(74, 1, 'E', 0);
   osd_p->wr_char(75, 1, 'D', 0);
   osd_p->wr_char(76, 1, ':', 0);
   osd_p->wr_char(77, 1, ' ', 0);
}



void scanning_check(I2cCore *compass_p, SpiCore *accelerometer_p, OsdCore *osd_p, ServoPwmCore *pwm_servo_p, SsegCore *sseg_p) {

   sseg_p->set_dp(0x00); // turn off all decimal points

   sseg_p->write_1ptn(0xff, 2); // turn off the 3rd digit
   sseg_p->write_1ptn(0xff, 4); // turn off the 5th digit position

   if(past_operation_mode != operation_mode) {
      angle = 0; // reset angle to 0 when mode changes to prevent angle overflow issues
   }

   if (operation_mode == 1) {
      sseg_p->write_1ptn(0xff, 1); // turn off the 2nd digit
      sseg_p->write_1ptn(0xf9, 3); // display 1 on the 4th seven segment display to indicate 360-degree mode
      servo_check(pwm_servo_p, osd_p, sseg_p); // perform the servo scanning for 180-degree swivel detection mode
   } else if(operation_mode == MANUAL_MODE) {
      sseg_p->write_1ptn(0xa4, 3); // display 2 on the 4th seven segment display to indicate manual mode
      osd_p->wr_char(77, 1, 'N', 0); // clear the speed unit display on the OSD since speed setting is disabled for manual mode
      osd_p->wr_char(78, 1, 'A', 0); // clear the speed unit display on the OSD since speed setting is disabled for manual mode
      pwm_servo_p->set_duty(0.0727, 0); // set servo to 90 degree position for manual mode by default
      compass_check(compass_p, accelerometer_p, sseg_p);
   } else {
      sseg_p->write_1ptn(0xff, 1); // turn off the 2nd digit
      sseg_p->write_1ptn(0xc0, 3); // display 0 on the 4th seven segment display to indicate 180-degree mode
      servo_check(pwm_servo_p, osd_p, sseg_p); // perform the servo scanning for 180-degree swivel detection mode
   }
   
}


void servo_check(ServoPwmCore *pwm_servo_p, OsdCore *osd_p, SsegCore *sseg_p) {
   uint8_t step = servo_speed; // step size for changing servo angle
   static bool reverse = false;
   double duty_cycle;

   int servo_max_angle = (operation_mode == 0) ? 180 : 360;

   sseg_p->write_1ptn(sseg_p->h2s(step), 0); // write servo speed to the first seven segment display for visual feedback

   osd_p->wr_char(77, 1, ' ', 0); // display the "S" character on the OSD to indicate the unit for distance (inches)
   osd_p->wr_char(78, 1, step + 0x30, 0); // display the "S" character on the OSD to indicate the unit for distance (inches)

   // Apply increment/decrement first
   if(reverse == false) {
      angle = angle + step;
   } else {
      angle = angle - step;
   }

   // Then check boundaries
   if (angle >= servo_max_angle) {
      angle = servo_max_angle;
      reverse = true;
   } else if (angle <= 0) {
      angle = 0;
      reverse = false;
   }

   duty_cycle = 0.025 + ((double)angle / (double)servo_max_angle) * 0.0975; // map angle to duty cycle (2.5% to 5.5%)

   pwm_servo_p->set_duty(duty_cycle, 0); // set duty cycle for servo channel (assuming channel 0 is used for the servo)

}


void compass_check(I2cCore *compass_p, SpiCore *accelerometer_p, SsegCore *sseg_p) {
   uint8_t wbytes[6], rbytes[6];
   //int ack;
   int16_t xc[MEAS_SAMPLES], yc[MEAS_SAMPLES], zc[MEAS_SAMPLES], x_sum = 0, y_sum = 0, z_sum = 0;
   double ax_sum = 0, ay_sum = 0, az_sum = 0, ax_avg, ay_avg, az_avg, ax, ay, az, pitch_den, roll_den, x_avg, y_avg, z_avg, pitch, roll, 
      x_corr, y_corr, z_corr, x, y, z, xh, yh;
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

   angle = atan2(y, x) * 180.0 / M_PI; // if you want to calculate heading without tilt compensation, use this formula instead

    // Add an offset angle to align the compass heading with the FPGA board facing forward
    // Note1: depending on where you place the compass sensor on the FPGA board, you may need to adjust the offset angle to make sure the compass heading is aligned with the direction the FPGA board is facing
    // Ex: mine is place on the right side of the board at JB Pmod header and the compass point's toward the boundary between W and SW by default.
    // Note2: similar to that of a iPhone compass where the direction you get is based on where the front of the phone is facing
    // i.e. the direction you get from compass is based on where the front of the FPGA board is facing 
   
    angle = angle + 88.0 - 180.0;
   
   if (angle < 0) {
      angle += 360.0; // convert to 0-360 degree range
   }

   if (angle > 360.0) {
      angle -= 360.0; // wrap around to 0-360 degree range
   }

   const char *direction;
   if ((angle > 337.25) || (angle < 22.5)) {
      direction = "N";
   } else if (angle >= 292.5 && angle <= 337.25) {
      direction = "NW";
   } else if (angle >= 247.5 && angle < 292.5) {
      direction = "W";
   } else if (angle >= 202.5 && angle < 247.5) {
      direction = "SW";
   } else if (angle >= 157.5 && angle < 202.5) {
      direction = "S";
   } else if (angle >= 112.5 && angle < 157.5) {
      direction = "SE";
   } else if (angle >= 67.5 && angle < 112.5) {
      direction = "E";
   } else {
      direction = "NE";
   }

   // display the heading in degree and direction on numerous interfaces
   compass_sseg(sseg_p, direction); 

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

void compass_sseg(SsegCore *sseg_p, const char* direction) {
   /* DIRECTION DISPLAY */

   // Conditional statement to match the direction with corresponding display patterns
   if (strcmp(direction, "N") == 0) {
      sseg_p->write_1ptn(0xab, 0); // display "N" at the rightmost position of the seven segment display
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display

   } else if (strcmp(direction, "NE") == 0) {
      sseg_p->write_1ptn(0xab, 1); // display "N" at the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0x86, 0); // display "E" at the rightmost position of the seven segment display

   } else if (strcmp(direction, "E") == 0) {
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0x86, 0); // display "E" at the rightmost position of the seven segment display

   } else if (strcmp(direction, "SE") == 0) {
      sseg_p->write_1ptn(0x92, 1); // display "S" at the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0x86, 0); // display "E" at the rightmost position of the seven segment display

   } else if (strcmp(direction, "S") == 0) {
      sseg_p->write_1ptn(0x92, 0); // display "S" at the rightmost position of the seven segment display
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display

   } else if (strcmp(direction, "SW") == 0) {
      sseg_p->write_1ptn(0x92, 1); // display "S" at the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0xd5, 0); // display "W" at the rightmost position of the seven segment display

   } else if (strcmp(direction, "W") == 0) {
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0xd5, 0); // display "W" at the rightmost position of the seven segment display

   } else if (strcmp(direction, "NW") == 0) {
      sseg_p->write_1ptn(0xab, 1); // display "N" at the second rightmost position of the seven segment display
      sseg_p->write_1ptn(0xd5, 0); // display "W" at the rightmost position of the seven segment display

   } else {
      sseg_p->write_1ptn(0xff, 0); // turn off the rightmost position of the seven segment display
      sseg_p->write_1ptn(0xff, 1); // turn off the second rightmost position of the seven segment display

   }
}



void ultrasonic_check(SensorUartCore *sensor_p, OsdCore *osd_p, PwmCore *pwm_p, SsegCore *sseg_p, RadarCore *radar_p) {
   uint8_t distance_sseg;

   distance = sensor_p->rx_distance();

   distance_sseg = (uint8_t)distance;

   if(distance > MAX_DISTANCE) {

      uart.disp("Object out of range of ");
      uart.disp(MAX_DISTANCE);
      uart.disp(" inch");
      uart.disp("\n\r");

      // set the duty cycle for blue registers on both leds
      pwm_p->set_duty(0.0, 0);
      pwm_p->set_duty(0.0, 3);

      // set the duty cycle for green registers on both leds
      pwm_p->set_duty(0.0, 1);
      pwm_p->set_duty(0.0, 4);

      // set the duty cycle for red registers on both leds
      pwm_p->set_duty(0.0, 2);
      pwm_p->set_duty(0.0, 5);
      
      // Display distance on OSD
      osd_p->wr_char(74, 0, ' ', 0); // hundreds digit
      osd_p->wr_char(75, 0, ' ', 0); // tens digit
      osd_p->wr_char(76, 0, ' ', 0); // ones digit
      
   } else {
      // set the duty cycle for blue registers on both leds
      pwm_p->set_duty(0.0, 0);
      pwm_p->set_duty(0.0, 3);

      // set the duty cycle for green registers on both leds
      pwm_p->set_duty(0.0, 1);
      pwm_p->set_duty(0.0, 4);

      // set the duty cycle for red registers on both leds
      pwm_p->set_duty(1.0, 2);
      pwm_p->set_duty(1.0, 5);

      uart.disp("Object Detected! Distance = ");
      uart.disp(distance);
      uart.disp(" in\n\r");

      // Display distance on OSD
      osd_p->wr_char(74, 0, (distance / 100) + 0x30, 0); // hundreds digit
      osd_p->wr_char(75, 0, ((distance / 10) % 10) + 0x30, 0); // tens digit
      osd_p->wr_char(76, 0, (distance % 10) + 0x30, 0); // ones digit

   }

   radar_p->update_scan(angle, distance); // update radar scan with the new angle and distance measurement

   // extract each digit and display on the seven segment display
   for (int i = 5; i < SSEG_DIST_NUM + 5; i++) {
      sseg_p->write_1ptn(sseg_p->h2s(distance_sseg % 10), i); // display the least significant digit at the rightmost position
      distance_sseg = distance_sseg / 10; // remove the least significant digit
   }
}


DispColors disp_color_map() {

   DispColors colors;

   if (disp_color == 'o' || disp_color == 'O') {
      colors.fill_color = 0x110; // default fill color to darker orange
      colors.outline_color = 0x1e0; // default outline color to slightly brighter orange for better visibility
      colors.osd_color = 0xfa0; // default osd color to orange
      colors.sweep_color = 0xfa0; // default sweep color to bright green for good contrast with orange   
   } else if (disp_color == 'y' || disp_color == 'Y') {
      colors.fill_color = 0x010; // default fill color to darker yellow
      colors.outline_color = 0x1b0; // default outline color to slightly brighter yellow for better visibility
      colors.osd_color = 0xff0; // default osd color to yellow
      colors.sweep_color = 0xff0; // default sweep color to yellow
   } else if (disp_color == 'b' || disp_color == 'B') {
      colors.fill_color = 0x004; // default fill color to darker blue
      colors.outline_color = 0x037; // default outline color to slightly brighter blue for better visibility
      colors.osd_color = 0x0ff; // default osd color to blue
      colors.sweep_color = 0x0ff; // default sweep color to blue
   } else if (disp_color == 'p' || disp_color == 'P') {
      colors.fill_color = 0x109; // default fill color to darker pink
      colors.outline_color = 0x1e5; // default outline color to slightly brighter pink for better visibility
      colors.osd_color = 0xf8b; // default osd color to pink
      colors.sweep_color = 0xf8b; // default sweep color to pink
   } else {
      colors.fill_color = 0x010; // default fill color to darker green
      colors.outline_color = 0x020; // default outline color to slightly brighter green for better visibility
      colors.osd_color = 0x0f0; // default osd color to green
      colors.sweep_color = 0x0f0; // default sweep color to bright green for good contrast with darker green
   }

   return colors;
}

// external core instantiation
GpoCore led(get_slot_addr(BRIDGE_BASE, S2_LED));
GpiCore sw(get_slot_addr(BRIDGE_BASE, S3_SW));
//USER user(get_slot_addr(BRIDGE_BASE, S4_USER));
//XadcCore adc(get_slot_addr(BRIDGE_BASE, S5_XDAC));
PwmCore pwm_rgb(get_slot_addr(BRIDGE_BASE, S6_PWM));
//DebounceCore btn(get_slot_addr(BRIDGE_BASE, S7_BTN));
SsegCore sseg(get_slot_addr(BRIDGE_BASE, S8_SSEG));
SpiCore accelerometer(get_slot_addr(BRIDGE_BASE, S9_SPI));
I2cCore compass(get_slot_addr(BRIDGE_BASE, S10_I2C));
Ps2Core ps2(get_slot_addr(BRIDGE_BASE, S11_PS2));
DdfsCore ddfs(get_slot_addr(BRIDGE_BASE, S12_DDFS));
AdsrCore adsr(get_slot_addr(BRIDGE_BASE, S13_ADSR), &ddfs);
SensorUartCore sensor_uart(get_slot_addr(BRIDGE_BASE, S14_UART2));
ServoPwmCore pwm_servo(get_slot_addr(BRIDGE_BASE, S15_PWM2));

FrameCore frame(FRAME_BASE);
//SweeperCore sweep(get_sprite_addr(BRIDGE_BASE, V7_BAR));
RadarCore radar(get_sprite_addr(BRIDGE_BASE, V7_RADAR));
GpvCore gray(get_sprite_addr(BRIDGE_BASE, V6_GRAY));
SpriteCore ghost(get_sprite_addr(BRIDGE_BASE, V3_GHOST), 1024);
OsdCore osd(get_sprite_addr(BRIDGE_BASE, V2_OSD));
SpriteCore mouse(get_sprite_addr(BRIDGE_BASE, V1_MOUSE), 1024);

int main() {
   frame.bypass(0);
   radar.bypass(0);
   osd.bypass(0);
   gray.bypass(1);
   ghost.bypass(1);
   mouse.bypass(1);

   radar.set_origin(RADAR_XCENTER, RADAR_YCENTER); // set radar origin to the center of the radar sprite
   radar.set_distance(distance); // set the initial distance reading from the ultrasonic sensor to the radar sprite
   radar.set_object_detected(distance);
   radar.set_angle(angle); // set the initial heading reading from the compass to the radar sprite
   radar.set_color(0x0f0); // set to green by default
   radar.set_mode(operation_mode); // set the radar mode based on the current operation mode (initially set to 180 swivel mode)
   radar.set_thickness(3,3,3); // set the thickness of the radar sweep line to 2 pixels for better visibility
   radar.set_fade(4,2,1); 

   radar_init(&frame, &osd, &radar); // initialize the radar display on the monitor with the initial settings
   pwm_servo.set_freq(50); // set servo PWM frequency to 50Hz
   int id = ps2.init();

   // write the inital heading and distance into the sweeping sprite cores, so it can be initialized and shown on monitor

   while (1) {

      keyboard_check(&ps2, &osd, &frame, &radar, id); // check for keyboard input to change settings 
      scanning_check(&compass, &accelerometer, &osd, &pwm_servo, &sseg); // check for which mode we are in and perform the corresponding scanning operation to get the heading if in scanning mode or perform compass check 
      ultrasonic_check(&sensor_uart, &osd, &pwm_rgb, &sseg, &radar); // check for new distance readings from the ultrasonic sensor and update accordingly

      past_operation_mode = operation_mode; // update past operation mode for the scanning check   

      sleep_ms(15);
   } 
} 
