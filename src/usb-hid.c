/*
 * usb-hid.c
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#include "usb-hid.h"

#include <stm32f0xx.h>
#include <stm32f0xx_it.h>
#include <dreamos-rt/ring-buffer.h>

#include "usb-device.h"

RING_BUFFER_INIT_STATIC(usb_hid_tx_buffer, 128);
RING_BUFFER_INIT_STATIC(usb_hid_rx_buffer, 128);

void usb_hid_init(void)
{
	ring_buffer_drain(usb_hid_tx_buffer);
	ring_buffer_drain(usb_hid_rx_buffer);
}

void usb_hid_deinit(void)
{
	
}

usbd_respond usb_hid_control(usbd_device *dev, usbd_ctlreq *req,
		usbd_rqc_callback *callback)
{
	uint8_t descriptor_type = (req->wValue & 0xff00) >> 8;
	__attribute__((unused))  uint8_t descriptor_idx = req->wValue & 0xff;
	
	if (req->bRequest == USB_STD_GET_DESCRIPTOR)
	{
		switch (descriptor_type)
		{
		case USB_DTYPE_HID_REPORT:
			dev->status.data_ptr = (void *)usb_hid_report_descriptor;
			dev->status.data_count = sizeof(usb_hid_report_descriptor);
			return usbd_ack;
			
		default:
			return usbd_fail;
		}
	}
	
	return usbd_fail;
}

void usb_hid_handle(usbd_device *dev, uint8_t event, uint8_t ep)
{
	int val;
	switch (event)
	{
	case usbd_evt_eptx:
		for (off_t count = 0; count < USB_PKT_SIZE; count++)
		{
			int rv = ring_buffer_peekchar(usb_hid_rx_buffer);
			if (rv > 0)
			{
				uint8_t ch = rv;
				val = usbd_ep_write(dev, USB_HID_IN_EP, &ch, 1);
				if (val > 0)
					ring_buffer_getchar(usb_hid_rx_buffer);
				else
					break;
			}
			else
			{
				usbd_ep_write(dev, USB_HID_IN_EP, NULL, 0);
				break;
			}
		}
		break;
	case usbd_evt_eprx:
		for (;;)
		{
			uint8_t ch = 0;
			val = usbd_ep_read(dev, USB_HID_OUT_EP, &ch, 1);
			if (val > 0)
			{
				ring_buffer_putchar(usb_hid_rx_buffer, ch);
			}
			else
			{
				break;
			}
		}
		break;
	default:
		break;
	}
}
