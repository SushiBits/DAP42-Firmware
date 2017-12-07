/*
 * usb-device.c
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#include "usb-device.h"

#include <stm32f0xx_it.h>
#include <stdbool.h>
#include <usb.h>
#include "usb-hid.h"
#include "usb-serial.h"

static usbd_device usbd;
static uint32_t usb_buffer[USB_PKT_SIZE];

static usbd_respond usb_get_descriptor(usbd_ctlreq *req, void **address, uint16_t *length);
static usbd_respond usb_set_config(usbd_device *dev, uint8_t cfg);
static usbd_respond usb_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback);

__attribute__((constructor)) void usb_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	SET_FIELD(GPIOA->MODER, 0b1111 << 22, 0b1010 << 22);

	NVIC_EnableIRQ(USB_IRQn);

	usbd_init(&usbd, &usbd_hw, USB_PKT_SIZE, usb_buffer, sizeof(usb_buffer));
	usbd_reg_descr(&usbd, usb_get_descriptor);
	usbd_reg_control(&usbd, usb_control);
	usbd_reg_config(&usbd, usb_set_config);
	usbd_enable(&usbd, true);
	usbd_connect(&usbd, true);
}

void USB_IRQHandler(void)
{
	usbd_poll(&usbd);
}

static usbd_respond usb_get_descriptor(usbd_ctlreq *req, void **address, uint16_t *length)
{
	uint8_t descriptor_type = (req->wValue & 0xff00) >> 8;
	uint8_t descriptor_idx = req->wValue & 0xff;

	const void *buf = NULL;
	uint16_t len = 0;

	switch (descriptor_type)
	{
	case USB_DTYPE_DEVICE:
		buf = &usb_device_desc;
		break;

	case USB_DTYPE_CONFIGURATION:
		buf = &usb_device_config_desc;
		len = sizeof(usb_device_config_desc);
		break;

	case USB_DTYPE_STRING:
		if (descriptor_idx < sizeof(usb_string_table) / sizeof(struct usb_string_descriptor))
			buf = usb_string_table[descriptor_idx];
		break;

	default:
		break;
	}

	if (!buf)
		return usbd_fail;
	len = len ?: ((struct usb_device_descriptor *)buf)->bLength;

	if (address)
		*address = (void *)buf;
	if (length)
		*length = len;

	return usbd_ack;
}

static usbd_respond usb_set_config(usbd_device *dev, uint8_t cfg)
{
	switch (cfg)
	{
	case 0:
		// Deconfigure everything.
		usbd_ep_deconfig(dev, USB_HID_IN_EP);
		usbd_ep_deconfig(dev, USB_HID_OUT_EP);
		usbd_reg_endpoint(dev, USB_HID_IN_EP, NULL);
		usbd_reg_endpoint(dev, USB_HID_OUT_EP, NULL);
		usb_hid_deinit();

		usbd_ep_deconfig(dev, USB_CDC_CTRL_EP);
		usbd_ep_deconfig(dev, USB_CDC_DATA_IN_EP);
		usbd_ep_deconfig(dev, USB_CDC_DATA_OUT_EP);
		usbd_reg_endpoint(dev, USB_CDC_DATA_IN_EP, NULL);
		usbd_reg_endpoint(dev, USB_CDC_DATA_OUT_EP, NULL);
		usb_cdc_deinit();

		return usbd_ack;

	case 1:
		usbd_ep_config(dev, USB_HID_IN_EP, USB_EPTYPE_INTERRUPT, USB_PKT_SIZE);
		usbd_ep_config(dev, USB_HID_OUT_EP, USB_EPTYPE_INTERRUPT, USB_PKT_SIZE);
		usbd_reg_endpoint(dev, USB_HID_IN_EP, usb_hid_handle);
		usbd_reg_endpoint(dev, USB_HID_OUT_EP, usb_hid_handle);
		usbd_ep_write(dev, USB_HID_IN_EP, NULL, 0);
		usb_hid_init();

		usbd_ep_config(dev, USB_CDC_CTRL_EP, USB_EPTYPE_INTERRUPT, USB_PKT_SIZE);
        usbd_ep_config(dev, USB_CDC_DATA_IN_EP, USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF, USB_PKT_SIZE);
        usbd_ep_config(dev, USB_CDC_DATA_OUT_EP, USB_EPTYPE_BULK | USB_EPTYPE_DBLBUF, USB_PKT_SIZE);
		usbd_reg_endpoint(dev, USB_CDC_DATA_IN_EP, usb_cdc_handle);
		usbd_reg_endpoint(dev, USB_CDC_DATA_OUT_EP, usb_cdc_handle);
		usbd_ep_write(dev, USB_CDC_DATA_IN_EP, NULL, 0);
		usb_cdc_init();

        return usbd_ack;

	default:
		break;
	}
	return usbd_fail;
}

static usbd_respond usb_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback)
{
	if ((req->bmRequestType & USB_REQ_RECIPIENT) == USB_REQ_INTERFACE)
	{
		switch (req->wIndex)
		{
		case 0:
			return usb_hid_control(dev, req, callback);

		case 1:
		case 2:
			return usb_cdc_control(dev, req, callback);

		default:
			return usbd_fail;
		}
	}

	if ((req->bmRequestType & USB_REQ_RECIPIENT) == USB_REQ_ENDPOINT)
	{
		switch (req->wIndex)
		{
		case USB_HID_IN_EP:
		case USB_HID_OUT_EP:
			return usb_hid_control(dev, req, callback);

		case USB_CDC_CTRL_EP:
		case USB_CDC_DATA_IN_EP:
		case USB_CDC_DATA_OUT_EP:
			return usb_cdc_control(dev, req, callback);

		default:
			return usbd_fail;
		}
	}

	return usbd_fail;
}
