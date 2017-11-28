/*
 * usb-hid.c
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#include "usb-hid.h"

#include <stm32f0xx.h>
#include <stm32f0xx_it.h>
#include "usb-device.h"

usbd_respond usb_hid_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback)
{
	uint8_t descriptor_type = (req->wValue & 0xff00) >> 8;
	uint8_t descriptor_idx = req->wValue & 0xff;

	if (req->bRequest == USB_STD_GET_DESCRIPTOR)
	{
		switch (descriptor_type)
		{
		case USB_DTYPE_HID_REPORT:
			dev->status.data_ptr = usb_hid_report_descriptor;
			dev->status.data_count = sizeof(usb_hid_report_descriptor);
			return usbd_ack;

		case USB_DTYPE_QUALIFIER:
			dev->status.data_ptr = &usb_qualifier_desc;
			dev->status.data_count = sizeof(usb_qualifier_desc);
			return usbd_ack;

		default:
			return usbd_fail;
		}
	}

	return usbd_fail;
}

void usb_hid_handle(usbd_device *dev, uint8_t event, uint8_t ep)
{

}
