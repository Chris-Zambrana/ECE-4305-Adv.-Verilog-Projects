/*****************************************************************//**
 * @file uart_core.h
 *
 * @brief Access MMIO timer core and
 *        display number/sting on a serial console
 *
 * @author p chu
 * @version v1.0: initial release
 ********************************************************************/

#ifndef _SENSOR_UART_CORE_H_INCLUDED
#define _SENSOR_UART_CORE_H_INCLUDED

#include "chu_init.h"
#include "chu_io_map.h"  // to use SYS_CLK_FREQ
/**
 * uart core driver
 * - transmit/receive data via MMIO uart core.
 * - display (print) number and string on serial console
 *
 */
class SensorUartCore {
   /**
    * register map
    *
    */
   enum {
      RD_DATA_REG = 0,   /**< rx data/status register */
      DVSR_REG = 1,      /**< baud rate divisor register */
      WR_DATA_REG = 2,   /**< wr data register */
      RM_RD_DATA_REG = 3 /**< remove read data offset */
   };
  /**
   * mask fields
   *
   */
   enum {
      TX_FULL_FIELD = 0x00000200, /**< bit 9 of rd_data_reg; full bit  */
      RX_EMPT_FIELD = 0x00000100, /**< bit 10 of rd_data_reg; empty bit */
      RX_DATA_FIELD = 0x000000ff  /**< bits 7..0 rd_data_reg; read data */
   };

public:
   /* methods */
   /**
    * constructor.
    *
    * @note set the default rate to 9600 baud
    */
   SensorUartCore(uint32_t core_base_addr);
   ~SensorUartCore();

   /**
    * set baud rate
    *
    * @param baud baud rate
    * @note baud rate = sys_clk_freq/16/(dvsr+1)
    */
   void set_baud_rate(int baud);

   /**
    * check whether uart receiver fifo is empty
    *
    * @return 1: if empty; 0: otherwise
    *
    */
   int rx_fifo_empty();

   /**
    * receive single byte from uart
    *
    * @return -1 if rx fifo empty; byte data otherwise
    */
   int rx_byte();

   /**
    * receive distance data
    *
    * @return distance in inches; -1 on packet format error
    *
    * @note the function blocks until complete packet received
    * @note packet format: 'R' + 3 ASCII digits + CR(0x0D)
    */
   uint8_t rx_distance();

private:
   uint32_t base_addr;
   int baud_rate;
};

#endif  // _SENSOR_UART_CORE_H_INCLUDED
