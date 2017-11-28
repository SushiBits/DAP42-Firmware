/*
 * time.c
 *
 *  Created on: Sep 24, 2017
 *      Author: technix
 */

#include <stm32f0xx.h>
#include <stm32f0xx_it.h>

#include <dreamos-rt/time.h>

#define _COMPILING_NEWLIB
#include <time.h>
#include <sys/time.h>
#include <sys/times.h>
#include <unistd.h>

/*
 * SysTick-based application run time clock.
 */

uint32_t millis_counter = 0;

__attribute__((constructor(1000))) void systick_init(void)
{
	SysTick_Config(SystemCoreClock / 1000);
}

void SysTick_IRQHandler(void)
{
	millis_counter++;
}

void yield(void) {}

uint32_t millis(void)
{
	return millis_counter;
}

uint32_t micros(void)
{
	uint32_t ms = millis_counter;
	uint32_t ticks;
	do
		ticks = SysTick->LOAD - SysTick->VAL;
	while (ms != millis_counter);

	return ms * 1000 + (ticks / (SystemCoreClock / 1000000));
}

unsigned sleep(unsigned seconds)
{
	int32_t ms_end = millis_counter + seconds * 1000;

	while (ms_end - (int32_t)millis_counter > 0)
		yield();

	return 0;
}

int usleep(useconds_t useconds)
{
	uint32_t ms = millis_counter;
	uint32_t ticks;
	do
		ticks = SysTick->VAL;
	while (ms != millis_counter && ticks > 20);
	ms = millis_counter;

	int32_t ms_end = ms + (useconds / 1000);
	int32_t ticks_end = ticks - (useconds % 1000) * (SystemCoreClock / 1000000);
	while (ticks_end < 0)
	{
		ms_end++;
		ticks_end += SysTick->LOAD;
	}

	while (ms_end - (int32_t)millis_counter > 0)
		yield();
	while ((int32_t)SysTick->VAL - ticks_end > 0)
		yield();

	return 0;
}

clock_t _times(struct tms *tm)
{
	if (tm)
	{
		tm->tms_utime = millis() / (1000 / CLK_TCK);
		tm->tms_stime = 0;
		tm->tms_cutime = 0;
		tm->tms_cstime = 0;
	}

	return 0;
}
