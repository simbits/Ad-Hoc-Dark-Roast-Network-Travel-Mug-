/*
  usart128.h - usart interface for ATMega128
  Copyright (c) 2009 Simon de Bakker (simon@v2.nl).  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

/* send/receive routines are ftm. based on wiring serial. */

#ifndef __USART128_H__
#define __USART128_H__

#include <avr/io.h>

#define USART_0	0
#define USART_1	1

#define USART_NO_RX	0x01
#define USART_NO_TX	0x02
#define USART_CTS	0x04
#define USART_RTS	0x08
#define USART_SYNC	0x10

#define USART_STOPBITS_1	0x00
#define USART_STOPBITS_2	0x01

#define USART_PARITY_NONE	0x00
#define USART_PARITY_EVEN	0x02
#define USART_PARITY_ODD	0x03

#define USART_BITS_5	0x00
#define USART_BITS_6	0x01
#define USART_BITS_7	0x02
#define USART_BITS_8	0x03
#define USART_BITS_9	0x07

#define RX_BUFFER_SIZE	128

#define USART_UBRR_VAL(__x) ((F_CPU + (__x) * 8L) / ((__x) * 16L) - 1)
//#define USART_UBRR_VAL(__x) (F_CPU / 16L / (__x) - 1)

//#define USART_USE_ISR	1

struct usart_dev {
	volatile uint8_t *ucsra;
	volatile uint8_t *ucsrb;
	volatile uint8_t *ucsrc;
	volatile uint8_t *ubrrl;
	volatile uint8_t *ubrrh;
	volatile uint8_t *udr;
	volatile uint8_t *cts;
	volatile uint8_t *rts;
	uint8_t cts_pin;
	uint8_t rts_pin;
	uint8_t flags;
	volatile uint8_t rx_head;
	volatile uint8_t rx_tail;
	unsigned char rx_buffer[RX_BUFFER_SIZE];
};

struct usart_dev* usart_init(uint8_t usart, 
			     uint32_t baudrate, 
			     uint8_t bits, 
			     uint8_t sbits, 
			     uint8_t parity, 
			     uint8_t flags);
void usart_send_byte(struct usart_dev *dev, char data);
void usart_send_bytes(struct usart_dev *dev, const char *bytes, uint16_t len);
void usart_send_string(struct usart_dev *dev, const char *bytes);
#ifdef USART_USE_ISR
uint8_t usart_data_available(struct usart_dev *dev);
#else
#define usart_data_available(__dev) (*(__dev)->ucsra & (1 << RXC0))
#endif
char usart_read(struct usart_dev *dev);
char usart_read_wait(struct usart_dev *dev);

#endif /* __USART128_H__ */
