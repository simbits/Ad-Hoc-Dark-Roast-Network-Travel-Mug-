#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdint.h>

typedef struct GPIO GPIO;
typedef uint8_t GPIOPinNum;
typedef uint8_t (*GPIOIntCallback)(GPIO *gpio, uint8_t value);

typedef enum {
	GPIO_DEFAULT,
	GPIO_TRISTATE,
	GPIO_IN_PULLUP,
	GPIO_IN,
	GPIO_LOW,
	GPIO_HIGH,
	GPIO_ANALOG,
} GPIODirection;

typedef enum {
	GPIO_INT_LOW,
	GPIO_INT_EDGE,
	GPIO_INT_FALLING,
	GPIO_INT_RISING,
} GPIOIntDirection;

typedef enum { 
	GPIO_INT0,
	GPIO_INT1,
	GPIO_INT2,
} GPIOIntNum;

typedef enum {
	PORT_A,
	PORT_B,
	PORT_C,
	PORT_D,
	PORT_MAX,
} GPIOPortName;

extern GPIOPortName analogPorts[];

GPIO *GPIOCreate(GPIOPortName name, GPIOPinNum num);
void GPIORelease(GPIO *gpio);
void GPIOSetDirection(GPIO *gpio, GPIODirection direction);
void GPIOInterruptInstall(GPIO *gpio, GPIOIntDirection direction, GPIOIntCallback handler, void *priv);
void GPIOInterruptEnable(GPIO *gpio);
void GPIOInterruptDisable(GPIO *gpio);
void GPIOInterruptChangeSense(GPIO *gpio, GPIOIntDirection direction);
uint8_t GPIOGetPinValue(GPIO *gpio);
uint8_t GPIOHasInterruptInstalled(GPIO *gpio);
void * GPIOGetPrivData(GPIO *gpio);

#endif /* __GPIO_H__ */
