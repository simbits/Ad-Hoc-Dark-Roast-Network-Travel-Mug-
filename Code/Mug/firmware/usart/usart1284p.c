/*
  usart128.c - usart interface for ATMega128
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


#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>

#include "usart128.h"

#define RXEN	RXEN0
#define TXEN	TXEN0
#define UCSZ1	UCSZ01
#define UCSZ2	UCSZ02
#define UMSEL	UMSEL00
#define UPM0	UPM00
#define USBS	USBS0
#define UDRE	UDRE0
#define RXC	RXC0

static struct usart_dev usart_0;
static struct usart_dev usart_1;

static inline void receive_byte(struct usart_dev * usart)
{
	char byte = *usart->udr;

	int i = (usart->rx_head + 1) % RX_BUFFER_SIZE;
	if (i != usart->rx_tail) {
		usart->rx_buffer[usart->rx_head] = byte;
		usart->rx_head = i;
	}
}

static inline void _usart_init(struct usart_dev *usart, 
			  volatile uint8_t *ucsra,
			  volatile uint8_t *ucsrb,
			  volatile uint8_t *ucsrc,
			  volatile uint8_t *ubrrl,
			  volatile uint8_t *ubrrh,
			  volatile uint8_t *udr,
			  uint8_t flags)
{
	usart->ucsra = ucsra;
	usart->ucsrb = ucsrb;
	usart->ucsrc = ucsrc;
	usart->ubrrl = ubrrl;
	usart->ubrrh = ubrrh;
	usart->udr = udr;
	usart->cts = NULL;
	usart->rts = NULL;
	usart->flags = flags;
	usart->rx_head = 0;
	usart->rx_tail = 0;
}

struct usart_dev* usart_init(uint8_t usart,
			     uint32_t baudrate, 
			     uint8_t bits, 
			     uint8_t sbits, 
			     uint8_t parity, 
			     uint8_t flags)
{
	uint8_t ucsra = 0x20;
	uint8_t ucsrb = 0x00;
	uint8_t ucsrc = 0x06;
	uint16_t ubrr = 0x0000;

	struct usart_dev * dev;

	if (usart == USART_0) {
		dev = &usart_0;
		_usart_init(dev, &UCSR0A, &UCSR0B, &UCSR0C, &UBRR0L, &UBRR0H, &UDR0, flags);
	} else {
		dev = &usart_1;
		_usart_init(dev, &UCSR1A, &UCSR1B, &UCSR1C, &UBRR1L, &UBRR1H, &UDR1, flags);
	}

	/*Set baud rate */
	//ubrr = USART_UBRR_VAL(baudrate) - 3;
	ubrr = USART_UBRR_VAL(baudrate);

	/*Enable receiver and transmitter */
	if (!(flags & USART_NO_RX))
		ucsrb |= (1 << RXEN);
	if (!(flags & USART_NO_TX))
		ucsrb |= (1 << TXEN);
	
	/* character bits */
	if (bits == USART_BITS_9)
		ucsrb |= (1 << UCSZ2);
	ucsrc &= (bits << 1) | 0xF9;
	
	/* transmission mode */
	if (flags & USART_SYNC)
		ucsrc |= (1 << UMSEL);

	/* parity */
	ucsrc |= (parity << UPM0);

	/* stop bits */
	ucsrc |= (sbits << USBS);

#if defined(USART_USE_CTS)
	/* cts, rts */
	if (flags & USART_CTS) {
		if (usart == USART_0) {
			DDRE &= ~(1 << DDE3); /* input */
			PORTE &= ~(1 << PE3); /* no int pull-up */
			dev->cts = &PINE;
			dev->cts_pin = PINE3;
		} else {
			DDRD &= ~(1 << DDD4); /* input */
			PORTD &= ~(1 << PD4); /* no int pull-up */
			usart_1.cts = &PORTD;
			usart_1.cts_pin = PIND4;
		}
	}
#endif
#if defined(USART_USE_RTS)
	if (flags & USART_RTS) {
		if (usart == USART_0) {
			DDRE |= (1 << DDE2); /* output */
			PORTE &= ~(1 << PE2); /* low */
			dev->rts = &PORTE;
			dev->rts_pin = PORTE2;
		} else {
			/** ?? **/
		}
	}
		
	/* interrupts */
#endif
#if defined(USART_USE_ISR)
	ucsrb |= (1 << RXCIE0);
#endif

	cli();
	*(dev->ucsrc) = ucsrc;
	*(dev->ucsrb) = ucsrb;
	*(dev->ucsra) = ucsra;
	*(dev->ubrrh) = (uint8_t)(ubrr >> 8);
	sei();
	*(dev->ubrrl) = (uint8_t)ubrr;

	return dev;
}

#ifdef USART_USE_ISR
ISR(USART0_RX_vect)
{
	receive_byte(&usart_0);
}

ISR(USART1_RX_vect)
{
	receive_byte(&usart_1);
}
#endif

void usart_send_byte(struct usart_dev *dev, char data)
{
	if (dev->flags & USART_CTS)
		while (*dev->cts & (1 << dev->cts_pin))
			;
	while (!(*dev->ucsra & (1 << UDRE)))
		;
	*dev->udr = data;
}

void usart_send_bytes(struct usart_dev *dev, const char *bytes, uint16_t len)
{
	uint16_t i;
	for (i=0; i<len; i++)
		usart_send_byte(dev, bytes[i]);
}

void usart_send_string(struct usart_dev *dev, const char *bytes)
{
	char c;
	while ((c = *bytes++))
		usart_send_byte(dev, c);
}

#ifdef USART_USE_ISR
uint8_t usart_data_available(struct usart_dev *dev)
{
	return (RX_BUFFER_SIZE + dev->rx_head - dev->rx_tail) % RX_BUFFER_SIZE;
}

char usart_read(struct usart_dev *dev)
{
	// if the head isn't ahead of the tail, we don't have any characters
	if (dev->rx_head == dev->rx_tail) {
		return -1;
	} else {
		char c = dev->rx_buffer[dev->rx_tail];
		dev->rx_tail = (dev->rx_tail + 1) % RX_BUFFER_SIZE;
		return c;
	}
}

char usart_read_wait(struct usart_dev *dev)
{
	while (!(usart_data_available(dev)))
		;
	return usart_read(dev);
}

#else

char usart_read(struct usart_dev *dev)
{
	if (!(*dev->ucsra & (1<<RXC)))
		return 0;
	return *dev->udr;
}

char usart_read_wait(struct usart_dev *dev)
{
	while (!(usart_data_available(dev))) 
		;
	return *dev->udr;
}

#endif /* USART_USE_ISR */
