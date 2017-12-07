/*
 * usb-serial.h
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#ifndef INCLUDE_USB_SERIAL_H_
#define INCLUDE_USB_SERIAL_H_

#include <sys/cdefs.h>
#include <stdint.h>
#include <stdbool.h>
#include <usb.h>
#include <usb_cdc.h>

__BEGIN_DECLS

void usb_cdc_init(void);
void usb_cdc_deinit(void);
usbd_respond usb_cdc_control(usbd_device *dev, usbd_ctlreq *req, usbd_rqc_callback *callback);
void usb_cdc_handle(usbd_device *dev, uint8_t event, uint8_t ep);

__END_DECLS

#endif /* INCLUDE_USB_SERIAL_H_ */
