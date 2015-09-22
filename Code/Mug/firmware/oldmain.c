#include <avr/io.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <string.h>
#include <stdio.h>
#include "hardware.h"
#include "gpio.h"
#include "power.h"
#include "clock.h"
#include "button.h"
#include "usart/usart128.h"
#include "oled/oled.h"
#include "xbee/xbee.h"

#ifndef true
#define true 1
#define false 0
#endif

typedef uint8_t bool;

bool volatile button_pressed_long = false;
bool volatile button_pressed_short = false;

uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));

typedef struct MugConfig {
	char name[16];
	char id[5];
	char pan[5];	
	char channel[3];
} MugConfig;

typedef enum {
	TTY_OLED,
	TTY_TERMINAL,
} ConsoleType;

static struct {
	XBee * xbee;
	OLED * oled;
	struct usart_dev *tty_console;
	struct usart_dev *tty_xbee;
	ConsoleType consoleType;
} Mug;

MugConfig mugConfigEep EEMEM = {
	.name = "foobar",
	.id = "1111",
	.pan = "1111",
	.channel = "1A"
};

static MugConfig mugConfig;

void get_mcusr(void)
{
	mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}

static uint8_t __init_hardware(void)
{	
	cli();

	systemInitClock();
	systemInitPower();

	sei();

	return 1;
}

static void __read_config(void)
{
	eeprom_busy_wait();
	eeprom_read_block(&mugConfig, &mugConfigEep, sizeof(MugConfig));
}

/* suppress unused warning */
static void __write_config(void) __attribute__((unused));
void __write_config(void) 
{
	eeprom_busy_wait();
	eeprom_write_block(&mugConfig, &mugConfigEep, sizeof(MugConfig));
} 

static void __oled_send_msg(const char *msg)
{
	OLEDSendCommand(Mug.oled, OLED_CMD_SET_MESSAGE, msg);
	OLEDSendCommand(Mug.oled, OLED_CMD_DISP_MESSAGE, 0);
}

void __button_event_handler(button *aButton, buttonEvent *event)
{
	if (event->event & BUTTON_EVENT_PRESS_LONG)
		button_pressed_long = true;
	if (event->event & BUTTON_EVENT_PRESS_SHORT)
		button_pressed_short = true;
}

int main(int argc, char *argv[])
{
	__init_hardware();

	GPIO *gpioConsoleOutputEnable;
	GPIO *gpioConsoleSwitch;
	GPIO *gpioButton;
	button *mainButton;


	/* PD[45] as output for console/oled output enable */
	gpioConsoleSwitch = GPIOCreate(CONSOLE_SWITCH_PORT, CONSOLE_SWITCH_PIN);
	gpioConsoleOutputEnable = GPIOCreate(CONSOLE_ENABLE_PORT, CONSOLE_ENABLE_PIN);
	gpioButton = GPIOCreate(BUTTON_PORT, BUTTON_PIN);
	GPIOSetDirection(gpioButton, GPIO_IN_PULLUP);

	Mug.tty_xbee = usart_init(ZIGBEE_UART,
			    ZIGBEE_BAUDRATE, 
			    USART_BITS_8,
			    USART_STOPBITS_1,
			    USART_PARITY_NONE,
			    0);

	Mug.tty_console = usart_init(OLED_CONSOLE_UART,
			       OLED_CONSOLE_BAUDRATE, 
			       USART_BITS_8,
			       USART_STOPBITS_1,
			       USART_PARITY_NONE,
			       0);

	Mug.oled = OLEDCreate(Mug.tty_console);
	Mug.xbee = XBeeCreate(Mug.tty_xbee);
	
	/* PD5 low: output enable */
	GPIOSetDirection(gpioConsoleOutputEnable, GPIO_LOW);

	/* PD4 high: console enable */
	GPIOSetDirection(gpioConsoleSwitch, GPIO_HIGH);
	
	/* take oled out of reset */
	OLEDEnable(Mug.oled);

	/* switch to oled */
	GPIOSetDirection(gpioConsoleSwitch, GPIO_LOW);

	/* settle oled */
	_delay_ms(1000);
	_delay_ms(1000);
	_delay_ms(1000);

	/* keep power on */
	systemMainPower(MAIN_POWER_ON);

	mainButton = buttonCreate(gpioButton, __button_event_handler, NULL);
	buttonUpState(mainButton, GPIO_LOW);
	buttonSetShortPressDelay(mainButton, 10, 1000);
	buttonSetLongPressDelay(mainButton, 3000, 5000);
	buttonListenForEvents(mainButton, BUTTON_EVENT_PRESS_SHORT | BUTTON_EVENT_PRESS_LONG);

	OLEDSendCommand(Mug.oled, OLED_CMD_POWER_LEVEL, OLED_POWER_ICON_L4);
	__oled_send_msg(OLEDRetrieveMessage(Mug.oled));
	OLEDSendCommand(Mug.oled, OLED_CMD_SHOW, OLED_ICON_POWER, OLED_ICON_SHOW);

	__oled_send_msg("Loading  Config");

	/* read configuration from EEPROM */
	__read_config();

	__oled_send_msg(mugConfig.name);
	__oled_send_msg(mugConfig.id);
	__oled_send_msg(mugConfig.pan);
	__oled_send_msg(mugConfig.channel);
	
	__oled_send_msg("Configuring ZigBee");

	/* enable and configure ZigBee */
	XBeeLeaveSleepMode(Mug.xbee);
	XBeeEnterCommandMode(Mug.xbee);

	XBeeSetDestination(Mug.xbee, "0000", "FFFF");
	XBeeSetMyId(Mug.xbee, mugConfig.id);
	XBeeSetPanId(Mug.xbee, mugConfig.pan);
	XBeeSetChannel(Mug.xbee, mugConfig.channel);
	XBeeSetCoordinatorMode(Mug.xbee, XBEE_END_DEVICE);
	XBeeSetEndDeviceAssociation(Mug.xbee, EDA_NO_ASSOCIATION);
	
	XBeeLeaveCommandMode(Mug.xbee);
	
	__oled_send_msg(XBeeRetrieveMessage(Mug.xbee));

	

	/* switch to console */
	//GPIOSetDirection(gpioConsoleSwitch, GPIO_HIGH);

#if 0
	oled_write_message(CONSOLE, "Console enabled");
	_delay_ms(1000);

	oled_write_message(CONSOLE, "Testing power off: ");
	_delay_ms(1000);
	oled_write_message(CONSOLE, "NOW!");
	powerSetMain(MAIN_POWER_OFF);
	_delay_ms(2000);
	oled_write_message(CONSOLE, "Power back on: ");
	_delay_ms(1000);
	oled_write_message(CONSOLE, "NOW!");
	powerSetMain(MAIN_POWER_ON);
	_delay_ms(2000);
#endif


	do {
		static uint32_t prev_time = 0;
		uint32_t time = get_elapsed_time();

		if (time - prev_time > 10000) {
			__oled_send_msg(".");
			prev_time = time;
		}

		if (button_pressed_long) {
			button_pressed_long = false;
			__oled_send_msg("powering down");
			_delay_ms(1000);
			systemMainPower(MAIN_POWER_OFF);
		}

		if (button_pressed_short) {
			button_pressed_short = false;
			__oled_send_msg("short");
		}


		static uint8_t usbConnectedState = 0;
		if (usbConnectedState != systemUsbHostConnected()) {
			if (systemUsbHostConnected()) {	
				OLEDSendCommand(Mug.oled, OLED_CMD_POWER_LEVEL, OLED_POWER_ICON_EXT);
				__oled_send_msg("USB Host detected");
				usbConnectedState = 1;
			} else {
				OLEDSendCommand(Mug.oled, OLED_CMD_POWER_LEVEL, OLED_POWER_ICON_L4);
				__oled_send_msg("USB Host removed");
				usbConnectedState = 0;
			}
		}

		if (XBeeHasData(Mug.xbee)) {
			char *msg;
			OLEDSendCommand(Mug.oled, OLED_CMD_SHOW, OLED_ICON_TRANSMISSION, OLED_ICON_SHOW);
			msg = XBeeRetrieveMessage(Mug.xbee);
			XBeeSendByte(Mug.xbee, '-');
			XBeeSendMessageNoBlock(Mug.xbee, msg);
			OLEDSendCommand(Mug.oled, OLED_CMD_SHOW, OLED_ICON_TRANSMISSION, OLED_ICON_HIDE);

			__oled_send_msg(msg);

		}
	} while(1);

	return 0;
}
