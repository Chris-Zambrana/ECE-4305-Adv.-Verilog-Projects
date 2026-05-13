/*****************************************************************//**
 * @file timer_core.cpp
 *
 * @brief implementation of various video core classes
 *
 * @author p chu
 * @version v1.0: initial release
 ********************************************************************/

#include "vga_core.h"
#include <cmath>

/**********************************************************************
 * General purpose video core methods
 *********************************************************************/
GpvCore::GpvCore(uint32_t core_base_addr) {
   base_addr = core_base_addr;
}
GpvCore::~GpvCore() {
}

void GpvCore::wr_mem(int addr, uint32_t data) {
   io_write(base_addr, addr, data);
}

void GpvCore::bypass(int by) {
   io_write(base_addr, BYPASS_REG, (uint32_t ) by);
}

/**********************************************************************
 * Sprite core methods
 *********************************************************************/
SpriteCore::SpriteCore(uint32_t core_base_addr, int sprite_size) {
   base_addr = core_base_addr;
   size = sprite_size;
}
SpriteCore::~SpriteCore() {
}

void SpriteCore::wr_mem(int addr, uint32_t color) {
   io_write(base_addr, addr, color);
}

void SpriteCore::bypass(int by) {
   io_write(base_addr, BYPASS_REG, (uint32_t ) by);
}

void SpriteCore::move_xy(int x, int y) {
   io_write(base_addr, X_REG, x);
   io_write(base_addr, Y_REG, y);
   return;
}

void SpriteCore::wr_ctrl(int32_t cmd) {
   io_write(base_addr, SPRITE_CTRL_REG, cmd);
}




/**********************************************************************
 * Radar Overlay Core methods
 *********************************************************************/
RadarCore::RadarCore(uint32_t core_base_addr) {
   base_addr = core_base_addr;
}

RadarCore::~RadarCore() {
}

void RadarCore::wr_mem(int addr, uint32_t data) {
   io_write(base_addr, addr, data);
}

void RadarCore::bypass(int by) {
   io_write(base_addr, BYPASS_REG, (uint32_t)(by & 0x01));
}

void RadarCore::set_origin(int x, int y) {
   io_write(base_addr, X0_REG, (uint32_t)(x & 0x07ff));
   io_write(base_addr, Y0_REG, (uint32_t)(y & 0x07ff));
}

void RadarCore::set_distance(int dist) {
   io_write(base_addr, DISTANCE_REG, (uint32_t)(dist & 0x00ff));
}

void RadarCore::set_angle(int deg) {
   io_write(base_addr, ANGLE_REG, (uint32_t)(deg & 0x01ff));
}

void RadarCore::set_color(uint32_t rgb) {
   io_write(base_addr, COLOR_REG, rgb & 0x0fff);
}

void RadarCore::set_mode(int mode) {
   io_write(base_addr, MODE_REG, (uint32_t)(mode & 0x07));
}

void RadarCore::set_object_detected(int distance) {
   int detected;

   if (distance >= 0 && distance <= MAX_DISTANCE)
      detected = 1;
   else
      detected = 0;

   io_write(base_addr, OBJECT_DETECTED_REG, (uint32_t)(detected & 0x01));
}

void RadarCore::set_thickness(int sweep_thick, int object_thick,
                              int fade_thick) {
   io_write(base_addr, THICKNESS_REG,
            pack_thickness(sweep_thick, object_thick, fade_thick));
}

void RadarCore::set_fade(int levels, int step, int enable) {
   io_write(base_addr, FADE_REG, pack_fade(levels, step, enable));
}
/*
void RadarCore::init_defaults() {
   bypass(0);
   set_origin(DEFAULT_X0, DEFAULT_Y0);
   set_distance(MAX_DISTANCE);
   set_angle(0);
   set_color(0x0f0);
   set_mode(MODE_180);
   set_object_detected(0);
   set_thickness(3, 5, 3);
   set_fade(4, 2, 1);
}
*/
void RadarCore::update_scan(int deg, int dist) {
   set_angle(deg);
   set_distance(dist);
   set_object_detected(dist);
}

uint32_t RadarCore::pack_thickness(int sweep_thick, int object_thick,
                                   int fade_thick) {
   uint32_t data;

   data = ((uint32_t)(sweep_thick  & 0x0f) << 0)  |
          ((uint32_t)(object_thick & 0x0f) << 4)  |
          ((uint32_t)(fade_thick   & 0x0f) << 8);
   return data;
}

uint32_t RadarCore::pack_fade(int levels, int step, int enable) {
   uint32_t data;

   data = ((uint32_t)(levels & 0x0f) << 0) |
          ((uint32_t)(step   & 0x0f) << 4) |
          ((uint32_t)(enable & 0x01) << 8);
   return data;
}

/**********************************************************************
 * OSD core methods
 *********************************************************************/
OsdCore::OsdCore(uint32_t core_base_addr) {
   base_addr = core_base_addr;
   set_color(0x0f0, CHROMA_KEY_COLOR);  // green on black
}
OsdCore::~OsdCore() {
}
// not used

void OsdCore::set_color(uint32_t fg_color, uint32_t bg_color) {
   io_write(base_addr, FG_CLR_REG, fg_color);
   io_write(base_addr, BG_CLR_REG, bg_color);
}

void OsdCore::wr_char(uint8_t x, uint8_t y, char ch, int reverse) {
   uint32_t ch_offset;
   uint32_t data;

   ch_offset = (y << 7) + (x & 0x07f);   // offset is concatenation of y and x
   if (reverse == 1)
      data = (uint32_t)(ch | 0x80);
   else
      data = (uint32_t) ch;
   io_write(base_addr, ch_offset, data);
   return;
}

void OsdCore::clr_screen() {
   int x, y;

   for (x = 0; x < CHAR_X_MAX; x++)
      for (y = 0; y < CHAR_Y_MAX; y++) {
         wr_char(x, y, NULL_CHAR);
      }
   return;
}

void OsdCore::bypass(int by) {
   io_write(base_addr, BYPASS_REG, (uint32_t ) by);
}

/**********************************************************************
 * FrameCore core methods
 *********************************************************************/
FrameCore::FrameCore(uint32_t frame_base_addr) {
   base_addr = frame_base_addr;
}
FrameCore::~FrameCore() {
}
// not used

void FrameCore::wr_pix(int x, int y, int color) {
   uint32_t pix_offset;

   pix_offset = HMAX * y + x;
   io_write(base_addr, pix_offset, color);
   return;
}

void FrameCore::clr_screen(int color) {
   int x, y;

   for (x = 0; x < HMAX; x++)
      for (y = 0; y < VMAX; y++) {
         wr_pix(x, y, color);
      }
   return;
}

void FrameCore::bypass(int by) {
   io_write(base_addr, BYPASS_REG, (uint32_t ) by);
}
// from AdaFruit
void FrameCore::plot_line(int x0, int y0, int x1, int y1, int color) {
   int dx, dy;
   int err, ystep, steep;

   if (x0 > x1) {
      swap(x0, x1);
      swap(y0, y1);
   }
   // slope is high
   steep = (abs(y1 - y0) > abs(x1 - x0)) ? 1 : 0;
   if (steep) {
      swap(x0, y0);
      swap(x1, y1);
   }
   // Ensure x0 <= x1 after steep swap
   if (x0 > x1) {
      swap(x0, x1);
      swap(y0, y1);
   }
   dx = x1 - x0;
   dy = abs(y1 - y0);
   err = dx / 2;
   if (y0 < y1) {
      ystep = 1;
   } else {
      ystep = -1;
   }
   for (; x0 <= x1; x0++) {
      if (steep) {
         wr_pix(y0, x0, color);
      } else {
         wr_pix(x0, y0, color);
      }
      err = err - dy;
      if (err < 0) {
         y0 = y0 + ystep;
         err = err + dx;
      }
   }
}

void FrameCore::plot_circle(int x0, int y0, int r, int color) {
   int f = 1 - r;
   int ddF_x = 1;
   int ddF_y = -2 * r;
   int x = 0;
   int y = r;

   // Plot cardinal points
   wr_pix(x0, y0 + r, color);
   wr_pix(x0, y0 - r, color);
   wr_pix(x0 + r, y0, color);
   wr_pix(x0 - r, y0, color);

   // Use Midpoint circle algorithm to plot remaining points
   while (x < y) {
      if (f >= 0) {
         y--;
         ddF_y += 2;
         f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      // Plot all 8 octants
      wr_pix(x0 + x, y0 + y, color);
      wr_pix(x0 - x, y0 + y, color);
      wr_pix(x0 + x, y0 - y, color);
      wr_pix(x0 - x, y0 - y, color);
      wr_pix(x0 + y, y0 + x, color);
      wr_pix(x0 - y, y0 + x, color);
      wr_pix(x0 + y, y0 - x, color);
      wr_pix(x0 - y, y0 - x, color);
   }
}

void FrameCore::fill_circle(int x0, int y0, int r, int color) {
   int16_t f = 1 - r;
   int16_t ddF_x = 1;
   int16_t ddF_y = -2 * r;
   int16_t x = 0;
   int16_t y = r;

   // Draw vertical line through center
   for (int i = y0 - r; i <= y0 + r; i++) {
      wr_pix(x0, i, color);
   }

   // Use Midpoint circle algorithm to fill remaining areas
   while (x < y) {
      if (f >= 0) {
         y--;
         ddF_y += 2;
         f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      // Draw vertical lines at each position (fills the circle)
      // Upper and lower halves
      for (int i = y0 - y; i <= y0 + y; i++) {
         wr_pix(x0 + x, i, color);
         wr_pix(x0 - x, i, color);
      }

      // Left and right sections (avoid double-drawing)
      if (x != y) {
         for (int i = y0 - x; i <= y0 + x; i++) {
            wr_pix(x0 + y, i, color);
            wr_pix(x0 - y, i, color);
         }
      }
   }
}

void FrameCore::plot_half_circle(int x0, int y0, int r, int color) {
   int16_t f = 1 - r;
   int16_t ddF_x = 1;
   int16_t ddF_y = -2 * r;
   int16_t x = 0;
   int16_t y = r;

   // Plot cardinal points (only top half for 180-degree radar)
   wr_pix(x0, y0 - r, color);  // top
   wr_pix(x0 + r, y0, color);  // right
   wr_pix(x0 - r, y0, color);  // left

   // Use Midpoint circle algorithm but only plot top half
   while (x < y) {
      if (f >= 0) {
         y--;
         ddF_y += 2;
         f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      // Plot only top half (y < center)
      wr_pix(x0 + x, y0 - y, color);  // top-right
      wr_pix(x0 - x, y0 - y, color);  // top-left
      wr_pix(x0 + y, y0 - x, color);  // top-right (rotated)
      wr_pix(x0 - y, y0 - x, color);  // top-left (rotated)
   }
}

void FrameCore::fill_half_circle(int x0, int y0, int r, int color) {
   // Optimized Midpoint circle algorithm for top-half fill
   // Uses horizontal line sweeps like Adafruit's fillCircleHelper
   
   int16_t f = 1 - r;
   int16_t ddF_x = 1;
   int16_t ddF_y = -2 * r;
   int16_t x = 0;
   int16_t y = r;
   int16_t px = x;
   int16_t py = y;

   // Draw center vertical line (from center up to top)
   for (int yy = y0; yy >= y0 - r; yy--) {
      wr_pix(x0, yy, color);
   }

   while (x < y) {
      if (f >= 0) {
         y--;
         ddF_y += 2;
         f += ddF_y;
      }
      x++;
      ddF_x += 2;
      f += ddF_x;

      // Draw horizontal lines for top half
      // For each octant transition, draw the corresponding horizontal sweep
      if (x < (y + 1)) {
         // Draw right side horizontal lines
         for (int yy = y0; yy >= y0 - y; yy--) {
            wr_pix(x0 + x, yy, color);
         }
         // Draw left side horizontal lines (mirrored)
         for (int yy = y0; yy >= y0 - y; yy--) {
            wr_pix(x0 - x, yy, color);
         }
      }

      // Avoid double-drawing lines
      if (y != py) {
         // Draw right side at py position
         for (int yy = y0; yy >= y0 - px; yy--) {
            wr_pix(x0 + py, yy, color);
         }
         // Draw left side at py position
         for (int yy = y0; yy >= y0 - px; yy--) {
            wr_pix(x0 - py, yy, color);
         }
         py = y;
      }
      px = x;
   }
}

void FrameCore::swap(int &a, int &b) {
   int tmp;

   tmp = a;
   a = b;
   b = tmp;
}

