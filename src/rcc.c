/*
 * rcc.c
 *
 *  Created on: Nov 8, 2017
 *      Author: technix
 */

#include <stm32f0xx.h>

void SystemInit(void)
{
	RCC->CR |= RCC_CR_HSEON;
	while (!(RCC->CR & RCC_CR_HSERDY))
		;
	
	RCC->CFGR = RCC_CFGR_PLLSRC_HSE_PREDIV | RCC_CFGR_PLLMUL6;
	RCC->CR |= RCC_CR_PLLON;
	while (!(RCC->CR & RCC_CR_PLLRDY))
		;
	
	FLASH->ACR = (1 << FLASH_ACR_LATENCY_Pos) | FLASH_ACR_PRFTBE;
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL)
		;
	
	RCC->CFGR3 = RCC_CFGR3_USART1SW_SYSCLK | RCC_CFGR3_USBSW_PLLCLK;
}
