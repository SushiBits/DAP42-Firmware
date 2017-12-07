/*
 * usb-serial.c
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#include "usb-serial.h"

#include <stm32f0xx.h>
#include <stm32f0xx_it.h>
#include <string.h>
#include <dreamos-rt/gpio.h>
#include <dreamos-rt/ring-buffer.h>

#include "usb-device.h"

RING_BUFFER_INIT_STATIC(usb_cdc_tx_buffer, 128);
RING_BUFFER_INIT_STATIC(usb_cdc_rx_buffer, 128);

static struct usb_cdc_line_coding usb_cdc_line_coding =
{ .dwDTERate = 115200, .bCharFormat = USB_CDC_1_STOP_BITS, .bParityType =
		USB_CDC_NO_PARITY, .bDataBits = 8, };

void usb_cdc_init(void)
{
	ring_buffer_drain(usb_cdc_tx_buffer);
	ring_buffer_drain(usb_cdc_rx_buffer);
	
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	__DSB();
	RCC->APB2RSTR |= RCC_APB2RSTR_USART1RST;
	__DSB();
	RCC->APB2RSTR &= ~RCC_APB2RSTR_USART1RST;
	__DSB();
	
	usb_cdc_line_coding.dwDTERate = 115200;
	usb_cdc_line_coding.bCharFormat = USB_CDC_1_STOP_BITS;
	usb_cdc_line_coding.bParityType = USB_CDC_NO_PARITY;
	usb_cdc_line_coding.bDataBits = 8;
	
	pinMode(0x16, AFIO_PU(GPIO_MODE_AF0));
	pinMode(0x17, AFIO_PU(GPIO_MODE_AF0));
}

void usb_cdc_deinit(void)
{
	pinMode(0x16, INPUT);
	pinMode(0x17, INPUT);
	
	RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
}

usbd_respond usb_cdc_control(usbd_device *dev, usbd_ctlreq *req,
		usbd_rqc_callback *callback)
{
	if (((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType)
			!= (USB_REQ_INTERFACE | USB_REQ_CLASS))
		return usbd_fail;
	
	switch (req->bRequest)
	{
	case USB_CDC_SET_LINE_CODING:
		memmove(req->data, &usb_cdc_line_coding, sizeof(usb_cdc_line_coding));
		
		// Validate data rate.
		if (usb_cdc_line_coding.dwDTERate < SystemCoreClock / UINT16_MAX)
			usb_cdc_line_coding.dwDTERate = SystemCoreClock / UINT16_MAX;
		if (usb_cdc_line_coding.dwDTERate > SystemCoreClock / 16)
			usb_cdc_line_coding.dwDTERate = SystemCoreClock / 16;
		
		// Validate the data bits.
		if (usb_cdc_line_coding.bParityType == USB_CDC_NO_PARITY)
		{
			if (usb_cdc_line_coding.bDataBits < 7)
				usb_cdc_line_coding.bDataBits = 7;
			if (usb_cdc_line_coding.bDataBits > 9)
				usb_cdc_line_coding.bDataBits = 9;
		}
		else
		{
			if (usb_cdc_line_coding.bDataBits < 6)
				usb_cdc_line_coding.bDataBits = 6;
			if (usb_cdc_line_coding.bDataBits > 8)
				usb_cdc_line_coding.bDataBits = 8;
		}
		
		return usbd_ack;
		
	case USB_CDC_GET_LINE_CODING:
		dev->status.data_ptr = &usb_cdc_line_coding;
		dev->status.data_count = sizeof(usb_cdc_line_coding);
		return usbd_ack;
		
	case USB_CDC_SET_CONTROL_LINE_STATE:
		return usbd_ack;
		
	default:
		return usbd_fail;
	}
}

void usb_cdc_handle(usbd_device *dev, uint8_t event, uint8_t ep)
{
	int _t;
	switch (event)
	{
	case usbd_evt_eptx:
		_t = usbd_ep_write(dev, USB_CDC_DATA_IN_EP, &fifo[0],
				(fpos < USB_PKT_SIZE) ? fpos : USB_PKT_SIZE);
		if (_t > 0)
		{
			memmove(&fifo[0], &fifo[_t], fpos - _t);
			fpos -= _t;
		}
		break;
	case usbd_evt_eprx:
		if (fpos < (sizeof(fifo) - USB_PKT_SIZE))
		{
			_t = usbd_ep_read(dev, USB_CDC_DATA_OUT_EP, &fifo[fpos],
					USB_PKT_SIZE);
			if (_t > 0)
			{
				fpos += _t;
			}
		}
		break;
	default:
		break;
	}
}
