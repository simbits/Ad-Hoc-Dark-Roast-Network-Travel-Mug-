#ifndef __TIMER_H__
#define __TIMER_H__

#include <stdint.h>

void systemInitClock(void);
uint32_t systemGetElapsedTime(void);

#endif /* __TIMER_H__ */
