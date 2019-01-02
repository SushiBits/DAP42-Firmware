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
#include <stdbool.h>
#include <sys/cdefs.h>

#include "usb-device.h"
#include "time.h"

#define BUFFER_SIZE 256
static volatile uint8_t tx_buffer[BUFFER_SIZE];
static volatile uint8_t tx_head = 0;
static volatile uint8_t tx_tail = 0;
static volatile uint8_t rx_buffer[BUFFER_SIZE];
static volatile uint8_t rx_head = 0;
static volatile uint8_t rx_tail = 0;
#define INCREASE(var) var = (var + 1) % BUFFER_SIZE

static volatile uint32_t txled_time;
static volatile uint32_t rxled_time;
static volatile uint32_t reset_time;

static struct usb_cdc_line_coding cdc_line =
{ .dwDTERate = 115200, .bCharFormat = USB_CDC_1_STOP_BITS, .bParityType =
USB_CDC_NO_PARITY, .bDataBits = 8, };

static void cdc_handle_line_change(void);
static void cdc_handle_status_bits(uint16_t bits);

void usb_cdc_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
	RCC->APB1RSTR |= RCC_APB1RSTR_USART2RST;
	__DSB();
	RCC->APB1RSTR &= ~RCC_APB1RSTR_USART2RST;

	SET_FIELD(GPIOA->ODR, 0x0033, 0x0033);
	SET_FIELD(GPIOA->MODER, 0x03c00fff, 0x028005a5);
	SET_FIELD(GPIOA->OSPEEDR, 0x03c00fff, 0x03c00000);
	SET_FIELD(GPIOA->OTYPER, 0x0033, 0x0033);
	SET_FIELD(GPIOA->AFR[0], 0x0000ff00, 0x00001100);
	SET_FIELD(GPIOA->AFR[1], 0x000ff000, 0x00077000);

	tx_head = 0;
	tx_tail = 0;
	rx_head = 0;
	rx_tail = 0;

	cdc_handle_line_change();
	USART2->CR1 |= USART_CR1_RE | USART_CR1_TE | USART_CR1_RXNEIE;
	NVIC_EnableIRQ(USART2_IRQn);
	USART2->CR1 |= USART_CR1_UE;
}

void usb_cdc_deinit(void)
{
	NVIC_DisableIRQ(USART2_IRQn);
	RCC->APB1ENR &= ~RCC_APB1ENR_USART2EN;
}

usbd_respond usb_cdc_control(usbd_device *dev, usbd_ctlreq *req,
		usbd_rqc_callback *callback)
{
    if (((USB_REQ_RECIPIENT | USB_REQ_TYPE) & req->bmRequestType) != (USB_REQ_INTERFACE | USB_REQ_CLASS))
    	return usbd_fail;

    switch (req->bRequest) {
    case USB_CDC_SET_CONTROL_LINE_STATE:
    	cdc_handle_status_bits(req->wValue);
        return usbd_ack;

    case USB_CDC_SET_LINE_CODING:
    {
    	struct usb_cdc_line_coding *new_line = (void *)req->data;

    	if (new_line->dwDTERate == 1200)
    	{
    		GPIOA->ODR &= ~0x0030;
    		reset_time = millis();
    	}

    	if (memcmp(&cdc_line, new_line, sizeof(cdc_line)))
    	{
    		memmove(&cdc_line, new_line, sizeof(cdc_line));
    		cdc_handle_line_change();
    	}

        return usbd_ack;
    }

    case USB_CDC_GET_LINE_CODING:
        dev->status.data_ptr = &cdc_line;
        dev->status.data_count = sizeof(cdc_line);
        return usbd_ack;

    default:
        return usbd_fail;
    }
}

void usb_cdc_handle(usbd_device *dev, uint8_t event, uint8_t ep)
{
	uint8_t data = 0;
    switch (event) {
    case usbd_evt_eptx:
        if (rx_head != rx_tail)
        {
        	data = rx_buffer[rx_tail];
        	INCREASE(rx_tail);
        	usbd_ep_write(dev, ep, &data, sizeof(data));
        }
        break;

    case usbd_evt_eprx:
    	while (usbd_ep_read(dev, ep, &data, sizeof(data)) > 0)
    	{
    		tx_buffer[tx_head] = data;
    		INCREASE(tx_head);
    		if (tx_head == tx_tail)
    			INCREASE(tx_tail);
    		USART2->CR1 |= USART_CR1_TXEIE;
    	}
    	break;

    default:
        break;
    }
}

#define CLAMP(var, lower, upper) \
	do { \
		typeof(var) *v = &(var), l = (lower), u = (upper); \
		if (*v < l) *v = l; \
		if (*v > u) *v = u; \
	} while (0)

static void cdc_handle_line_change(void)
{
	// Validate the line code
	uint8_t data_bits;

	CLAMP(cdc_line.dwDTERate, SystemCoreClock / UINT16_MAX, SystemCoreClock / 16);

	if (cdc_line.bParityType == USB_CDC_NO_PARITY)
	{
		CLAMP(cdc_line.bDataBits, 7, 8);
		data_bits = cdc_line.bDataBits;
	}
	else
	{
		CLAMP(cdc_line.bDataBits, 6, 8);
		data_bits = cdc_line.bDataBits + 1;
	}

	if (cdc_line.bCharFormat > 0b11)
		cdc_line.bCharFormat = 0b11;

	bool reenable = !!(USART2->CR1 & USART_CR1_UE);
	USART2->CR1 &= ~USART_CR1_UE;

	static const uint32_t stop_map[] = {0b00 << USART_CR2_STOP_Pos, 0b11 << USART_CR2_STOP_Pos, 0b10 << USART_CR2_STOP_Pos};
	SET_FIELD(USART2->CR1, USART_CR1_PS | USART_CR1_PCE | USART_CR1_M0 | USART_CR1_M1,
			((cdc_line.bParityType == USB_CDC_ODD_PARITY) ? USART_CR1_PS : 0) |
			((cdc_line.bParityType == USB_CDC_ODD_PARITY || cdc_line.bParityType == USB_CDC_EVEN_PARITY) ? USART_CR1_PCE : 0) |
			((data_bits == 9) ? USART_CR1_M0 : 0) |
			((data_bits == 7) ? USART_CR1_M1 : 0));
	SET_FIELD(USART2->CR2, USART_CR2_STOP_Msk, stop_map[cdc_line.bCharFormat]);
	USART2->BRR = SystemCoreClock / cdc_line.dwDTERate;

	if (reenable)
		USART2->CR1 |= USART_CR1_UE;
}

static uint16_t cdc_status_bits = 0;

static void cdc_handle_status_bits(uint16_t bits)
{
	if (bits != cdc_status_bits)
	{
		if (!(cdc_status_bits & 0x0001) && (bits & 0x0001))
		{
			SET_FIELD(GPIOA->ODR, 0x0030, 0x0020);
			reset_time = millis();
		}
		cdc_status_bits = bits;
	}
}

void USART2_IRQHandler(void)
{
	uint32_t now = millis();

	if (USART2->ISR & USART_ISR_RXNE)
	{
		uint16_t data = USART2->RDR;
		bool okay = true;

		switch (cdc_line.bParityType)
		{
		case USB_CDC_ODD_PARITY:
		case USB_CDC_EVEN_PARITY:
		case USB_CDC_NO_PARITY:
			okay = true;
			break;

		case USB_CDC_MARK_PARITY:
			okay = (data & 0b00000001) == 1;
			data = data >> 1;
			break;

		case USB_CDC_SPACE_PARITY:
			okay = (data & 0b00000001) == 0;
			data = data >> 1;
			break;
		}

		if (okay)
		{
			rx_buffer[rx_head] = data;
			INCREASE(rx_head);
			if (rx_tail == rx_head)
				INCREASE(rx_tail);
			GPIOA->ODR &= ~0x0001;
			txled_time = now;
		}
	}

	if (USART2->ISR & USART_ISR_TXE)
	{
		if (tx_tail != tx_head)
		{
			uint16_t data = tx_buffer[tx_tail];
			INCREASE(tx_tail);

			switch (cdc_line.bParityType)
			{
			case USB_CDC_ODD_PARITY:
			case USB_CDC_EVEN_PARITY:
			case USB_CDC_NO_PARITY:
				USART2->TDR = data;
				break;

			case USB_CDC_MARK_PARITY:
				USART2->TDR = data << 1 | 1;
				break;

			case USB_CDC_SPACE_PARITY:
				USART2->TDR = data << 1 | 0;
				break;
			}

			GPIOA->ODR &= ~0x0002;
			rxled_time = now;
		}
		else
		{
			USART2->CR1 &= ~USART_CR1_TXEIE;
		}
	}
}

void usb_cdc_update(void)
{
	uint32_t now = millis();

	if (rx_head != rx_tail)
	{
		usbd_ep_write(&usbd, USB_CDC_DATA_OUT_EP, NULL, 0);
	}

	if (!(GPIOA->ODR & 0x0001) && (now - txled_time > 100))
	{
		GPIOA->ODR |= 0x0001;
	}

	else if (!(GPIOA->ODR & 0x0002) && (now - rxled_time > 100))
	{
		GPIOA->ODR |= 0x0002;
	}

	if (!(GPIOA->ODR & 0x0010) && (now - reset_time > 50))
		GPIOA->ODR |= 0x0010;
}
