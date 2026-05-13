/*****************************************************************//**
 * @file vga_core.h
 *
 * @brief contain classes of video cores
 *
 *
 * @author p chu
 * @version v1.0: initial release
 ********************************************************************/
#ifndef _VGA_H_INCLUDED
#define _VGA_H_INCLUDED

#include "chu_init.h"
#include <stdlib.h>

/**********************************************************************
 * General-purpose video core
 *********************************************************************/
/**
 *  General-purpose video core driver
 *
 */
class GpvCore {
public:
   /**
    * register map
    *
    */
   enum {
      BYPASS_REG = 0x2000  /**< bypass control register */
   };
   /* methods */
   GpvCore(uint32_t core_base_addr);
   ~GpvCore();                  // not used

   /**
    * write a 32-bit word to memory module/registers of a video core
    * @param addr offset address within core
    * @param color data to be written
    *
    */
   void wr_mem(int addr, uint32_t color);

   /**
    * enable/disable core bypass
    * @param by 1: bypass current core; 0: not bypass
    *
    */
   void bypass(int by);

private:
   uint32_t base_addr;
};

/**********************************************************************
 * Sprite Core
 *********************************************************************/
/**
 * sprite video core driver
 *
 * video subsystem HDL parameter:
 *  - CHROMA_KEY (KEY_COLOR) = 0
 *
 */
class SpriteCore {
public:
   /**
    * register map
    *
    */
   enum {
      BYPASS_REG = 0x2000,     /**< bypass control register */
      X_REG = 0x2001,          /**< x-axis of sprite origin */
      Y_REG = 0x2002,          /**< y-axis of sprite origin */
      SPRITE_CTRL_REG = 0x2003 /**< sprite control register */
   };
   /**
    * symbolic constants
    *
    */
   enum {
      KEY_COLOR = 0,  /**< chroma-key color */
   };
   /* methods */
   SpriteCore(uint32_t core_base_addr, int size);
   ~SpriteCore();                  // not used

   /**
    * write a 32-bit word to memory module/registers of a video core
    * @param addr offset address within core
    * @param color data to be written
    *
    */
   void wr_mem(int addr, uint32_t color);

   /**
    * move sprite to a location
    * @param x x-coordinate of sprite origin
    * @param y y-coordinate of sprite origin
    *
    * @note origin is the top-left corner of sprite
    */
   void move_xy(int x, int y);

   /**
    * write sprite control command
    * @param cmd control command
    *
    */
   void wr_ctrl(int32_t cmd);

   /**
    * enable/disable core bypass
    * @param by 1: bypass current core; 0: not bypass
    * @note type of command depends on each individual sprite core
    */
   void bypass(int by);

private:
   uint32_t base_addr;
   int size;   // sprite memory size
};



/**********************************************************************
 * Radar Overlay Core
 *********************************************************************/
/**
 * Procedural radar overlay core driver
 *
 * This core is intended to sit in the VGA pixel stream after the
 * frame-buffer radar background. It does not draw into frame memory.
 * Instead, it writes control registers used by RTL to overlay:
 *  - active sweep line
 *  - fading sweep trail
 *  - fixed-red object-detection beam
 */
class RadarCore {
public:
   /**
    * register map
    */
   enum {
      BYPASS_REG          = 0x2000, /**< bypass control register */
      X0_REG              = 0x2001, /**< radar origin x-coordinate */
      Y0_REG              = 0x2002, /**< radar origin y-coordinate */
      DISTANCE_REG        = 0x2003, /**< ultrasonic distance in inches */
      ANGLE_REG           = 0x2004, /**< current sweep/heading angle */
      COLOR_REG           = 0x2005, /**< sweep/fade RGB444 color */
      MODE_REG            = 0x2006, /**< radar display/sweep mode */
      OBJECT_DETECTED_REG = 0x2007, /**< object-detected flag */
      THICKNESS_REG       = 0x2008, /**< sweep/object/fade thickness config */
      FADE_REG            = 0x2009  /**< fade level/spacing/enable config */
   };

   /**
    * symbolic constants
    */
   enum {
      MODE_180       = 0,       /**< 180-degree semicircle radar */
      MODE_360       = 1,       /**< 360-degree circular radar */
      MODE_MANUAL    = 2,       /**< manual/compass heading mode */
      DEFAULT_X0     = 320,
      DEFAULT_Y0     = 240,
      DEFAULT_RADIUS = 200,
      MIN_DISTANCE   = 6,
      MAX_DISTANCE   = 200,
      OBJECT_RED     = 0xf00    /**< fixed object color used in RTL */
   };

   /* methods */
   RadarCore(uint32_t core_base_addr);
   ~RadarCore();                  // not used

   /**
    * write a 32-bit word to memory module/registers of radar core
    * @param addr offset address within core
    * @param data data to be written
    */
   void wr_mem(int addr, uint32_t data);

   /**
    * enable/disable radar overlay bypass
    * @param by 1: bypass overlay; 0: enable overlay
    */
   void bypass(int by);

   /**
    * set radar origin coordinate
    * @param x x-coordinate of radar origin/center
    * @param y y-coordinate of radar origin/center
    */
   void set_origin(int x, int y);

   /**
    * set ultrasonic distance register only
    * @param dist distance in inches; hardware uses 8 bits
    */
   void set_distance(int dist);

   /**
    * set current sweep/heading angle
    * @param deg angle in degrees, expected 0 through 360
    */
   void set_angle(int deg);

   /**
    * set sweep/fade color
    * @param rgb 12-bit RGB444 color
    */
   void set_color(uint32_t rgb);

   /**
    * set radar mode
    * @param mode MODE_180, MODE_360, MODE_MANUAL, or project-defined value
    */
   void set_mode(int mode);

   /**
    * set object-detected flag
    * @param detected 1: draw object beam; 0: no object beam
    */
   void set_object_detected(int distance);

   /**
    * pack and write sweep/object/fade thickness config
    * @param sweep_thick active sweep-line thickness, 4-bit field
    * @param object_thick red object-beam thickness, 4-bit field
    * @param fade_thick fade-trail beam thickness, 4-bit field
    */
   void set_thickness(int sweep_thick, int object_thick, int fade_thick);

   /**
    * pack and write fade config
    * @param levels number of active fade levels, 4-bit field
    * @param step spacing between fade samples, 4-bit field
    * @param enable 1: enable fading trail; 0: disable fading trail
    */
   void set_fade(int levels, int step, int enable);

   /**
    * write commonly used defaults for the radar overlay core
    * 
    */

   //void init_defaults();

   /*
    * update the live scan registers that normally change in the main loop
    * @param deg current angle in degrees
    * @param dist current distance in inches
    * @param mode current radar mode
    * @param rgb current sweep/fade RGB444 color
    */

   void update_scan(int deg, int dist);

   /**
    * helper to pack thickness register value
    */
   static uint32_t pack_thickness(int sweep_thick, int object_thick,
                                  int fade_thick);

   /**
    * helper to pack fade register value
    */
   static uint32_t pack_fade(int levels, int step, int enable);

private:
   uint32_t base_addr;
};

/**********************************************************************
 * OSD Core
 *********************************************************************/
/**
 * osd (on-screen display) video core driver
 *
 * video subsystem HDL parameter:
 *  - CHROMA_KEY (CHROMA_KEY_COLOR) = 0
 *
 */
class OsdCore {
public:
   /**
    * register map
    *
    */
   enum {
      BYPASS_REG = 0x2000,  /**< bypass control register */
      FG_CLR_REG = 0x2001,  /**< foreground color register */
      BG_CLR_REG = 0x2002   /**< background color register */
   };
   /**
    * symbolic constants
    *
    */
   enum {
      CHROMA_KEY_COLOR = 0,   // chroma key
      NULL_CHAR = 0x00,       // signature for transparent char tile
      CHAR_X_MAX = 80,        // 80 char per row
      CHAR_Y_MAX = 30         // 30 char per column
   };
   /* methods */
   OsdCore(uint32_t core_base_addr);
   ~OsdCore();
   // not used

   /**
    * set foreground/background text display colors
    * @param fg_color foreground text display color
    * @param bg_color background text display color
    *
    */
   void set_color(uint32_t fg_color, uint32_t bg_color);

   /**
    * write a char to tile RAM
    * @param x x-coordinate of the tile (between 0 and CHAR_X_MAX)
    * @param y y-coordinate of the tile (between 0 and CHAR_Y_MAX)
    * @param ch char to be written
    * @param reverse 0: normal display; 1: reversed display
    *
    * @note reversed display swaps the foreground/background colors
    *
    */
   void wr_char(uint8_t x, uint8_t y, char ch, int reverse = 0);

   /**
    * clear tile RAM (by writing NULL_CHAR to all tiles)
    *
    */
   void clr_screen();

   /**
    * enable/disable core bypass
    * @param by 1: bypass current core; 0: not bypass
    *
    */
   void bypass(int by);
private:
   uint32_t base_addr;
};

/**********************************************************************
 * Frame Core
 *********************************************************************/
/**
 * frame buffer core driver
 *
 */
class FrameCore {
public:
   /**
    * Register map
    *
    */
   enum {
      BYPASS_REG = 0xfffff  /**< bypass control register */
   };
   /**
    * Symbolic constants for frame buffer size
    *
    */
   enum {
    HMAX = 640,  /**< 640 pixels per row */
    VMAX = 480   /**< 480 pixels per row */
   };
   /* methods */
   FrameCore(uint32_t frame_base_addr);
   ~FrameCore();                  // not used

   /**
    * write a pixel to frame buffer
    * @param x x-coordinate of the pixel (between 0 and HMAX)
    * @param y y-coordinate of the pixel (between 0 and VMAX)
    * @param color pixel color
    *
    */
   void wr_pix(int x, int y, int color);

   /**
    * clear frame buffer (fill the frame with a specific color)
    * @param color color to fill the frame
    *
    */
   void clr_screen(int color);


   /**
    * generate pixels for a line in frame buffer (plot a line)
    * @param x1 x-coordinate of starting point
    * @param y1 y-coordinate of starting point
    * @param x2 x-coordinate of ending point
    * @param y2 y-coordinate of ending point
    * @param color line color
    *
    */
   void plot_line(int x1, int y1, int x2, int y2, int color);

   /**
    * generate pixels for a circle in frame buffer (plot a circle)
    * @param x0 x-coordinate of circle center
    * @param y0 y-coordinate of circle center
    * @param r radius of the circle
    * @param color circle color
    *
    */
   void plot_circle(int x0, int y0, int r, int color);

   /**
    * generate pixels for a filled circle in frame buffer (fill a circle)
    * @param x0 x-coordinate of circle center
    * @param y0 y-coordinate of circle center
    * @param r radius of the circle
    * @param color circle color
    *
    */
   void fill_circle(int x0, int y0, int r, int color);

   /**
    * generate pixels for a circle in frame buffer (plot a circle)
    * @param x0 x-coordinate of circle center
    * @param y0 y-coordinate of circle center
    * @param r radius of the circle
    * @param color circle color
    *
    */
   void plot_half_circle(int x0, int y0, int r, int color);

   /**
    * generate pixels for a filled half circle in frame buffer (fill a half circle)
    * @param x0 x-coordinate of circle center
    * @param y0 y-coordinate of circle center
    * @param r radius of the circle
    * @param color circle color
    *
    */
   void fill_half_circle(int x0, int y0, int r, int color);

   /**
    * enable/disable core bypass
    * @param by 1: bypass current core; 0: not bypass
    *
    */
   void bypass(int by);


private:
   uint32_t base_addr;
   void swap(int &a, int &b);
};

#endif  // _VGA_H_INCLUDED
