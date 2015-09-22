#include <avr/io.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>
#include <stdlib.h>

#include "gpio.h"

struct GPIO {
	GPIOPortName name;
	GPIOPinNum num;
	GPIODirection direction;
	
	uint8_t interruptInstalled;
	GPIOIntNum interrupt;
	GPIOIntDirection interruptDirection;

	void * priv;

	volatile uint8_t *port;
	volatile uint8_t *pin;
	volatile uint8_t *ddr;

	struct {
		uint8_t (*setDirection)(GPIO *gpio, GPIODirection direction);
		uint8_t (*getPinValue)(GPIO *gpio);
		GPIOIntCallback interruptHandler;
	} vtable;
};

typedef struct GPIOInterruptMap {
	GPIOPortName name;
	GPIOPinNum num;
	GPIOIntNum interrupt;
	GPIO *gpio;
} GPIOInterruptMap;

static GPIOInterruptMap __interruptMap[] = {
	{ PORT_D, 2, GPIO_INT0, NULL},
	{ PORT_D, 3, GPIO_INT1, NULL },
	{ PORT_B, 2, GPIO_INT2, NULL }};

GPIOPortName analogPorts[] = { PORT_A } ;

#define ANALOG_PORTS	(sizeof(analogPorts) / sizeof(GPIOPortName))
#define INTERRUPT_PINS	(sizeof(__interruptMap) / sizeof(GPIOInterruptMap))

static inline uint8_t __valid_gpio_port(GPIOPortName name)
{
	if (name >= PORT_MAX) 
		return 0;
	return 1;	
}

static inline uint8_t __is_valid_analog_port(GPIOPortName name)
{
	uint8_t i, found_match;
	
	found_match = 0;
	
	for (i=0; i<ANALOG_PORTS; i++) {
		if (name == analogPorts[i]) {
			found_match = 1;
			break;
		}
	}

	return found_match;
}

static inline int8_t __get_gpio_int_num(GPIO *gpio) 
{
	uint8_t i;

	if (!gpio)
		return -1;

	for (i=0; i<INTERRUPT_PINS; i++) {
		if (gpio->name == __interruptMap[i].name
		    && gpio->num == __interruptMap[i].num)
		return __interruptMap[i].interrupt;
	}

	return -1;
}

static inline uint8_t __interrupt_enabled(GPIOIntNum interrupt)
{
	return (EIMSK & _BV(interrupt));
}

static inline void __clear_interrupt_flag(GPIOIntNum interrupt) 
{
	EIFR |= _BV(interrupt);
}

static inline void __disable_interrupt(GPIOIntNum interrupt) 
{
	EIMSK &= ~(_BV(interrupt));
}

static inline void __enable_interrupt(GPIOIntNum interrupt) 
{
	if (!__interrupt_enabled(interrupt)) {
		__clear_interrupt_flag(interrupt);
		EIMSK |= _BV(interrupt);
	}
}

static inline void __set_interrupt_sense_control(GPIOIntNum interrupt, GPIOIntDirection direction)
{
	uint8_t iEnabled = __interrupt_enabled(interrupt);

	if (iEnabled) {
		__disable_interrupt(interrupt);
	}

	EICRA &= ~(0x03 << (interrupt * 2));
	EICRA |= (direction << (interrupt * 2));

	if (iEnabled) {
		__clear_interrupt_flag(interrupt);
		__enable_interrupt(interrupt);
	}
}

static inline uint8_t __is_valid_interrupt_gpio(GPIO *gpio)
{
	return !(__get_gpio_int_num(gpio) < 0);
}

static inline uint8_t __install_interrupt(GPIO *gpio, GPIOIntDirection direction, GPIOIntCallback handler, void *priv)
{
	int8_t interrupt;

	if (!gpio)
		return 0; 

	if (!handler)
		return 0;

	if (!__is_valid_interrupt_gpio(gpio))
		return 0;
	
	interrupt = __get_gpio_int_num(gpio);

	if (interrupt < 0)
		return 0;

	__set_interrupt_sense_control(interrupt, direction);

	gpio->interrupt = interrupt;
	gpio->interruptDirection = direction;
	gpio->vtable.interruptHandler = handler;
	gpio->interruptInstalled = 1;
	gpio->priv = priv;
	__interruptMap[interrupt].gpio = gpio;

	return 1;
}

static inline void __call_interrupt_handler(GPIOIntNum interrupt)
{
	GPIO *gpio = __interruptMap[interrupt].gpio;

	if (gpio && gpio->vtable.interruptHandler) {
		uint8_t v = gpio->vtable.getPinValue(gpio);
		gpio->vtable.interruptHandler(gpio, v);
	}
}

ISR(INT0_vect) {
	__call_interrupt_handler(GPIO_INT0);
}

ISR(INT1_vect) {
	__call_interrupt_handler(GPIO_INT1);
}

ISR(INT2_vect) {
	__call_interrupt_handler(GPIO_INT2);
}

static inline uint8_t __set_gpio_pointers(GPIO *gpio, GPIOPortName name, GPIOPinNum num)
{
	volatile uint8_t *port, *ddr, *pin;

	if (!gpio)
		return 0;

	if (!__valid_gpio_port(name))
		return 0;
	
	gpio->name = name;
	gpio->num = num;

	switch (name) {
		case PORT_A:
			port = &PORTA; ddr = &DDRA; pin = &PINA;
			break;
		case PORT_B:
			port = &PORTB; ddr = &DDRB; pin = &PINB;
			break;
		case PORT_C:
			port = &PORTC; ddr = &DDRC; pin = &PINC;
			break;
		case PORT_D:
			port = &PORTD; ddr = &DDRD; pin = &PIND;
			break;
		default:
			port = NULL; ddr = NULL; pin = NULL;
	}

	gpio->port = port;
	gpio->ddr = ddr;
	gpio->pin = pin;

	return 1;
}

static inline uint8_t __set_gpio_direction(GPIO *gpio, GPIODirection direction)
{
	uint8_t currentDirection;
	uint8_t pin;

	if (!gpio)
		return 0;
	
	currentDirection = gpio->direction;
	pin = gpio->num;

	switch (direction) {
		case GPIO_LOW:
			*(gpio->port) &= ~(_BV(pin));
			if (currentDirection == GPIO_IN_PULLUP) {
				/* Set gpio to 0 first so if going from 
				   GPIO_IN_PULLUP to GPIO_LOW 
				   through GPIO_TRISTATE */
				_NOP();
			}
			*(gpio->ddr) |= _BV(pin);
			break;
		case GPIO_HIGH:
			*(gpio->port) |= _BV(pin);
			if (currentDirection == GPIO_TRISTATE) {
				/* Set gpio to 1 first so if going from 
				   GPIO_TRISTATE to GPIO_HIGH 
				   through GPIO_IN_PULLUP */
				_NOP();
			}
			*(gpio->ddr) |= _BV(pin);
			break;
		case GPIO_IN_PULLUP:
			*(gpio->ddr) &= ~(_BV(pin));
			*(gpio->port) |= _BV(pin);
			break;
		case GPIO_ANALOG:
			if (!__is_valid_analog_port(gpio->name))
				return 0;
			
			break;
		case GPIO_DEFAULT: 
		case GPIO_TRISTATE:
		case GPIO_IN:
		default:
			*(gpio->ddr) &= ~(_BV(pin));
			*(gpio->port) &= ~(_BV(pin));
	}

	gpio->direction = direction;
	return 1;
}

static inline uint8_t __get_gpio_value(GPIO *gpio) 
{
	if (!gpio)
		return 0;

	return (*(gpio->pin) & _BV(gpio->num)) >> gpio->num;
}

static inline uint8_t __gpio_initialize(GPIO *gpio, GPIOPortName name, GPIOPinNum num)
{
	if (!gpio)
		return 0;

	if (!__set_gpio_pointers(gpio, name, num)) {
		GPIORelease(gpio);
		return 0;
	}

	gpio->vtable.setDirection = __set_gpio_direction;
	gpio->vtable.getPinValue = __get_gpio_value;

	GPIOSetDirection(gpio, GPIO_DEFAULT);

	return 1;
}

static inline GPIO *__allocate_gpio(void)
{
	GPIO *gpio = malloc(sizeof(GPIO));
	return gpio;
}

GPIO *GPIOCreate(GPIOPortName name, GPIOPinNum num)
{
	GPIO *gpio = __allocate_gpio();

	if (!gpio)
		return NULL;

	if (!__gpio_initialize(gpio, name, num))
		return NULL;

	return gpio;
}

void GPIORelease(GPIO *gpio)
{
	if (gpio) {
		if (gpio->interruptInstalled)
			GPIOInterruptDisable(gpio);
		free(gpio);
		gpio = NULL;
	}
}

void GPIOSetDirection(GPIO *gpio, GPIODirection direction)
{
	if (!gpio || !gpio->vtable.setDirection)
		return;
	gpio->vtable.setDirection(gpio, direction);
}

void GPIOInterruptInstall(GPIO *gpio, GPIOIntDirection direction, GPIOIntCallback handler, void *priv)
{
	if (!gpio)
		return;

	__install_interrupt(gpio, direction, handler, priv);
}

void GPIOInterruptEnable(GPIO *gpio)
{
	if (!gpio)
		return;

	if (!gpio->interruptInstalled)
		return;

	__enable_interrupt(gpio->interrupt);
}

void GPIOInterruptDisable(GPIO *gpio)
{
	if (!gpio)
		return;

	if (!gpio->interruptInstalled)
		return;

	__disable_interrupt(gpio->interrupt);
}

void GPIOInterruptChangeSense(GPIO *gpio, GPIOIntDirection direction)
{
	if (!gpio)
		return;

	if (!gpio->interruptInstalled)
		return;

	if (gpio->interruptDirection == direction)
		return;

	__set_interrupt_sense_control(gpio->interrupt, gpio->interruptDirection);
	gpio->interruptDirection = direction;
}

uint8_t GPIOGetPinValue(GPIO *gpio)
{
	if (!gpio || !gpio->vtable.setDirection)
		return 0;
	return gpio->vtable.getPinValue(gpio);
}

uint8_t GPIOHasInterruptInstalled(GPIO *gpio)
{
	return (gpio && gpio->interruptInstalled);
}

void * GPIOGetPrivData(GPIO *gpio)
{
	if (!gpio)
		return NULL;
	return gpio->priv;
}
