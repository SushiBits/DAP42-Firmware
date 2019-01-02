/*
 * main.c
 *
 *  Created on: Nov 8, 2017
 *      Author: technix
 */

#include <stm32f0xx.h>
#include "usb-hid.h"
#include "usb-serial.h"

int main(void)
{
	usb_hid_init();

	for (;;)
	{
		usb_hid_update();
		usb_cdc_update();
		__WFE();
	}
}
