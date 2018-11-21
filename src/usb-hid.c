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
static uint8_t USB_Response[DAP_PACKET_COUNT][DAP_PACKET_SIZE];
static uint32_t USB_ResponseSize[DAP_PACKET_COUNT];
static uint8_t USB_NewRequest[DAP_PACKET_SIZE];
static uint32_t free_count = (DAP_PACKET_COUNT);
static uint32_t send_count = 0;
static uint32_t recv_idx = 0;
static uint32_t send_idx = 0;
static uint32_t handle_idx = 0;

void usb_hid_init(void)
{
	DAP_Setup();
	memset(USB_Request, 0, sizeof(USB_Request));
	memset(USB_Response, 0, sizeof(USB_Response));
	send_count = 0;
	free_count = (DAP_PACKET_COUNT);
	recv_idx = 0;
	send_idx = 0;
	handle_idx = 0;
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
			usbd_ep_write(dev, ep, USB_Response[send_idx], /*(USB_ResponseSize[send_idx] & 0xffff) ?:*/ DAP_PACKET_SIZE);
	        send_idx = (send_idx + 1) % DAP_PACKET_COUNT;
	        free_count++;
		}
		usbd_ep_write(dev, ep, NULL, 0);
		break;

	case usbd_evt_eprx:
		memset(USB_NewRequest, 0, DAP_PACKET_SIZE);
		val = usbd_ep_read(dev, ep, USB_NewRequest, DAP_PACKET_SIZE);

		if (val <= 0)
			break;

		if (!free_count)
			break;

        free_count--;
        memset(USB_Request[recv_idx], 0, DAP_PACKET_SIZE);
        memset(USB_Response[recv_idx], 0, DAP_PACKET_SIZE);
        memcpy(USB_Request[recv_idx], USB_NewRequest, val);
        USB_ResponseSize[recv_idx] = DAP_ExecuteCommand(USB_Request[recv_idx], USB_Response[recv_idx]);
        recv_idx = (recv_idx + 1) % DAP_PACKET_COUNT;
        send_count++;
		break;

	default:
		break;
	}
}

void usb_hid_update_dap(void)
{
	/*
	while (free_count < DAP_PACKET_COUNT && send_count < DAP_PACKET_COUNT)
	{
		USB_ResponseSize[handle_idx] = DAP_ExecuteCommand(USB_Request[handle_idx], USB_Response[handle_idx]);
		handle_idx = (handle_idx + 1) % DAP_PACKET_COUNT;
		free_count++;
		send_count--;
	}
	*/

	__WFE();
}
