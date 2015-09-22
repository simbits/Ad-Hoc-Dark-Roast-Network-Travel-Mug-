#ifndef __EVENT_H__
#define __EVENT_H__

#include "gpio.h"
#include "clock.h"

typedef struct button button;

typedef enum {
	BUTTON_EVENT_NONE		= 0x00,
	BUTTON_EVENT_PRESS_LONG 	= 0x01,
	BUTTON_EVENT_PRESS_SHORT 	= 0x02,
	BUTTON_EVENT_DOWN 		= 0x04,
	BUTTON_EVENT_UP 		= 0x08,
	BUTTON_EVENT_ALL		= 0xff,
} buttonEventType;

typedef struct buttonEvent {
	buttonEventType event;
	uint32_t start;
	uint32_t ticks;
} buttonEvent;

typedef void (*buttonCallback)(button *aButton, buttonEvent *event);

button *buttonCreate(GPIO *gpio, buttonCallback callback, void *priv);
void buttonRelease(button *aButton, uint8_t releaseGPIO);
void buttonListenForEvents(button *aButton, buttonEventType events);
void buttonStopListeningForEvents(button *aButton, buttonEventType events);
void buttonSetShortPressDelay(button *aButton, uint16_t min, uint16_t max);
void buttonSetLongPressDelay(button *aButton, uint16_t min, uint16_t max);
void buttonUpState(button *aButton, GPIODirection state);

#endif /* __EVENT_H__ */
