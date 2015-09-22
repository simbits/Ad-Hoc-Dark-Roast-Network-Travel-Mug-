#include "utils/watchdog.h"
#include "arch/watchdog.h"

uint8_t mcusr_mirror __attribute__((section (".noinit")));

void get_mcusr(void)
{
	mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

void watchdog_enable(void) 
{
	wdt_enable();
	wdt_reset();
}

void watchdog_disable(void) 
{
	wdt_disable();
}

/*
void watchdog_setTimeoutAction(TimeoutAction action) {}
*/
