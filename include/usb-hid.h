/*
 * usb-hid.h
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#ifndef INCLUDE_USB_HID_H_
#define INCLUDE_USB_HID_H_

#include <sys/cdefs.h>
#include <stdint.h>
#include <stdbool.h>
#include <usb.h>
#include <usb_hid.h>

__BEGIN_DECLS

void usb_hid_init(void);
usbd_respond usb_hid_control(usbd_device *dev, usbd_ctlreq *req,
		usbd_rqc_callback *callback);
void usb_hid_handle(usbd_device *dev, uint8_t event, uint8_t ep);
void usb_hid_update(void);

__END_DECLS

#endif /* INCLUDE_USB_HID_H_ */
