/*
 * gpio.c
 *
 *  Created on: Sep 17, 2017
 *      Author: technix
 */

#include <dreamos-rt/gpio.h>
#include <stm32f0xx.h>

__attribute__((constructor(1000))) void GPIO_Initialize(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN | RCC_AHBENR_GPIOBEN;
}

static inline GPIO_TypeDef *GPIO_GetPin(uint8_t pin, uint8_t *pin_id)
{
	static GPIO_TypeDef *const GPIO[] =
	{
			GPIOA,
			GPIOB,
			GPIOC,
			NULL,
			NULL,
			GPIOF
	};

	uint8_t gpio = (pin & 0xf0) >> 4;
	if (gpio > 5)
		return NULL;

	if (pin_id)
		*pin_id = pin & 0x0f;
	return GPIO[gpio];
}

uint16_t getPinMode(uint8_t pin)
{
	uint8_t pinid;
	GPIO_TypeDef *GPIO = GPIO_GetPin(pin, &pinid);
	if (!GPIO)
		return 0;

	uint32_t direction = (GPIO->MODER >> (pinid * 2)) & 0x3;
	uint32_t type = (GPIO->OTYPER >> pinid) & 0x1;
	uint32_t speed = (GPIO->OSPEEDR >> (pinid * 2)) & 0x3;
	uint32_t pupd = (GPIO->PUPDR >> (pinid * 2)) & 0x3;
	uint32_t afio = (GPIO->AFR[(pinid & 0x8) ? 1 : 0] >> ((pinid & 0x7) * 4)) & 0xf;

	return
			direction << GPIO_MODE_Pos |
			type << GPIO_MODE_TYPE_Pos |
			speed << GPIO_MODE_SPEED_Pos |
			pupd << GPIO_MODE_PUPD_Pos |
			afio << GPIO_MODE_AFIO_Pos;
}

void pinMode(uint8_t pin, uint16_t mode)
{
	uint8_t pinid;
	GPIO_TypeDef *GPIO = GPIO_GetPin(pin, &pinid);
	if (!GPIO)
		return;

	uint32_t direction = (mode & GPIO_MODE_Msk) >> GPIO_MODE_Pos;
	uint32_t type = (mode & GPIO_MODE_TYPE_Msk) >> GPIO_MODE_TYPE_Pos;
	uint32_t speed = (mode & GPIO_MODE_SPEED_Msk) >> GPIO_MODE_SPEED_Pos;
	uint32_t pupd = (mode & GPIO_MODE_PUPD_Msk) >> GPIO_MODE_PUPD_Pos;
	uint32_t afio = (mode & GPIO_MODE_AFIO_Msk) >> GPIO_MODE_AFIO_Pos;

	SET_FIELD(GPIO->MODER, 0x3 << (pinid * 2), direction << (pinid * 2));
	SET_FIELD(GPIO->OTYPER, 0x1 << pinid, type << pinid);
	SET_FIELD(GPIO->OSPEEDR, 0x3 << (pinid * 2), speed << (pinid * 2));
	SET_FIELD(GPIO->PUPDR, 0x3 << (pinid * 2), pupd << (pinid * 2));
	SET_FIELD(GPIO->AFR[(pinid & 0x8) ? 1 : 0], 0xf << ((pinid & 0x7) * 4), afio << ((pinid & 0x7) * 4));
	__DSB();
}

void digitalWrite(uint8_t pin, bool value)
{
	uint8_t pinid;
	GPIO_TypeDef *GPIO = GPIO_GetPin(pin, &pinid);
	if (!GPIO)
		return;

	SET_FIELD(GPIO->ODR, 0x1 << pinid, (value ? 0x1 : 0x0) << pinid);
	__DSB();
}

bool digitalRead(uint8_t pin)
{
	uint8_t pinid;
	GPIO_TypeDef *GPIO = GPIO_GetPin(pin, &pinid);
	if (!GPIO)
		return false;

	return ((GPIO->IDR >> pinid) & 0x1) ? true : false;
}
