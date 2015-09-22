#include <stdlib.h>
#include <stdint.h>

#include "button.h"

struct button {
	GPIO *gpio;
	void *priv;
	buttonEventType listenForEvents;
	buttonEvent event;
	uint8_t upstate;
	uint16_t minLong;
	uint16_t maxLong;
	uint16_t minShort;
	uint16_t maxShort;

	buttonCallback handler;
};

static void __button_dispatch_event(button *aButton)
{
	if (aButton->handler && (aButton->listenForEvents & aButton->event.event)) {
		aButton->handler(aButton, &aButton->event);
	}
}

static void __button_handle_up_event(button *aButton)
{
	buttonEvent *event = &aButton->event;
	buttonEventType previousEvent = event->event;
	uint32_t now = systemGetElapsedTime();

	event->event = BUTTON_EVENT_UP;

	if (aButton->listenForEvents & (BUTTON_EVENT_PRESS_SHORT | BUTTON_EVENT_PRESS_LONG)) {
		if (previousEvent &= BUTTON_EVENT_DOWN) {
			uint32_t ticks = now - event->start;

			if (ticks >= aButton->minShort && ticks <= aButton->maxShort) {
				event->event |= BUTTON_EVENT_PRESS_SHORT;
			} else if (ticks >= aButton->minLong && ticks <= aButton->maxLong) {
				event->event |= BUTTON_EVENT_PRESS_LONG;
			}

			event->ticks = ticks;
		}
	}
	event->start = now;

	__button_dispatch_event(aButton);
}

static void __button_handle_down_event(button *aButton)
{
	buttonEvent *event = &aButton->event;
	uint32_t now = systemGetElapsedTime();

	event->event = BUTTON_EVENT_DOWN;
	event->start = now;

	__button_dispatch_event(aButton);
}

static uint8_t __gpio_change_handler(GPIO *gpio, uint8_t value)
{
	button *aButton = (button*)GPIOGetPrivData(gpio);

	if (!aButton)
		return 0;

	if (value == aButton->upstate) {
		__button_handle_up_event(aButton);
	} else {
		__button_handle_down_event(aButton);
	}

	return 1;
}

static inline void __button_clear_event(buttonEvent *event)
{
	event->event = BUTTON_EVENT_NONE;
	event->start = 0;
	event->ticks = 0;
}

static inline void __button_enable(button *aButton)
{
	aButton->event.event = BUTTON_EVENT_NONE;
	if (aButton->listenForEvents != BUTTON_EVENT_NONE) {
		GPIOInterruptEnable(aButton->gpio);
	}
}

static inline void __button_disable(button *aButton)
{
	GPIOInterruptDisable(aButton->gpio);
}

static inline uint8_t __button_prepare_gpio(button *aButton, GPIO *gpio)
{
	if (!gpio)
		return 0;

	if (GPIOHasInterruptInstalled(gpio)) {
		GPIOInterruptDisable(gpio);
	}

	GPIOInterruptInstall(gpio, GPIO_INT_EDGE, __gpio_change_handler, aButton);

	/* do not yet enable gpio interrupt here */

	return 1;
}

static inline uint8_t __button_initialize(button *aButton, GPIO *gpio, buttonCallback handler, void *priv)
{
	if (!handler)
		return 0;

	if (!__button_prepare_gpio(aButton, gpio))
		return 0;

	aButton->gpio = gpio;
	aButton->priv = priv;
	aButton->upstate = 0;
	aButton->handler = handler;
	aButton->listenForEvents = BUTTON_EVENT_NONE;
	
	__button_clear_event(&aButton->event);

	return 1;
}

static inline button *__allocate_button(void)
{
	button *aButton = malloc(sizeof(button));
	return aButton;
}

button *buttonCreate(GPIO *gpio, buttonCallback callback, void *priv)
{
	button *aButton = NULL;

	if (!(aButton = __allocate_button()))
		return NULL;

	if (!__button_initialize(aButton, gpio, callback, priv)) {
		free(aButton);
		return NULL;
	}

	return aButton;
}

void buttonRelease(button *aButton, uint8_t releaseGPIO) 
{
	if (aButton) {
		if (releaseGPIO) {
			GPIORelease(aButton->gpio);
		}
		free(aButton);
		aButton = NULL;
	}
}

void buttonListenForEvents(button *aButton, buttonEventType events) 
{
	if (!aButton)
		return;

	__button_disable(aButton);

	aButton->listenForEvents = events;

	if (aButton->listenForEvents != BUTTON_EVENT_NONE) {
		__button_enable(aButton);
	}
}

void buttonStopListeningForEvents(button *aButton, buttonEventType events)
{
	if (!aButton)
		return;

	__button_disable(aButton);

	aButton->listenForEvents &= ~events;

	if (aButton->listenForEvents != BUTTON_EVENT_NONE) {
		__button_enable(aButton);
	}
}

void buttonSetShortPressDelay(button *aButton, uint16_t min, uint16_t max) 
{
	if (!aButton)
		return;

	if (min > max)
		min = 0;

	aButton->minShort = min;
	aButton->maxShort = max;
}

void buttonSetLongPressDelay(button *aButton, uint16_t min, uint16_t max) 
{

	if (!aButton)
		return;
	
	if (min > max)
		max = 0xffff;
	
	aButton->minLong = min;
	aButton->maxLong = max;
}

void buttonUpState(button *aButton, GPIODirection state)
{
	if (!aButton)
		return;
	if (state == GPIO_HIGH) {
		aButton->upstate = 1;
	} else {
		aButton->upstate = 0;
	}
}
