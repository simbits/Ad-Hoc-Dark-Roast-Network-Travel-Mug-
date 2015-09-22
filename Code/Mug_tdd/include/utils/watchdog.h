#ifndef __UTILS_WATCHDOG_H__
#define __UTILS_WATCHDOG_H__

typedef enum { 
	WATCHDOG_RESET, 
	WATCHDOG_INTERRUPT 
} TimeoutAction;

void watchdog_enable(void);
void watchdog_disable(void);
void watchdog_setTimeoutAction(TimeoutAction action);

#endif /* __UTILS_WATCHDOG_H__ */
