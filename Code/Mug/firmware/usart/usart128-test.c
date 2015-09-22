#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usart128.h"

/* Internal Oscillator Callibration Value from high signature byte */
#define OSCCAL_VAL	0xAC 

/* set internal oscillator calibration value asap */
void earlyInit(void) __attribute__((section(".init3"), naked));
void earlyInit(void)
{
        OSCCAL = OSCCAL_VAL;
}

int main(void)
{
	//char *msg = " There be dragons ";
	char *msg = ".";
	
	struct usart_dev *ser0 = usart_init(USART_0,
					    9600, 
					    USART_BITS_8,
					    USART_STOPBITS_1,
					    USART_PARITY_NONE,
					    0);

	struct usart_dev *ser1 = usart_init(USART_1,
					    19200, 
					    USART_BITS_8,
					    USART_STOPBITS_1,
					    USART_PARITY_NONE,
					    0);
	for (;;) {
		char r[5];
		usart_send_byte(ser0, '0');
		usart_send_byte(ser1, '1');
/*
		r[0] = *ser0->ucsra;
		r[1] = *ser0->ucsrb;
		r[2] = *ser0->ucsrc;
		usart_send_bytes(ser0, r, 3);
		r[0] = *ser1->ubrrh;
		r[1] = *ser1->ubrrl;
		usart_send_bytes(ser1, r, 2);

*/

		if (usart_data_available(ser0)) {
			char b = usart_read(ser0);
			usart_send_byte(ser0, '+');
			usart_send_byte(ser0, b);
		}

		if (usart_data_available(ser1)) {
			char b = usart_read(ser1);
			usart_send_byte(ser1, '-');
			usart_send_byte(ser1, b);
		}

		_delay_ms(10);
	}
}
