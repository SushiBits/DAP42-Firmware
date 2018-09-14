/*
 * usb-hid.c
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#include "usb-hid.h"

#include <stm32f0xx.h>
#include <stm32f0xx_it.h>
#include <string.h>

#include "usb-device.h"

#include "DAP_config.h"
#include "DAP.h"

static uint8_t USB_Request[DAP_PACKET_COUNT][DAP_PACKET_SIZE];
static uint8_t USB_NewRequest[DAP_PACKET_SIZE];
static uint32_t free_count = (DAP_PACKET_COUNT);
static uint32_t send_count = 0;
static uint32_t recv_idx = 0;
static uint32_t send_idx = 0;

void usb_hid_init(void)
{
	DAP_Setup();
	memset(USB_Request, 0, sizeof(USB_Request));
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
		if (send_count)
		{
			send_count--;
			usbd_ep_write(dev, USB_HID_IN_EP, USB_Request[send_idx], DAP_PACKET_SIZE);
	        send_idx = (send_idx + 1) % DAP_PACKET_COUNT;
			free_count++;
		}
		else
		{
			usbd_ep_write(dev, USB_HID_IN_EP, NULL, 0);
		}
		break;

	case usbd_evt_eprx:
		val = usbd_ep_read(dev, USB_HID_OUT_EP, USB_NewRequest, DAP_PACKET_SIZE);

		if (val <= 0)
			break;

		if (USB_NewRequest[0] == ID_DAP_TransferAbort)
		{
            DAP_TransferAbort = 1;
            break;
		}

		if (!free_count)
			break;

        free_count--;
        memcpy(USB_Request[recv_idx], USB_NewRequest, val);
        DAP_ExecuteCommand(USB_NewRequest, USB_Request[recv_idx]);
        recv_idx = (recv_idx + 1) % DAP_PACKET_COUNT;
        send_count++;
		break;

	default:
		break;
	}
}
