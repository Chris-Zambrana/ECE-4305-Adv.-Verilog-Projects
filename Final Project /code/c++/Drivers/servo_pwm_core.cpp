
#include "gpio_cores.h"

#include "servo_pwm_core.h"

/**********************************************************************
 * ServoPwmCore
 **********************************************************************/
ServoPwmCore::ServoPwmCore(uint32_t core_base_addr) {
   base_addr = core_base_addr;
   set_freq(50);
}

ServoPwmCore::~ServoPwmCore() {
}

void ServoPwmCore::set_freq(int freq) {
   uint32_t dvsr;
   dvsr = (uint32_t) SYS_CLK_FREQ * 1000000 / MAX / freq;
   io_write(base_addr, DVSR_REG, dvsr);
}

void ServoPwmCore::set_duty(int duty, int channel) {
   uint32_t d;

   if (duty > MAX) {
      d = MAX;
   } else {
      d = duty;
   }
   io_write(base_addr, DUTY_REG_BASE + channel, d);
}

void ServoPwmCore::set_duty(double f, int channel) {
   int duty;
   duty = (int) (f * MAX);
   debug("set_duty_f: ", f, duty);
   set_duty(duty, channel);
}