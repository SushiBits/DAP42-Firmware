/*
 * usb-device.c
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#include "usb-device.h"

#include <stm32f0xx.h>
#include <stm32f0xx_it.h>
#include <stdbool.h>
#include <usb.h>

static usbd_device usbd;

__attribute__((constructor)) void usb_init(void)
{
	NVIC_EnableIRQ(USB_IRQn);

	usbd_enable(&usbd, true);
	usbd_connect(&usbd, true);
}

void USB_IRQHandler(void)
{
	usbd_poll(&usbd);
}
