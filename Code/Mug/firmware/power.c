#include "hardware.h"
#include "gpio.h"
#include "power.h"

static struct __power {
	GPIO *main;
	GPIO *usbHost;
} __power;

static void __init_main_power(void)
{
	__power.main = GPIOCreate(POWER_ONOFF_PORT, POWER_ONOFF_PIN);
	GPIOSetDirection(__power.main, MAIN_POWER_OFF);
}

static void __set_main_power_on(void)
{
	GPIOSetDirection(__power.main, MAIN_POWER_ON);
}

static void __set_main_power_off(void)
{
	GPIOSetDirection(__power.main, MAIN_POWER_OFF);
}

static void __init_usb_host(void)
{
	__power.usbHost = GPIOCreate(POWER_USBHOST_PORT, POWER_USBHOST_PIN);
	GPIOSetDirection(__power.usbHost, GPIO_IN);
}

void systemInitPower(void)
{
	__init_main_power();
	__init_usb_host();
}

void systemMainPower(uint8_t state)
{
	if (state == MAIN_POWER_ON) {
		__set_main_power_on();
	} else {
		__set_main_power_off();
	}
}

uint8_t systemUsbHostConnected(void)
{
	return !GPIOGetPinValue(__power.usbHost);
}

