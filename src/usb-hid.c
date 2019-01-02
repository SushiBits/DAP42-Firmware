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

struct DAP_Packet
{
	uint8_t data[DAP_PACKET_SIZE];
	uint8_t size;
};

#define HID_PACKET_COUNT (DAP_PACKET_COUNT + 1)

static struct DAP_Packet request[HID_PACKET_COUNT];
static uint8_t req_buffer[DAP_PACKET_SIZE];
static uint8_t request_head = 0;
static uint8_t request_tail = 0;

static struct DAP_Packet response[HID_PACKET_COUNT];
static uint8_t response_head = 0;
static uint8_t response_tail = 0;

#define LOOP_NEXT(x) ((x + 1) % HID_PACKET_COUNT)
#define LOOP_INCREASE(var) var = LOOP_NEXT(var)

void usb_hid_init(void)
{
	DAP_Setup();
	memset(req_buffer, 0, sizeof(req_buffer));
	memset(request, 0, sizeof(request));
	memset(response, 0, sizeof(response));
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
		if (response_head != response_tail)
		{
			struct DAP_Packet *packet = &(response[response_tail]);
			usbd_ep_write(dev, USB_HID_IN_EP, packet->data, packet->size);
			LOOP_INCREASE(response_tail);
		}
		else
		{
			usbd_ep_write(dev, USB_HID_IN_EP, NULL, 0);
		}
		break;

	case usbd_evt_eprx:
		memset(req_buffer, 0, DAP_PACKET_SIZE);
		val = usbd_ep_read(dev, USB_HID_OUT_EP, req_buffer, DAP_PACKET_SIZE);

		if (val <= 0)
			break;

		if (LOOP_NEXT(request_head) != request_tail)
		{
			struct DAP_Packet *req = &(request[request_head]);
			memcpy(req->data, req_buffer, DAP_PACKET_SIZE);
			LOOP_INCREASE(request_head);
		}
		break;

	default:
		break;
	}
}

void usb_hid_update(void)
{
	uint32_t value = 0;
	while ((request_head != request_tail) && (LOOP_NEXT(response_head) != response_tail))
	{
		struct DAP_Packet *req = &(request[request_tail]);
		struct DAP_Packet *resp = &(response[response_head]);
		memset(resp->data, 0, sizeof(resp->data));
		value = DAP_ExecuteCommand(req->data, resp->data);
		resp->size = DAP_PACKET_SIZE;
		LOOP_INCREASE(request_tail);
		LOOP_INCREASE(response_head);
	}
}
