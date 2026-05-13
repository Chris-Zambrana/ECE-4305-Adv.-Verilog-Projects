/*****************************************************************//**
 * @file uart_core.cpp
 *
 * @brief implementation of UartCore class
 *
 * @author p chu
 * @version v1.0: initial release
 ********************************************************************/

#include "sensor_uart_core.h"

SensorUartCore::SensorUartCore(uint32_t core_base_addr) {
   base_addr = core_base_addr;
   set_baud_rate(9600);      //default baud rate
}

SensorUartCore::~SensorUartCore() {
}

/* baud rate = sys_clk_freq/16/(dvsr+1) */
void SensorUartCore::set_baud_rate(int baud) {
   uint32_t dvsr;

   dvsr = SYS_CLK_FREQ*1000000 / 16 / baud - 1;
   io_write(base_addr, DVSR_REG, dvsr);
}

int SensorUartCore::rx_fifo_empty() {
   uint32_t rd_word;
   int empty;

   rd_word = io_read(base_addr, RD_DATA_REG);
   empty = (int) (rd_word & RX_EMPT_FIELD) >> 8;
   return (empty);
}

int SensorUartCore::rx_byte() {
   uint32_t data;

   if (rx_fifo_empty())
      return (-1);
   else {
      data = io_read(base_addr, RD_DATA_REG) & RX_DATA_FIELD;
      io_write(base_addr, RM_RD_DATA_REG, 0); //dummy write to remove data from rx FIFO
      return ((int) data);
   }
}

uint8_t SensorUartCore::rx_distance() {
   uint8_t data;
   uint8_t byte_count = 0;
   uint8_t packet[5];
   uint8_t distance;

   // Wait for complete 5-byte packet: 'R' + 3 values + CR
   while (byte_count < 5) {
      if(!rx_fifo_empty()) {
         data = rx_byte();

         if(byte_count == 0) {
         if (data == 'R') {
            // If the first byte is not 'R', discard it and continue waiting for the start of a new packet
            packet[byte_count++] = data; // store the first byte of the packet
         } 
         } else if (byte_count >=1 && byte_count <= 3) {
            if((data >= '0' && data <= '9')) { // check if the current byte is a valid ASCII digit
               packet[byte_count++] = data; // store the current byte of the packet
            } else if (data == 'R') { // if we receive another 'R' before completing the packet, treat it as the start of a new packet
               packet[0] = data; // store the first byte of the new packet
               byte_count = 1; // reset byte count to wait for the start of a new packet
            } else {
               byte_count = 0; // reset byte count to wait for the start of a new packet
            }
         } else {
            if (byte_count == 4) { // check if the last byte is a carriage return

               if (data == 0x0d) {
                  packet[byte_count++] = data;
               }
               else if (data == 'R') {
                  packet[0] = data;
                  byte_count = 1;
               }
               else {
                  byte_count = 0;
               }
            }
         }
      }
   }
   distance = (packet[1] - 0x30) * 100 + (packet[2] - 0x30) * 10 + (packet[3] - 0x30); // convert ASCII digits to integer distance value

   return (distance); // success
}


