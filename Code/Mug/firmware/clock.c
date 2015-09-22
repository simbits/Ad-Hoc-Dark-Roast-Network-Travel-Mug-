#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/atomic.h>

#include "clock.h"

static volatile union __ticks {
	uint32_t ticks;
	uint16_t ticks_h;
	uint16_t ticks_l;
} ticks;

ISR(TIMER0_COMPA_vect) {
	ticks.ticks++;
}

EMPTY_INTERRUPT(TIMER0_OVF_vect);

static void __init_timer(void)
{
	/* ~1ms timer @ 8MHz */
	/* @8MHz: 8 bit timer, 64 prescaler, OCR: 3D => ~1008Hz */
	/* @16Mhz: 64 prescaler, OCR: 7C => 1000Hz */
	TCCR0A = 0x42;
	TCCR0B = 0x03;
	OCR0A = 0x7C;
	TIMSK0 = 0x02;
}

void systemInitClock(void)
{
	ticks.ticks = 0;	
	__init_timer();	
}

uint32_t systemGetElapsedTime(void)
{
	uint32_t t;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		t = ticks.ticks;
	}

	return t;
}
