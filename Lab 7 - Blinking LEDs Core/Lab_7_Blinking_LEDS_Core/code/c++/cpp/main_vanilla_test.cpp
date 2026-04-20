/*****************************************************************//**
 * @file main_vanilla_test.cpp
 *
 * @brief Basic test of 4 basic i/o cores
 *
 * @author p chu
 * @version v1.0: initial release
 *********************************************************************/

//#define _DEBUG
#include "chu_init.h"
#include "gpio_cores.h"

/**
 * blink once per second for 5 times.
 * provide a sanity check for timer (based on SYS_CLK_FREQ)
 * @param led_p pointer to led instance
 */
void blinking_led_write(BlinkingLedCore *led_p, int led_num, uint32_t rate) {
   led_p->write_rate(led_num, rate);
}

/*
int get_user_input(UartCore &uart) {
    char buf[10];
    int i = 0;
    int ch, buff_int;
   
    uart.disp("Enter blink rate (ms): ");

    while (1) {
        ch = uart.rx_byte();          // returns -1 if no byte available
        if (ch == -1) continue;       // keep polling

        // Enter pressed
        if (ch == '\r' || ch == '\n') 
            break;
            
        uart.disp((char)ch);          // echo typed character
        buf[i++] = (char)ch;          // store character

        if (i >= 9)                   // avoid overflow
            break;
    }

    buff_int = atoi(buf);

    if(buff_int > 65535) {
         uart.disp("Invalid input. Please enter a number between 0 and 65535.\r\n");
         return get_user_input(uart); // Recursively call to get valid input
      }

    buf[i] = '\0';                    // null-terminate
    uart.disp("\r\n");
    return buff_int;                 // convert "250" -> 250
}
*/

/*
 * uart transmits test line.
 * @note uart instance is declared as global variable in chu_io_basic.h

void uart_check(BlinkingLedCore *led_p) {
   uart.disp("LED Rates: ");
   uart.disp("\n\r");
   uart.disp("led0:");
   uart.disp(led_p->read_rate(0));
   uart.disp("\n\r");
   uart.disp("led1:");
   uart.disp(led_p->read_rate(1));
   uart.disp("\n\r");
   uart.disp("led2:");
   uart.disp(led_p->read_rate(2));
   uart.disp("\n\r");
   uart.disp("led3:");
   uart.disp(led_p->read_rate(3));
   uart.disp("\n\r");
}
*/

// instantiate switch, led
BlinkingLedCore blinking_led(get_slot_addr(BRIDGE_BASE, S4_USER));

int main() {

   while (1) {
      blinking_led_write(&blinking_led, 0, 500);
      blinking_led_write(&blinking_led, 1, 1000);
      blinking_led_write(&blinking_led, 2, 2000);
      blinking_led_write(&blinking_led, 3, 4000);
      //uart_check(&blinking_led);
   } //while
} //main

