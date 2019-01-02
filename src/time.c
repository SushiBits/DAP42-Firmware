/*
 * time.c
 *
 *  Created on: Dec 6, 2018
 *      Author: technix
 */

#include "time.h"

#include <stm32f0xx.h>

static uint32_t ms_counter = 0;

__attribute__((constructor)) void time_begin(void)
{
	SysTick_Config(SystemCoreClock / 1000);
}

void SysTick_IRQHandler(void)
{
	ms_counter++;
}

uint32_t millis(void)
{
	return ms_counter;
}

