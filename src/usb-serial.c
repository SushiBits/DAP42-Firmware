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
#include <stdbool.h>
#include <sys/cdefs.h>

#include "usb-device.h"

RING_BUFFER_INIT_STATIC(usb_cdc_tx_buffer, 128);
RING_BUFFER_INIT_STATIC(usb_cdc_rx_buffer, 128);

static struct usb_cdc_line_coding usb_cdc_line_coding =
{ .dwDTERate = 115200, .bCharFormat = USB_CDC_1_STOP_BITS, .bParityType =
USB_CDC_NO_PARITY, .bDataBits = 8, };

static void usb_cdc_set_usart(void);

void usb_cdc_init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	__DSB();
	NVIC_EnableIRQ(USART2_IRQn);
	
	usb_cdc_line_coding.dwDTERate = 115200;
	usb_cdc_line_coding.bCharFormat = USB_CDC_1_STOP_BITS;
	usb_cdc_line_coding.bParityType = USB_CDC_NO_PARITY;
	usb_cdc_line_coding.bDataBits = 8;
	
	usb_cdc_set_usart();
	
	pinMode(0x02, AFIO_PU(GPIO_MODE_AF0));
	pinMode(0x03, AFIO_PU(GPIO_MODE_AF0));
}

void usb_cdc_deinit(void)
{
	pinMode(0x02, INPUT);
	pinMode(0x03, INPUT);
	
	NVIC_DisableIRQ(USART2_IRQn);
	RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
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
		memmove(&usb_cdc_line_coding, req->data, sizeof(usb_cdc_line_coding));
		
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
			if (usb_cdc_line_coding.bDataBits > 8)
				usb_cdc_line_coding.bDataBits = 8;
		}
		else
		{
			if (usb_cdc_line_coding.bDataBits < 6)
				usb_cdc_line_coding.bDataBits = 6;
			if (usb_cdc_line_coding.bDataBits > 8)
				usb_cdc_line_coding.bDataBits = 8;
		}
		
		usb_cdc_set_usart();
		
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

static void usb_cdc_set_usart(void)
{
	// Reset the USART peripheral.
	RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
	__DSB();
	RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;
	__DSB();
	
	ring_buffer_drain(usb_cdc_tx_buffer);
	ring_buffer_drain(usb_cdc_rx_buffer);

	// Set the data bits register.
	uint8_t data_bits = 8;
	if (usb_cdc_line_coding.bParityType == USB_CDC_NO_PARITY)
		data_bits = usb_cdc_line_coding.bDataBits;
	else
		data_bits = usb_cdc_line_coding.bDataBits + 1;
	switch (data_bits)
	{
	case 7:
		USART2->CR1 = USART_CR1_M1;
		break;

	case 8:
		USART2->CR1 = 0;
		break;

	case 9:
		USART2->CR1 = USART_CR1_M0;
		break;

	default:
		USART2->CR1 = 0;
		break;
	}

	// Set the parity bits register
	switch (usb_cdc_line_coding.bParityType)
	{
	case USB_CDC_ODD_PARITY:
		USART2->CR1 |= USART_CR1_PCE | USART_CR1_PS;
		break;

	case USB_CDC_EVEN_PARITY:
		USART2->CR1 |= USART_CR1_PCE;
		break;

	default:
		break;
	}

	// Set the stop bits register
	switch (usb_cdc_line_coding.bCharFormat)
	{
	case USB_CDC_1_STOP_BITS:
		USART2->CR2 = 0x0 << USART_CR2_STOP_Pos;
		break;

	case USB_CDC_1_5_STOP_BITS:
		USART2->CR2 = 0x3 << USART_CR2_STOP_Pos;
		break;

	case USB_CDC_2_STOP_BITS:
		USART2->CR2 = 0x2 << USART_CR2_STOP_Pos;
		break;

	default:
		USART2->CR2 = 0x0 << USART_CR2_STOP_Pos;
		break;
	}

	// Set the baudrate
	USART2->BRR = SystemCoreClock / usb_cdc_line_coding.dwDTERate;

	USART2->CR1 |= USART_CR1_RXNEIE | USART_CR1_TE | USART_CR1_RE;
	__DSB();

	USART2->CR1 |= USART_CR1_UE;
}

void USART2_IRQHandler()
{
	if (USART2->ISR & USART_ISR_RXNE)
	{
		if (USART2->ISR & USART_ISR_PE)
			USART2->ICR = USART_ICR_PECF;

		uint8_t ch = 0;
		switch (usb_cdc_line_coding.bParityType)
		{
		case USB_CDC_MARK_PARITY:
		case USB_CDC_SPACE_PARITY:
		case USB_CDC_ODD_PARITY:
		case USB_CDC_EVEN_PARITY:
			ch = USART2->RDR & ((1 << usb_cdc_line_coding.bDataBits) - 1);
			break;

		default:
			ch = USART2->RDR;
		}

		ring_buffer_putchar(usb_cdc_rx_buffer, ch);
	}

	if (USART2->ISR & USART_ISR_TXE)
	{
		int ch = ring_buffer_getchar(usb_cdc_tx_buffer);
		if (ch < 0)
		{
			USART2->CR1 &= ~USART_CR1_TXEIE;
		}
		else
		{
			ch &= (1 << usb_cdc_line_coding.bDataBits) - 1;
			if (usb_cdc_line_coding.bParityType == USB_CDC_MARK_PARITY)
			{
				ch |= (1 << usb_cdc_line_coding.bDataBits);
			}

			USART2->TDR = ch;
		}
	}
}

void usb_cdc_handle(usbd_device *dev, uint8_t event, uint8_t ep)
{
	int val;
	switch (event)
	{
	case usbd_evt_eptx:
		for (off_t count = 0; count < USB_PKT_SIZE; count++)
		{
			int rv = ring_buffer_peekchar(usb_cdc_rx_buffer);
			if (rv > 0)
			{
				uint8_t ch = rv;
				val = usbd_ep_write(dev, USB_CDC_DATA_IN_EP, &ch, 1);
				if (val > 0)
					ring_buffer_getchar(usb_cdc_rx_buffer);
				else
					break;
			}
			else
			{
				usbd_ep_write(dev, USB_CDC_DATA_IN_EP, NULL, 0);
				break;
			}
		}
		break;

	case usbd_evt_eprx:
		for (;;)
		{
			uint8_t ch = 0;
			val = usbd_ep_read(dev, USB_CDC_DATA_OUT_EP, &ch, 1);
			if (val > 0)
			{
				ring_buffer_putchar(usb_cdc_tx_buffer, ch);
				USART2->CR1 |= USART_CR1_TXEIE;
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
