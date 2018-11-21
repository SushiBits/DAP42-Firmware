/*
 * main.c
 *
 *  Created on: Nov 8, 2017
 *      Author: technix
 */

#include <stm32f0xx.h>

#include "usb-hid.h"

int main(void)
{
	for (;;)
		usb_hid_update_dap();
}
