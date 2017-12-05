/*
 * usb-device.h
 *
 *  Created on: Nov 13, 2017
 *      Author: technix
 */

#ifndef INCLUDE_USB_DEVICE_H_
#define INCLUDE_USB_DEVICE_H_

#define USB_VID          0x0002
#define USB_PID          0x8010

#define USB_VENDOR_NAME  "SushiBits"
#define USB_PRODUCT_NAME "DAP42 Comprehensive Debug Adapter"
#define CDC_PRODUCT_NAME "DAP42 Virtual COM Port"
#define HID_PRODUCT_NAME "DAP42 CMSIS-DAP Debug Probe"
#define MSC_PRODUCT_NAME "DAP42 Mass Storage"

#define USB_PKT_SIZE        64
#define USB_HID_IN_EP       (0x01 | USB_EPDIR_IN)
#define USB_HID_OUT_EP      (0x01 | USB_EPDIR_OUT)
#define USB_CDC_CTRL_EP     (0x02 | USB_EPDIR_OUT)
#define USB_CDC_DATA_IN_EP  (0x03 | USB_EPDIR_IN)
#define USB_CDC_DATA_OUT_EP (0x03 | USB_EPDIR_OUT)

#include <sys/cdefs.h>
#include <stdint.h>
#include <usb_std.h>
#include <usb_cdc.h>
#include <usb_hid.h>

__BEGIN_DECLS

__attribute__((packed)) struct usb_device_config_descriptor
{
	struct usb_config_descriptor    config_descriptor;

	// USB HID
	struct usb_iad_descriptor       hid_iad_descriptor;
	struct usb_interface_descriptor hid_interface_descriptor;
	struct usb_hid_descriptor       hid_descriptor;
	struct usb_endpoint_descriptor  hid_ep_in_descriptor;
	struct usb_endpoint_descriptor  hid_ep_out_descriptor;

	// USB CDC ACM
	struct usb_iad_descriptor       cdc_iad_descriptor;
	struct usb_interface_descriptor cdc_ctrl_interface_descriptor;
    struct usb_cdc_header_desc      cdc_header_descriptor;
    struct usb_cdc_call_mgmt_desc   cdc_call_management_descriptor;
    struct usb_cdc_acm_desc         cdc_acm_descriptor;
    struct usb_cdc_union_desc       cdc_union_descriptor;
    struct usb_endpoint_descriptor  cdc_ep_ctrl_descriptor;
    struct usb_interface_descriptor cdc_data_interface_descriptor;
    struct usb_endpoint_descriptor  cdc_ep_data_rx_descriptor;
    struct usb_endpoint_descriptor  cdc_ep_data_tx_descriptor;

    // USB MSC
};

static const struct usb_device_descriptor usb_device_desc =
{
		.bLength            = sizeof(struct usb_device_descriptor),
	    .bDescriptorType    = USB_DTYPE_DEVICE,
	    .bcdUSB             = VERSION_BCD(2, 0, 0),
	    .bDeviceClass       = 0xef,
	    .bDeviceSubClass    = 0x02,
	    .bDeviceProtocol    = 0x01,
	    .bMaxPacketSize0    = USB_PKT_SIZE,
	    .idVendor           = USB_VID,
	    .idProduct          = USB_PID,
	    .bcdDevice          = VERSION_BCD(0, 1, 0),
	    .iManufacturer      = 1,
	    .iProduct           = 2,
	    .iSerialNumber      = INTSERIALNO_DESCRIPTOR,
	    .bNumConfigurations = 1
};

static const uint8_t usb_hid_report_descriptor[] =
{
	    0x06, 0x00, 0xff,              // USAGE_PAGE (Vendor Defined Page 1)
		0x09, 0x01,                    // USAGE (Vendor Usage 1)
	    0xa1, 0x01,                    // COLLECTION (Application)
	    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
	    0x26, 0xff, 0x00,              //   LOGICAL_MAXIMUM (255)
	    0x75, 0x08,                    //   REPORT_SIZE (8)
	    0x95, 0x40,                    //   REPORT_COUNT (64)
	    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
	    0x81, 0x82,                    //   INPUT (Data,Var,Abs,Vol)
	    0x09, 0x01,                    //   USAGE (Vendor Usage 1)
	    0x91, 0x82,                    //   OUTPUT (Data,Var,Abs,Vol)
	    0xc0                           // END_COLLECTION
};

static const struct usb_device_config_descriptor usb_device_config_desc =
{
		.config_descriptor =
		{
				.bLength             = sizeof(struct usb_config_descriptor),
				.bDescriptorType     = USB_DTYPE_CONFIGURATION,
				.wTotalLength        = sizeof(struct usb_device_config_descriptor),
				.bNumInterfaces      = 3,
				.bConfigurationValue = 1,
				.iConfiguration      = NO_DESCRIPTOR,
		        .bmAttributes        = USB_CFG_ATTR_RESERVED | USB_CFG_ATTR_SELFPOWERED,
		        .bMaxPower           = USB_CFG_POWER_MA(450)
		},

		.hid_iad_descriptor =
		{
			    .bLength             = sizeof(struct usb_iad_descriptor),
			    .bDescriptorType     = USB_DTYPE_INTERFASEASSOC,
			    .bFirstInterface     = 0,
			    .bInterfaceCount     = 1,
			    .bFunctionClass      = USB_CLASS_HID,
			    .bFunctionSubClass   = USB_HID_SUBCLASS_NONBOOT,
			    .bFunctionProtocol   = USB_HID_PROTO_NONBOOT,
			    .iFunction           = 3
		},
		.hid_interface_descriptor =
		{
			    .bLength             = sizeof(struct usb_interface_descriptor),
			    .bDescriptorType     = USB_DTYPE_INTERFACE,
			    .bInterfaceNumber    = 0,
			    .bAlternateSetting   = 0,
			    .bNumEndpoints       = 2,
			    .bInterfaceClass     = USB_CLASS_HID,
			    .bInterfaceSubClass  = USB_HID_SUBCLASS_NONBOOT,
			    .bInterfaceProtocol  = USB_HID_PROTO_NONBOOT,
			    .iInterface          = 3
		},
		.hid_descriptor =
		{
				.bLength             = sizeof(struct usb_hid_descriptor),
				.bDescriptorType     = USB_DTYPE_HID,
				.bcdHID              = VERSION_BCD(1, 1, 1),
				.bCountryCode        = USB_HID_COUNTRY_US,
				.bNumDescriptors     = 1,
				.bDescriptorType0    = USB_DTYPE_HID_REPORT,
				.wDescriptorLength0  = sizeof(usb_hid_report_descriptor)
		},
		.hid_ep_in_descriptor =
		{
		        .bLength             = sizeof(struct usb_endpoint_descriptor),
		        .bDescriptorType     = USB_DTYPE_ENDPOINT,
		        .bEndpointAddress    = USB_HID_IN_EP,
		        .bmAttributes        = USB_EPTYPE_INTERRUPT,
		        .wMaxPacketSize      = USB_PKT_SIZE,
		        .bInterval           = 1
		},
		.hid_ep_out_descriptor =
		{
		        .bLength             = sizeof(struct usb_endpoint_descriptor),
		        .bDescriptorType     = USB_DTYPE_ENDPOINT,
		        .bEndpointAddress    = USB_HID_OUT_EP,
		        .bmAttributes        = USB_EPTYPE_INTERRUPT,
		        .wMaxPacketSize      = USB_PKT_SIZE,
		        .bInterval           = 1
		},

		.cdc_iad_descriptor =
		{
			    .bLength             = sizeof(struct usb_iad_descriptor),
			    .bDescriptorType     = USB_DTYPE_INTERFASEASSOC,
			    .bFirstInterface     = 1,
			    .bInterfaceCount     = 2,
			    .bFunctionClass      = USB_CLASS_CDC,
			    .bFunctionSubClass   = USB_CDC_SUBCLASS_ACM,
			    .bFunctionProtocol   = USB_CDC_PROTO_V25TER,
			    .iFunction           = 4
		},
		.cdc_ctrl_interface_descriptor =
		{
			    .bLength             = sizeof(struct usb_interface_descriptor),
			    .bDescriptorType     = USB_DTYPE_INTERFACE,
			    .bInterfaceNumber    = 1,
			    .bAlternateSetting   = 0,
			    .bNumEndpoints       = 1,
			    .bInterfaceClass     = USB_CLASS_CDC,
			    .bInterfaceSubClass  = USB_CDC_SUBCLASS_ACM,
			    .bInterfaceProtocol  = USB_CDC_PROTO_V25TER,
			    .iInterface          = 4
		},
		.cdc_header_descriptor =
		{
		        .bFunctionLength     = sizeof(struct usb_cdc_header_desc),
		        .bDescriptorType     = USB_DTYPE_CS_INTERFACE,
		        .bDescriptorSubType  = USB_DTYPE_CDC_HEADER,
		        .bcdCDC              = VERSION_BCD(1,1,0)
		},
		.cdc_call_management_descriptor =
		{
		        .bFunctionLength     = sizeof(struct usb_cdc_call_mgmt_desc),
		        .bDescriptorType     = USB_DTYPE_CS_INTERFACE,
		        .bDescriptorSubType  = USB_DTYPE_CDC_CALL_MANAGEMENT,
		        .bmCapabilities      = 0,
		        .bDataInterface      = 2
		},
	    .cdc_acm_descriptor =
	    {
	        .bFunctionLength         = sizeof(struct usb_cdc_acm_desc),
	        .bDescriptorType         = USB_DTYPE_CS_INTERFACE,
	        .bDescriptorSubType      = USB_DTYPE_CDC_ACM,
	        .bmCapabilities          = USB_CDC_CAP_LINE | USB_CDC_CAP_BRK,
	    },
	    .cdc_union_descriptor =
	    {
	        .bFunctionLength         = sizeof(struct usb_cdc_union_desc),
	        .bDescriptorType         = USB_DTYPE_CS_INTERFACE,
	        .bDescriptorSubType      = USB_DTYPE_CDC_UNION,
	        .bMasterInterface0       = 1,
	        .bSlaveInterface0        = 2,
	    },
		.cdc_ep_ctrl_descriptor =
		{
		        .bLength             = sizeof(struct usb_endpoint_descriptor),
		        .bDescriptorType     = USB_DTYPE_ENDPOINT,
		        .bEndpointAddress    = USB_CDC_CTRL_EP,
		        .bmAttributes        = USB_EPTYPE_INTERRUPT,
		        .wMaxPacketSize      = USB_PKT_SIZE,
		        .bInterval           = 1
		},
		.cdc_data_interface_descriptor =
		{
			    .bLength             = sizeof(struct usb_interface_descriptor),
			    .bDescriptorType     = USB_DTYPE_INTERFACE,
			    .bInterfaceNumber    = 2,
			    .bAlternateSetting   = 0,
			    .bNumEndpoints       = 2,
			    .bInterfaceClass     = USB_CLASS_CDC_DATA,
			    .bInterfaceSubClass  = USB_SUBCLASS_NONE,
			    .bInterfaceProtocol  = USB_PROTO_NONE,
			    .iInterface          = 4
		},
		.cdc_ep_data_rx_descriptor =
		{
		        .bLength             = sizeof(struct usb_endpoint_descriptor),
		        .bDescriptorType     = USB_DTYPE_ENDPOINT,
		        .bEndpointAddress    = USB_CDC_DATA_OUT_EP,
		        .bmAttributes        = USB_EPTYPE_BULK,
		        .wMaxPacketSize      = USB_PKT_SIZE,
		        .bInterval           = 1
		},
		.cdc_ep_data_tx_descriptor =
		{
		        .bLength             = sizeof(struct usb_endpoint_descriptor),
		        .bDescriptorType     = USB_DTYPE_ENDPOINT,
		        .bEndpointAddress    = USB_CDC_DATA_IN_EP,
		        .bmAttributes        = USB_EPTYPE_BULK,
		        .wMaxPacketSize      = USB_PKT_SIZE,
		        .bInterval           = 1
		},
};

static const struct usb_string_descriptor usb_lang_desc        = USB_ARRAY_DESC(USB_LANGID_ENG_US);
static const struct usb_string_descriptor usb_manuf_desc_en    = USB_STRING_DESC(USB_VENDOR_NAME);
static const struct usb_string_descriptor usb_prod_desc_en     = USB_STRING_DESC(USB_PRODUCT_NAME);
static const struct usb_string_descriptor usb_hid_prod_desc_en = USB_STRING_DESC(HID_PRODUCT_NAME);
static const struct usb_string_descriptor usb_cdc_prod_desc_en = USB_STRING_DESC(CDC_PRODUCT_NAME);
static const struct usb_string_descriptor usb_msc_prod_desc_en = USB_STRING_DESC(MSC_PRODUCT_NAME);
static const struct usb_string_descriptor *const usb_string_table[] =
{
		&usb_lang_desc,
		&usb_manuf_desc_en,
		&usb_prod_desc_en,
		&usb_hid_prod_desc_en,
		&usb_cdc_prod_desc_en,
		&usb_msc_prod_desc_en
};

__END_DECLS

#endif /* INCLUDE_USB_DEVICE_H_ */
