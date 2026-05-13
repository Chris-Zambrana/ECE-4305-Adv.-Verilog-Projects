#ifndef _SERVO_PWM_CORE_H_INCLUDED
#define _SERVO_PWM_CORE_H_INCLUDED

#include "chu_init.h"

class ServoPwmCore {
public:
   /**
    * register map
    *
    */
   enum {
      DVSR_REG = 0,         /**< pwm divisor register */
      DUTY_REG_BASE = 0x10  /**< channel 0 duty cycle register */
   };
   /**
    * symbolic constant
    *
    */
   enum {
      RESOLUTION_BITS = 21, /**< # resolution bits defined in HDL */
      MAX = 1 << RESOLUTION_BITS /**< # max levels in duty cycle (= 2^ESOLUTION_BITS; 100% duty cycle) */
   };
   /**
    * constructor.
    * @note default pwm frequency is set to 1K Hz
    * @note all pwm channels have the same frequency
    *
    */
   ServoPwmCore(uint32_t core_base_addr);
   ~ServoPwmCore();

   /* methods */
   /**
    * set pwm switching frequency
    *
    * @param freq pwm switching frequency
    *
    */
   void set_freq(int freq);

   /**
    * set duty cycle in unsigned format (between 0 and MAX)
    *
    * @param duty duty cycle (between 0 and MAX)
    * @param channel pwm channel number
    *
    */
   void set_duty(int duty, int channel);

   /**
    * set duty cycle in real format (between 0.0 and 1.0)
    *
    * @param f duty cycle % (between 0.0 and 1.0)
    * @param channel pwm channel number
    *
    */
   void set_duty(double f, int channel);

private:
   uint32_t base_addr;
   uint32_t freq;
};

#endif