#ifndef _SPEED_CALC_INCLUDED
#define _SPEED_CALC_INCLUDED

#include "chu_init.h"
#include "gpio_cores.h"

#define SPEED_SW_MASK 0xFF3E // mask for bits 1-5, since we have 5 switches

class SpeedMap {

public:
    SpeedMap(int x1, int x2, int y1, int y2);
    ~SpeedMap();

    int speed_calc(int current_lvl);

    void uart_speed_check(int speed);

private: 
    int min_speed; // 3150 ms
    int max_speed; // 50 ms
    int min_lvl; // 0
    int max_lvl; // 31 (2^5 - 1, since we have 5 switches)
    int lvl;
    int speed;
};

#endif  // _SPEED_CALC_INCLUDED
