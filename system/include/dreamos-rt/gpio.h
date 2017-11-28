/*
 * gpio.h
 *
 *  Created on: Sep 17, 2017
 *      Author: technix
 */

#ifndef SYSTEM_INCLUDE_DREAMOS_RT_GPIO_H_
#define SYSTEM_INCLUDE_DREAMOS_RT_GPIO_H_

#include <stdint.h>
#include <stdbool.h>
#include <sys/cdefs.h>

#define GPIO_MODE_Pos		(0)
#define GPIO_MODE_Msk		(0x3 << GPIO_MODE_Pos)
#define GPIO_MODE_INPUT		(0 << GPIO_MODE_Pos)
#define GPIO_MODE_OUTPUT		(1 << GPIO_MODE_Pos)
#define GPIO_MODE_AFIO		(2 << GPIO_MODE_Pos)
#define GPIO_MODE_ANALOG		(3 << GPIO_MODE_Pos)

#define GPIO_MODE_TYPE_Pos	(2)
#define GPIO_MODE_TYPE_Msk	(0x1 << GPIO_MODE_TYPE_Pos)
#define GPIO_MODE_TYPE_PP	(0 << GPIO_MODE_TYPE_Pos)
#define GPIO_MODE_TYPE_OD	(1 << GPIO_MODE_TYPE_Pos)

#define GPIO_MODE_SPEED_Pos	(4)
#define GPIO_MODE_SPEED_Msk	(0x3 << GPIO_MODE_SPEED_Pos)
#define GPIO_MODE_SPEED_LO	(0 << GPIO_MODE_SPEED_Pos)
#define GPIO_MODE_SPEED_MID	(1 << GPIO_MODE_SPEED_Pos)
#define GPIO_MODE_SPEED_HI	(3 << GPIO_MODE_SPEED_Pos)

#define GPIO_MODE_PUPD_Pos	(6)
#define GPIO_MODE_PUPD_Msk	(0x3 << GPIO_MODE_PUPD_Pos)
#define GPIO_MODE_PUPD_NONE	(0 << GPIO_MODE_PUPD_Pos)
#define GPIO_MODE_PUPD_PU	(1 << GPIO_MODE_PUPD_Pos)
#define GPIO_MODE_PUPD_PD	(2 << GPIO_MODE_PUPD_Pos)

#define GPIO_MODE_AFIO_Pos	(8)
#define GPIO_MODE_AFIO_Msk	(0xf << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF0		(0 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF1		(1 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF2		(2 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF3		(3 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF4		(4 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF5		(5 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF6		(6 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF7		(7 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF8		(8 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF9		(9 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF10		(10 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF11		(11 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF12		(12 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF13		(13 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF14		(14 << GPIO_MODE_AFIO_Pos)
#define GPIO_MODE_AF15		(15 << GPIO_MODE_AFIO_Pos)

#define INPUT				(GPIO_MODE_INPUT | GPIO_MODE_PUPD_NONE)
#define ANALOG				(GPIO_MODE_ANALOG | GPIO_MODE_PUPD_NONE)
#define INPUT_PULLUP			(GPIO_MODE_INPUT | GPIO_MODE_PUPD_PU)
#define OUTPUT				(GPIO_MODE_OUTPUT | GPIO_MODE_TYPE_PP | GPIO_MODE_SPEED_HI)
#define AFIO(afio)			(GPIO_MODE_AFIO | GPIO_MODE_TYPE_PP | GPIO_MODE_PUPD_NONE | GPIO_MODE_SPEED_HI | afio)
#define AFIO_OD(afio)		(GPIO_MODE_AFIO | GPIO_MODE_TYPE_OD | GPIO_MODE_PUPD_NONE | GPIO_MODE_SPEED_HI | afio)
#define AFIO_PU(afio)		(GPIO_MODE_AFIO | GPIO_MODE_TYPE_PP | GPIO_MODE_PUPD_PU | GPIO_MODE_SPEED_HI | afio)
#define AFIO_OD_PU(afio)		(GPIO_MODE_AFIO | GPIO_MODE_TYPE_OD | GPIO_MODE_PUPD_PU | GPIO_MODE_SPEED_HI | afio)

__BEGIN_DECLS

uint16_t getPinMode(uint8_t pin);
void pinMode(uint8_t pin, uint16_t mode);
void digitalWrite(uint8_t pin, bool value);
bool digitalRead(uint8_t pin);

__END_DECLS

#endif /* SYSTEM_INCLUDE_DREAMOS_RT_GPIO_H_ */
