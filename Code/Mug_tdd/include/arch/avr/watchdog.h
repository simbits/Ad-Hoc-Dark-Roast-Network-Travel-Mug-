#ifndef __ARCH_AVR_WATCHDOG__
#define __ARCH_AVR_WATCHDOG__

#include <stdint.h>
#include <avr/wdt.h>

void get_mcusr(void) \
	__attribute__((naked)) \
	__attribute__((section(".init3")));

#endif /* __ARCH_AVR_WATCHDOG__ */
