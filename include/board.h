/*
 * board.h
 *
 *  Created on: Dec 5, 2017
 *      Author: technix
 */

#ifndef INCLUDE_BOARD_H_
#define INCLUDE_BOARD_H_

#include <stm32f0xx.h>
#include <dreamos-rt/gpio.h>

#define UART         USART1
#define UART_RX      0x17
#define UART_TX      0x16

#define JTAG_SWO     USART2
#define JTAG_TRST    0x07
#define JTAG_TDI     0x06
#define JTAG_TMS     0x05
#define JTAG_TCK     0x04
#define JTAG_RTCK    0x02
#define JTAG_TDO     0x03
#define JTAG_RST     0x01

#define SPI          SPI1
#define SPI_NSS      0x0f
#define SPI_MISO     0x14
#define SPI_MOSI     0x15
#define SPI_SCK      0x13

#define WKUP_BTN     0x00
#define TGT_PWR      0x10

#endif /* INCLUDE_BOARD_H_ */
