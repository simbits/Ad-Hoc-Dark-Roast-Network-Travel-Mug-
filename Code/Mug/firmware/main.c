#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/wdt.h>
#include <util/atomic.h>
#include <util/delay.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "adc.h"
#include "button.h"
#include "clock.h"
#include "gpio.h"
#include "hardware.h"
#include "power.h"
#include "oled/oled.h"
#include "usart/usart128.h"
#include "xbee/xbee.h"

#ifndef true
#define true 1
#define false 0
#endif

typedef uint8_t bool;

volatile uint32_t activityTimer;

#define MUG_EMSG_COUNT	20
#define MUG_EMSG_EMPTY	0

uint8_t EEMEM eMugMsg0[MUG_MAX_MESSAGE_SIZE] = "";
uint8_t EEMEM eMugMsg1[MUG_MAX_MESSAGE_SIZE] = "is tired";
uint8_t EEMEM eMugMsg2[MUG_MAX_MESSAGE_SIZE] = "is lonely";
uint8_t EEMEM eMugMsg3[MUG_MAX_MESSAGE_SIZE] = "is bored";
uint8_t EEMEM eMugMsg4[MUG_MAX_MESSAGE_SIZE] = "is horney";
uint8_t EEMEM eMugMsg5[MUG_MAX_MESSAGE_SIZE] = "isn't sorry";
uint8_t EEMEM eMugMsg6[MUG_MAX_MESSAGE_SIZE] = "is relieved";
uint8_t EEMEM eMugMsg7[MUG_MAX_MESSAGE_SIZE] = "is stoked";
uint8_t EEMEM eMugMsg8[MUG_MAX_MESSAGE_SIZE] = "is late";
uint8_t EEMEM eMugMsg9[MUG_MAX_MESSAGE_SIZE] = "is late";
uint8_t EEMEM eMugMsg10[MUG_MAX_MESSAGE_SIZE] = "is very very late";
uint8_t EEMEM eMugMsg11[MUG_MAX_MESSAGE_SIZE] = "is tired";
uint8_t EEMEM eMugMsg12[MUG_MAX_MESSAGE_SIZE] = "is lonely";
uint8_t EEMEM eMugMsg13[MUG_MAX_MESSAGE_SIZE] = "is bored";
uint8_t EEMEM eMugMsg14[MUG_MAX_MESSAGE_SIZE] = "is horney";
uint8_t EEMEM eMugMsg15[MUG_MAX_MESSAGE_SIZE] = "Fucking hell what to f***";
uint8_t EEMEM eMugMsg16[MUG_MAX_MESSAGE_SIZE] = "is relieved";
uint8_t EEMEM eMugMsg17[MUG_MAX_MESSAGE_SIZE] = "is stoked";
uint8_t EEMEM eMugMsg18[MUG_MAX_MESSAGE_SIZE] = "is late";
uint8_t EEMEM eMugMsg19[MUG_MAX_MESSAGE_SIZE] = "Latest message";

void * eMugMsgs[MUG_EMSG_COUNT] = {
	&eMugMsg0,
	&eMugMsg1,
	&eMugMsg2,
	&eMugMsg3,
	&eMugMsg4,
	&eMugMsg5,
	&eMugMsg6,
	&eMugMsg7,
	&eMugMsg8,
	&eMugMsg9,
	&eMugMsg10,
	&eMugMsg11,
	&eMugMsg12,
	&eMugMsg13,
	&eMugMsg14,
	&eMugMsg15,
	&eMugMsg16,
	&eMugMsg17,
	&eMugMsg18,
	&eMugMsg19
};

typedef enum {
	MUG_STATE_INIT,
	MUG_STATE_INITIALIZED,
	MUG_STATE_RECEIVE_MSG,
	MUG_STATE_SEND_MSG,
	MUG_STATE_SWITCH_CONSOLE,
	MUG_STATE_SWITCH_CONSOLE_CANCEL,
	MUG_STATE_SWITCH_CONSOLE_ACCEPT,
	MUG_STATE_USB_HOST_CHANGED,
	MUG_STATE_CHECK_BATTERY,
	MUG_STATE_GESTURE_IS_VALID,
	MUG_STATE_BUTTON_PRESS_LONG,
	MUG_STATE_BUTTON_PRESS_SHORT,
	MUG_STATE_TERMINAL,
	MUG_STATE_IDLE,
	MUG_STATE_POWER_DOWN,
} mugStateType;

typedef enum {
	MUG_POWER_SAVE,
	MUG_POWER_AWAKE,
} mugPowerStateType;

typedef enum {
	MUG_NO_MSG,
	MUG_HAS_MSG,
} mugMsgStateType;

typedef enum {
	TTY_OLED,
	TTY_TERMINAL,
} consoleType;

typedef struct mugConfig {
	char name[16];
	char id[5];
	char pan[5];	
	char channel[3];
	uint8_t sleepTime;
} mugConfig;

typedef enum {
	GESTURE_STATE_INIT,
	GESTURE_STATE_NONE,
	GESTURE_STATE_STARTED,
	GESTURE_STATE_BUSY,
	GESTURE_STATE_DONE,
	GESTURE_STATE_VALID
} gestureStateType;

typedef struct gesture {
	gestureStateType previousState;
	gestureStateType currentState;
	uint8_t samples;
	uint16_t x;
	uint16_t y;
	uint16_t z;
	uint16_t xWindow[ACC_WINDOW_SIZE];
	uint16_t yWindow[ACC_WINDOW_SIZE];
	uint16_t zWindow[ACC_WINDOW_SIZE];
	uint16_t duration;
} gesture;

typedef struct mug {
	XBee * xbee;
	OLED * oled;
	struct usart_dev *tty_console;
	struct usart_dev *tty_xbee;

	mugStateType previousState;
	mugStateType currentState;
	mugPowerStateType powerState;
	mugMsgStateType msgState;

	consoleType console;
	bool usbHostConnected;
	GPIO *gpioConsoleOutputEnable;
	GPIO *gpioConsoleSwitch;
	GPIO *gpioButton;
	GPIO *gpioAccST;

	button *button;
	uint8_t buttonState;
	uint16_t batteryLevel;
	uint8_t batteryIcon;

	uint8_t msgIndex;
	int8_t msgSendDelay;
	char selectedMessage[MUG_MAX_MESSAGE_SIZE];
	char msgReceive[MUG_MAX_MESSAGE_SIZE];

	mugConfig config;
} mug;

mugConfig mugConfigEep EEMEM = {
	.name 	= CONFIG_DEFAULT_NAME,
	.id 	= CONFIG_DEFAULT_ID,
	.pan 	= CONFIG_DEFAULT_PAN,
	.channel = CONFIG_DEFAULT_CHANNEL,
	.sleepTime = 30,
};

/* Make sure watchdog is disabled */
uint8_t mcusr_mirror __attribute__ ((section (".noinit")));
void get_mcusr(void) __attribute__((naked)) __attribute__((section(".init3")));
void get_mcusr(void)
{
	mcusr_mirror = MCUSR;
	MCUSR = 0;
	wdt_disable();
}
/* --- */

#define BUFFER_SIZE     128
#define WAITING			0
#define RECEIVING		1
#define DONE			2
#define PARSE_ERROR		3
#define BUFFER_OVERFLOW	4

static uint8_t terminalStatus;
static char terminalReceivedBuffer[BUFFER_SIZE];
static uint8_t terminalBytesReceived;

static mug aMug;
static gesture aGesture;
static void mugSetCurrentState(mugStateType newState);
static void mugSwitchConsoleTo(consoleType console);


#define filter_fwa(__old, __new, __prct) (((__old * (100 - (__prct)) + (__new) * (__prct))) / 100)

#define mugDisplayCommand(__cmd, ...) 						\
	do {									\
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 				\
		{								\
		if (OLEDConstructMessage((aMug.oled), (__cmd), __VA_ARGS__)) {	\
			consoleType __c = aMug.console;				\
			mugSwitchConsoleTo(TTY_OLED);				\
			_delay_ms(5);						\
			OLEDSendMessage((aMug.oled));				\
			mugSwitchConsoleTo(__c);				\
		}								\
		}								\
	} while(0)

#define mugDisplayMessage(msg)	 						\
	do {									\
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 				\
		{								\
		consoleType __c = aMug.console;					\
		mugSwitchConsoleTo(TTY_OLED);					\
		_delay_ms(5);							\
		OLEDSendCommand(aMug.oled, OLED_CMD_WRITE_MESSAGE, msg);	\
		mugSwitchConsoleTo(__c);					\
		}								\
	} while(0)

#define mugDisplaySelectMessage(msg) 						\
	do {									\
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 				\
		{								\
		consoleType __c = aMug.console;					\
		mugSwitchConsoleTo(TTY_OLED);					\
		_delay_ms(5);							\
		OLEDSendCommand(aMug.oled, OLED_CMD_SET_MESSAGE, msg);		\
		mugSwitchConsoleTo(__c);					\
		}								\
	} while(0)

#define mugDisplaySelectedMessage() 						\
	do {									\
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) 				\
		{								\
		consoleType __c = aMug.console;					\
		mugSwitchConsoleTo(TTY_OLED);					\
		_delay_ms(5);							\
		OLEDSendCommand(aMug.oled, OLED_CMD_DISP_MESSAGE, 0);		\
		mugSwitchConsoleTo(__c);					\
		}								\
	} while(0)

#define mugResetActivity()							\
	do {									\
		aMug.powerState = MUG_POWER_AWAKE;				\
		ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {				\
			activityTimer = 0;					\
		}								\
	} while(0)

#define mugResetActivityNonAtomic()						\
	do {									\
		aMug.powerState = MUG_POWER_AWAKE;				\
		activityTimer = 0;						\
	} while(0)


void __button_event_handler(button *aButton, buttonEvent *event)
{
	aMug.buttonState = event->event;
}

static void mugReadConfig(void)
{
	eeprom_busy_wait();
	eeprom_read_block(&aMug.config, &mugConfigEep, sizeof(mugConfig));
}

static void mugWriteConfig(void)
{
	eeprom_busy_wait();
	eeprom_write_block(&aMug.config, &mugConfigEep, sizeof(mugConfig));
}

static void mugShutdown(void)
{
	mugDisplayMessage(MSG_SHUTDOWN);
	XBeeRelease(aMug.xbee);	
	mugSwitchConsoleTo(TTY_OLED);
	mugDisplayCommand(OLED_CMD_CONTRAST, 0);
	OLEDDisable(aMug.oled);
	OLEDRelease(aMug.oled);
	systemMainPower(MAIN_POWER_OFF);
}

static void mugSetCurrentState(mugStateType newState)
{
	if (newState != aMug.currentState) {
		aMug.previousState = aMug.currentState;
		aMug.currentState = newState;	
	}
}

static void mugSwitchConsoleTo(consoleType console) 
{
	switch (console) {
		case TTY_TERMINAL:
			GPIOSetDirection(aMug.gpioConsoleSwitch, GPIO_HIGH);
			break;
		case TTY_OLED:
		default:
			GPIOSetDirection(aMug.gpioConsoleSwitch, GPIO_LOW);
	}
	aMug.console = console;
}

static void mugEnableConsole(void)
{
	GPIOSetDirection(aMug.gpioConsoleOutputEnable, GPIO_LOW);
}

static void mugDisableConsole(void) __attribute__((unused));
static void mugDisableConsole(void)
{
	GPIOSetDirection(aMug.gpioConsoleOutputEnable, GPIO_HIGH);
}

static void mugSetupConsole(void)
{
	aMug.gpioConsoleSwitch = GPIOCreate(CONSOLE_SWITCH_PORT, CONSOLE_SWITCH_PIN);
	aMug.gpioConsoleOutputEnable = GPIOCreate(CONSOLE_ENABLE_PORT, CONSOLE_ENABLE_PIN);

	aMug.tty_console = usart_init(OLED_CONSOLE_UART,
			       OLED_CONSOLE_BAUDRATE, 
			       USART_BITS_8,
			       USART_STOPBITS_1,
			       USART_PARITY_NONE,
			       0);

	aMug.oled = OLEDCreate(aMug.tty_console);

	mugSwitchConsoleTo(TTY_OLED);
	mugEnableConsole();

	OLEDEnable(aMug.oled);
	OLEDWaitForResponse(aMug.oled);
}

static void mugInitializeXBee(void)
{
	XBeeLeaveSleepMode(aMug.xbee);
	
	XBeeEnterCommandMode(aMug.xbee);

	XBeeSetDestination(aMug.xbee, "0000", "FFFF");
	XBeeSetMyId(aMug.xbee, aMug.config.id);
	XBeeSetPanId(aMug.xbee, aMug.config.pan);
	XBeeSetChannel(aMug.xbee, aMug.config.channel);
	XBeeSetCoordinatorMode(aMug.xbee, XBEE_END_DEVICE);
	XBeeSetEndDeviceAssociation(aMug.xbee, EDA_NO_ASSOCIATION);
	
	mugDisplayMessage(XBeeRetrieveMessage(aMug.xbee));

	XBeeLeaveCommandMode(aMug.xbee);
}

static void mugSetupXBee(void)
{
	aMug.tty_xbee = usart_init(ZIGBEE_UART,
			    ZIGBEE_BAUDRATE, 
			    USART_BITS_8,
			    USART_STOPBITS_1,
			    USART_PARITY_NONE,
			    0);

	aMug.xbee = XBeeCreate(aMug.tty_xbee);
}

static void mugSetupButton(void)
{
	aMug.gpioButton = GPIOCreate(BUTTON_PORT, BUTTON_PIN);
	GPIOSetDirection(aMug.gpioButton, GPIO_IN_PULLUP);
	
	aMug.button = buttonCreate(aMug.gpioButton, __button_event_handler, NULL);
	buttonUpState(aMug.button, GPIO_LOW);
	buttonSetShortPressDelay(aMug.button, BUTTON_PRESS_SHORT_MIN, BUTTON_PRESS_SHORT_MAX);
	buttonSetLongPressDelay(aMug.button, BUTTON_PRESS_LONG_MIN, BUTTON_PRESS_LONG_MAX);
	buttonListenForEvents(aMug.button, BUTTON_EVENT_PRESS_SHORT | BUTTON_EVENT_PRESS_LONG);
}

/* watchdog is only used for battery check */
static void mugSetupWatchdog(void)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
		asm volatile ("wdr");
		WDTCSR |= (_BV(WDCE) | _BV(WDE));
		WDTCSR = (_BV(WDIE) | _BV(WDP2) | _BV(WDP1));
	}
}

static void mugInitializeSystemModules(void)
{
	cli();
	systemInitPower();
	systemInitClock();
	mugSetupWatchdog();
	adcInitialize(ADC_PRESCALER_64, ADC_REF_AVCC, ADC_RIGHT_ADJUST);
	sei();
}

static void mugWriteMessage(const char *msg)
{
	switch (aMug.console)
	{
		case TTY_TERMINAL:
			usart_send_string(aMug.tty_console, msg);
			usart_send_bytes(aMug.tty_console, "\n\r", 2);
			break;	
		case TTY_OLED:
		default:
			OLEDSendCommand(aMug.oled, OLED_CMD_SET_MESSAGE, msg);
			OLEDSendCommand(aMug.oled, OLED_CMD_DISP_MESSAGE, 0);
	}
}

static inline uint8_t mugBatteryLevelToIcon(uint16_t level)
{
	if (level == BATTERY_EXT_VAL) return OLED_POWER_ICON_EXT;
	if (level < BATTERY_L1_VAL) return OLED_POWER_ICON_L0;
	if (level < BATTERY_L2_VAL) return OLED_POWER_ICON_L1;
	if (level < BATTERY_L3_VAL) return OLED_POWER_ICON_L2;
	if (level < BATTERY_L4_VAL) return OLED_POWER_ICON_L3;
	return OLED_POWER_ICON_L4;
}

static inline void mugUpdateBatteryIcon(void)
{
	uint8_t icon = mugBatteryLevelToIcon(aMug.batteryLevel);

	if (icon != aMug.batteryIcon) {
		mugDisplayCommand(OLED_CMD_POWER_LEVEL, icon);
		aMug.batteryIcon = icon;
	}
}

static inline bool mugCheckUSBHost(void)
{
	uint8_t hostConnected = systemUsbHostConnected();
	bool hostConnectionChanged = false;

	if (aMug.usbHostConnected != hostConnected) {
		if (hostConnected) {
			aMug.batteryLevel = 0;
            if (aMug.currentState != MUG_STATE_INIT) {
                mugDisplayMessage("To switch to\nconsole press\nbutton within\n5 seconds");
                mugSetCurrentState(MUG_STATE_SWITCH_CONSOLE);
            }
		} else {
            if (aMug.currentState != MUG_STATE_INIT) {
                mugSetCurrentState(MUG_STATE_SWITCH_CONSOLE_CANCEL);
            }
		}
		aMug.usbHostConnected = hostConnected;

		hostConnectionChanged = true;
	}

	return hostConnectionChanged;
}

static inline bool mugCheckBatteryLevel(void)
{
	bool updateBatteryIcon = false;

	if (!aMug.usbHostConnected) {
		uint16_t level, currentLevel;

		adcStartConversion(MUX_BATTERY_LEVEL);
		level = adcConversionResult();
		currentLevel = aMug.batteryLevel;

		if (!currentLevel) {
			aMug.batteryLevel = level;
			updateBatteryIcon = true;
		} else {
			/* simple hystherisis */
			if (level < currentLevel - BATTERY_HYST_VAL || 
			    level > currentLevel + BATTERY_HYST_VAL) {
				aMug.batteryLevel = level;
				updateBatteryIcon = true;
			}
		}
	}

	return updateBatteryIcon;
}

static inline void updateBatteryStatus(void)
{
	if (mugCheckUSBHost()) {
		mugCheckBatteryLevel();
		mugUpdateBatteryIcon();
	} else if (mugCheckBatteryLevel()) {
		mugUpdateBatteryIcon();
	}
}

static inline void gestureHandleValidGesture(void)
{
	if (aMug.msgSendDelay <= 0) {
		mugSetCurrentState(MUG_STATE_GESTURE_IS_VALID);
		aMug.msgSendDelay = 100;
	}
}

static inline void gestureSetCurrentState(gestureStateType state)
{
	aGesture.previousState = aGesture.currentState;
	aGesture.currentState = state;
}

static inline void mugProcessAccelVector(uint32_t delay)
{
	static uint16_t samples = 0;
	static uint16_t sx = 0, sy = 0, sz = 0;
	static int16_t cumDelay = 0;
	uint16_t x, y, z;

	adcStartConversion(MUX_ACC_X);
	x = adcConversionResult();
	adcStartConversion(MUX_ACC_Y);
	y = adcConversionResult();
	adcStartConversion(MUX_ACC_Z);
	z = adcConversionResult();

	if (aGesture.currentState == GESTURE_STATE_INIT) {
		if (samples <= ACC_WINDOW_SIZE-1) {
			x = filter_fwa(aGesture.x, x, ACC_FWA_RATIO);
			y = filter_fwa(aGesture.y, y, ACC_FWA_RATIO);
			z = filter_fwa(aGesture.z, z, ACC_FWA_RATIO);
		} else {
			sx = x;
			sy = y;
			sz = z;
			gestureSetCurrentState(GESTURE_STATE_NONE);
		}
		goto done;
	}

	x = filter_fwa(aGesture.x, x, ACC_FWA_RATIO);
	y = filter_fwa(aGesture.y, y, ACC_FWA_RATIO);
	z = filter_fwa(aGesture.z, z, ACC_FWA_RATIO);

done:
	cumDelay += delay;

	if (++samples >= ACC_WINDOW_SIZE) {
		int16_t dx, dy, dz;

		dx = (int16_t)x - sx;
		dy = (int16_t)y - sy;
		dz = (int16_t)z - sz;
		
		dx = (dx * 1000) / cumDelay;
		dy = (dy * 1000) / cumDelay;
		dz = (dz * 1000) / cumDelay;

/*
		if (aMug.console == TTY_TERMINAL) {
			snprintf(msg, 127, "%.d (%d): %d - %d - %d", samples, cumDelay, dx, dy, dz);
			mugWriteMessage(msg);
		}
*/

		if (dx > 5 || dx < -5 ||
		    dy > 5 || dy < -5 ||
                    dz > 5 || dz < -5) {
			mugResetActivity();
		}

		/* right handed */
		if (dx > 40 && dy > 40 /*&& dz > 40*/) {
			gestureHandleValidGesture();
			mugResetActivity();
		}

		if (dx < -40 && dy > 40 /*&& dz > 40*/) {
			gestureHandleValidGesture();
			mugResetActivity();
		}


		/* TODO: left handed */

		sx = x;
		sy = y;
		sz = z;

		samples = 0;
		cumDelay = 0;
	}

	aGesture.x = x;
	aGesture.y = y;
	aGesture.z = z;
}

static mugStateType switchToConsole(uint16_t delay)
{
	static uint32_t elapsed = 0;
    
	if (delay == 0) {
        elapsed = 0;
        mugDisplayMessage("Switching to\nconsole");
        return MUG_STATE_SWITCH_CONSOLE_ACCEPT;
	}
    
    if (elapsed == 0) {
        elapsed = systemGetElapsedTime();
    } else{
        if (systemGetElapsedTime() - elapsed >= delay) {
            elapsed = 0;
            mugDisplayMessage("Console Cancelled");
            return MUG_STATE_SWITCH_CONSOLE_CANCEL;
        }
    }
    
    return aMug.currentState;
}

static void mugPowerDown(uint16_t delay)
{
	static uint32_t elapsed = 0;

	if (delay == 0) {
		mugWriteMessage(MSG_SHUTDOWN_CANCELLED);
		elapsed = 0;
		return;
	}

	if (elapsed == 0) {
		char msg[32];

		snprintf(msg, 31, MSG_SHUTDOWN_NOTIFY, delay / 1000);
		mugDisplayMessage(msg);
		mugDisplayMessage(MSG_BTN_TO_CANCEL);
		elapsed = systemGetElapsedTime();
	} else { 
		if (systemGetElapsedTime() - elapsed >= delay) {
			mugShutdown();
		}
	}
}

static void mugStoreMessage(uint8_t index, const char *msgPtr)
{
	char msg[MUG_MAX_MESSAGE_SIZE];
	snprintf(msg, MUG_MAX_MESSAGE_SIZE-1, "%s", msgPtr);
	eeprom_busy_wait();
	eeprom_write_block(&msg, eMugMsgs[index], MUG_MAX_MESSAGE_SIZE);
}

static char *mugReadMessage(uint8_t index)
{
	static char msg[MUG_MAX_MESSAGE_SIZE] = "";

	
	eeprom_busy_wait();
	eeprom_read_block(&msg, eMugMsgs[index], MUG_MAX_MESSAGE_SIZE);

	return msg;
}

static void mugSelectMessage(uint8_t index)
{
	char *newMsg = "";
	char *msg = aMug.selectedMessage;


	for (; index < MUG_EMSG_COUNT; index++) {
		newMsg = mugReadMessage(index);	
		if (strlen(newMsg) == 0) {
			if (index == MUG_EMSG_EMPTY) {
				goto select;
			}
		} else {
			goto select;
		}
	}

	index = MUG_EMSG_EMPTY;

select:
	snprintf(msg, MUG_MAX_MESSAGE_SIZE-1, "> %s", mugReadMessage(index));
	aMug.msgIndex = index;
}

static mugStateType mugHandleLongButtonPress(void)
{
	mugStateType newState = MUG_STATE_IDLE;

	switch (aMug.currentState) {
		case MUG_STATE_TERMINAL:
			newState = MUG_STATE_SWITCH_CONSOLE_CANCEL;
			break;
		default:
			newState = MUG_STATE_POWER_DOWN;
	} 

	return newState;
}

static mugStateType mugHandleShortButtonPress(void)
{
	mugStateType newState = MUG_STATE_IDLE;

	switch (aMug.currentState) {
		case MUG_STATE_SWITCH_CONSOLE:
			newState = switchToConsole(0);
			break;
		case MUG_STATE_POWER_DOWN:
			mugPowerDown(0);
			break;
		default:
			if (aMug.msgIndex++ >= MUG_EMSG_COUNT-1)
				aMug.msgIndex = 0;
			mugSelectMessage(aMug.msgIndex);	
			mugDisplaySelectMessage(aMug.selectedMessage); 
			newState = aMug.currentState;
	}
	
	return newState;
}

static inline mugStateType mugReceiveMessage(void)
{
	char msg[128];
	uint8_t cnt = 0;

	mugDisplayCommand(OLED_CMD_SHOW, OLED_ICON_TRANSMISSION, OLED_ICON_SHOW);
	do {
		char c = usart_read(aMug.tty_xbee);

		if (c == XBEE_EOL) {
			break;
		}

		if (c != '\0')
			msg[cnt++] = c;

	} while (1);

	msg[cnt] = '\0';
	mugDisplayMessage(msg);
	mugDisplayCommand(OLED_CMD_SHOW, OLED_ICON_TRANSMISSION, OLED_ICON_HIDE);
	mugResetActivity();

	return MUG_STATE_IDLE;
}

static mugStateType mugSendMessage(void)
{
	uint8_t i, j;
	uint8_t foundSpace;
	char msg[128];

	if (aMug.msgIndex == MUG_EMSG_EMPTY)
		return MUG_STATE_IDLE;

	mugDisplayCommand(OLED_CMD_SHOW, OLED_ICON_TRANSMISSION, OLED_ICON_SHOW);
	
	snprintf(msg, 128, "*%s%s", aMug.config.name, aMug.selectedMessage);

	foundSpace = 0;
	for (i=1,j=0; i<strlen(msg); i++,j++) { 

		if (j==18) {
			if (foundSpace) {
				*(msg + foundSpace) = '\n';
				i = foundSpace + 1;
				foundSpace = 0;
			} else {
				*(msg+i) = '\n';
			}
			j = 0;
		} else if (*(msg+i) == ' ') {
			foundSpace = i;
		}


	}

	XBeeSendMessageNoBlock(aMug.xbee, msg);
	XBeeSendByte(aMug.xbee, '\r');
	
	/* fake delay */
	_delay_ms(500);
	snprintf(msg, 128, "/me%s", aMug.selectedMessage);

	foundSpace = 0;
	for (i=1,j=0; i<strlen(msg); i++,j++) { 

		if (j>=18) {
			if (foundSpace) {
				msg[foundSpace] = '\n';
				i = foundSpace + 1;
				foundSpace = 0;
			} else {
				msg[i] = '\n';
			}
			j = 0;
		} else if (msg[i] == ' ') {
			foundSpace = i;
		}



	}

	mugDisplayMessage(msg); 
	mugDisplayCommand(OLED_CMD_SHOW, OLED_ICON_TRANSMISSION, OLED_ICON_HIDE);

	return MUG_STATE_IDLE;
}

static void mugWakeUpIfNeeded(void)
{
	static mugPowerStateType currentPowerState = MUG_POWER_AWAKE;

	if (aMug.powerState != currentPowerState) {
		if (aMug.powerState == MUG_POWER_AWAKE) {
			mugDisplayCommand(OLED_CMD_CONTRAST, 16);
		} else {
			mugDisplayCommand(OLED_CMD_CONTRAST, 0);
		}
		currentPowerState = aMug.powerState;
	} else {
		if (activityTimer > aMug.config.sleepTime) {
			aMug.powerState = MUG_POWER_SAVE;
		}
	}
}

static void mugIdleState(void)
{
	static uint32_t startTime = 0;
	uint32_t msElapsed;

	if (startTime == 0) {
		startTime = systemGetElapsedTime();
	}
	msElapsed = systemGetElapsedTime() - startTime;

	if (aMug.buttonState & BUTTON_EVENT_PRESS_SHORT) {
		mugResetActivity();
		mugWakeUpIfNeeded();
		mugSetCurrentState(mugHandleShortButtonPress());
		aMug.buttonState = 0;
	} else if (aMug.buttonState & BUTTON_EVENT_PRESS_LONG) {
		mugResetActivity();
		mugWakeUpIfNeeded();
		mugSetCurrentState(mugHandleLongButtonPress());
		aMug.buttonState = 0;
	}

	if (aMug.currentState == MUG_STATE_INIT || aMug.currentState == MUG_STATE_TERMINAL) 
		return;

	if (aMug.msgState == MUG_HAS_MSG) {
		mugResetActivity();
		mugWakeUpIfNeeded();
		mugDisplayCommand(OLED_CMD_SHOW, OLED_ICON_TRANSMISSION, OLED_ICON_SHOW);
		mugDisplayMessage(aMug.msgReceive);
		mugDisplayCommand(OLED_CMD_SHOW, OLED_ICON_TRANSMISSION, OLED_ICON_HIDE);
		aMug.msgState = MUG_NO_MSG;
	}

	if (msElapsed > ACC_SAMPLE_INTERVAL) {
		if (--aMug.msgSendDelay <= 0) 
			aMug.msgSendDelay = 0;
		mugProcessAccelVector(msElapsed);
		startTime = systemGetElapsedTime();
	}

	mugWakeUpIfNeeded();
}

static inline char *skip_whitespace(const char *str)
{
	char *ptr = (char *)str;
	while (isspace((int)*ptr))
		ptr++;
	return ptr;
}

static uint8_t parse_general(const char *str)
{
	char *ptr = (char *)str;
	char action = 0;
	
	if (!isdigit((int)*ptr)) {
		if (isspace((int)*ptr)) {
			ptr = skip_whitespace(ptr);
			switch (*ptr++) {
				case 'n': case 'N':
					ptr = skip_whitespace(ptr);
					action = *ptr;
					if (action == 'g') {
						usart_send_string(aMug.tty_console, "NAME: ");
						usart_send_string(aMug.tty_console, aMug.config.name);
						usart_send_string(aMug.tty_console, "\n\r");
					} else if (action == 's') {
						ptr++;
						ptr = skip_whitespace(ptr);
						snprintf(aMug.config.name, 16, "%s", ptr);
						mugWriteConfig();
						mugReadConfig();
						usart_send_string(aMug.tty_console, "OK\n\r");
					} else {
						return PARSE_ERROR;
					}
					break;
				case 'i': case 'I':
					ptr = skip_whitespace(ptr);
					action = *ptr;
					if (action == 'g') {
						usart_send_string(aMug.tty_console, "ID: ");
						usart_send_string(aMug.tty_console, aMug.config.id);
						usart_send_string(aMug.tty_console, "\n\r");
					} else if (action == 's') {
						ptr++;
						ptr = skip_whitespace(ptr);
						snprintf(aMug.config.id, 5, "%s", ptr);
						mugWriteConfig();
						mugReadConfig();
						usart_send_string(aMug.tty_console, "OK\n\r");
					} else {
						return PARSE_ERROR;
					}
					break;
				case 'p': case 'P':
					ptr = skip_whitespace(ptr);
					action = *ptr;
					if (action == 'g') {
						usart_send_string(aMug.tty_console, "PAN: ");
						usart_send_string(aMug.tty_console, aMug.config.pan);
						usart_send_string(aMug.tty_console, "\n\r");
					} else if (action == 's') {
						ptr++;
						ptr = skip_whitespace(ptr);
						snprintf(aMug.config.pan, 5, "%s", ptr);
						mugWriteConfig();
						mugReadConfig();
						usart_send_string(aMug.tty_console, "OK\n\r");
					} else {
						return PARSE_ERROR;
					}
					break;
				case 'c': case 'C':
					ptr = skip_whitespace(ptr);
					action = *ptr;
					if (action == 'g') {
						usart_send_string(aMug.tty_console, "CHANNEL: ");
						usart_send_string(aMug.tty_console, aMug.config.channel);
						usart_send_string(aMug.tty_console, "\n\r");
					} else if (action == 's') {
						ptr++;
						ptr = skip_whitespace(ptr);
						snprintf(aMug.config.channel, 3, "%s", ptr);
						mugWriteConfig();
						mugReadConfig();
						usart_send_string(aMug.tty_console, "OK\n\r");
					} else {
						return PARSE_ERROR;
					}
					break;

				case 's': case 'S':
					ptr = skip_whitespace(ptr);
					action = *ptr;
					if (action == 'g') {
						char msg[16];
						snprintf(msg, 16, "Sleep: %d\n\r", aMug.config.sleepTime);
						usart_send_string(aMug.tty_console, msg);
					} else if (action == 's') {
						uint8_t sleep;
						ptr++;
						ptr = skip_whitespace(ptr);
						
						if (isdigit((int)*ptr))					
							sleep = atoi(ptr);
						else 
							return PARSE_ERROR;

						if (sleep == 0)	
							return PARSE_ERROR;
						aMug.config.sleepTime = sleep;
						mugWriteConfig();
						mugReadConfig();
						usart_send_string(aMug.tty_console, "OK\n\r");
					} else {
						return PARSE_ERROR;
					}
					break;

				case '?':
					usart_send_string(aMug.tty_console, "GENERAL name: n \n\r");
					usart_send_string(aMug.tty_console, "  get: g\n\r");
					usart_send_string(aMug.tty_console, "  set: s  name\n\r");
					usart_send_string(aMug.tty_console, "GENERAL id: i \n\r");
					usart_send_string(aMug.tty_console, "  get: g\n\r");
					usart_send_string(aMug.tty_console, "  set: s  id\n\r");
					usart_send_string(aMug.tty_console, "GENERAL pan: p \n\r");
					usart_send_string(aMug.tty_console, "  get: g\n\r");
					usart_send_string(aMug.tty_console, "  set: s  pan\n\r");
					usart_send_string(aMug.tty_console, "GENERAL channel: c \n\r");
					usart_send_string(aMug.tty_console, "  get: g\n\r");
					usart_send_string(aMug.tty_console, "  set: s  channel\n\r");
					usart_send_string(aMug.tty_console, "GENERAL sleep (seconds): s \n\r");
					usart_send_string(aMug.tty_console, "  get: g\n\r");
					usart_send_string(aMug.tty_console, "  set: s [1..255]\n\r");

					break;
				default:
					return PARSE_ERROR;
			}
		} else {
			return PARSE_ERROR;
		}
		return 0;
	}
	
	return 0;
}

static uint8_t parse_messages(const char *str)
{
	char *ptr = (char *)str;
	int index = 0;
	
	if (!isdigit((int)*ptr)) {
		if (isspace((int)*ptr)) {
			ptr = skip_whitespace(ptr);
			switch (*ptr++) {
				case 'r': case 'R':
					ptr = skip_whitespace(ptr);

					if (isdigit((int)*ptr))					
						index = atoi(ptr);

					if (index > MUG_EMSG_COUNT)
						return PARSE_ERROR;
	
					if (index > 0) {
						usart_send_string(aMug.tty_console, mugReadMessage(index));
						usart_send_string(aMug.tty_console, "\n\r");
					} else {
						for (;index<MUG_EMSG_COUNT;index++) {
							char m[8];
							char *str = mugReadMessage(index);
							snprintf(m, 7, "%d: ", index);
							usart_send_string(aMug.tty_console, m); 
							if (strlen(str) == 0) {
								usart_send_string(aMug.tty_console, "*empty message*");
							} else {
								usart_send_string(aMug.tty_console, mugReadMessage(index));
							}
							usart_send_string(aMug.tty_console, "\n\r");
						}
					}
					break;
				case 'w': case 'W':
					ptr = skip_whitespace(ptr);
					if (!isdigit((int)*ptr))					
						return PARSE_ERROR;

					index = atoi(ptr++);

					if (index > MUG_EMSG_COUNT)
						return PARSE_ERROR;
                    
				        ptr++;
					ptr = skip_whitespace(ptr);
					mugStoreMessage(index, ptr);
					usart_send_string(aMug.tty_console, "OK\n\r");
					break;
				case '?':
					usart_send_string(aMug.tty_console, "MESSAGES read: r [index]\n\r");
					usart_send_string(aMug.tty_console, "MESSAGES write: w index msg\n\r");
					break;
				default:
					return PARSE_ERROR;
			}
		} else {
			return PARSE_ERROR;
		}
		return 0;
	}
	
	return 0;
}

static uint8_t parse_result(const char *str, uint8_t length)
{
	char *ptr = (char *)str;
	uint8_t error = 0;
	
	if (!str)
		return PARSE_ERROR;
	if (length <= 0)
		return PARSE_ERROR;
	
	switch (*ptr++) {
		case 'g': case 'G':
			error = parse_general(ptr);
			break;
		case 'm': case 'M':
			error = parse_messages(ptr);
			break;
		case '?':
			usart_send_string(aMug.tty_console, MSG_TERMINAL_WELCOME);
			usart_send_string(aMug.tty_console, "General: g (g ? for help)\n\r");
			usart_send_string(aMug.tty_console, "Messages: m (m ? for help)\n\r");
			usart_send_string(aMug.tty_console, "Accel: a (debug accelerometer)\n\r");
			usart_send_string(aMug.tty_console, "Shutdown: s (shutdown now!)\n\r");
			usart_send_string(aMug.tty_console, "Help: ?\n\r");
			break;
		case 'q': case 'Q':
		    	mugDisplayMessage("^sys Console Cancelled");
		    	aMug.currentState = MUG_STATE_SWITCH_CONSOLE_CANCEL;
		    	break;
		case 'a': case 'A':
			aMug.currentState = MUG_STATE_IDLE;
			break;
		case 's': case 'S':
			mugShutdown();
			break;
		default:
			usart_send_string(aMug.tty_console, "ERR: unknown command '");
			usart_send_byte(aMug.tty_console, *(ptr-1));
			usart_send_string(aMug.tty_console, "'\n\r");
	}
	
	if (error) {
		return PARSE_ERROR;
	}
	
	return DONE;
}


static void mugTerminal(void)
{
	char received_char;
	
	if (terminalStatus == DONE) {
		memset(terminalReceivedBuffer, '\0', sizeof(terminalReceivedBuffer));
		terminalBytesReceived = 0;
		terminalStatus = WAITING;
	}
	
	if (!usart_data_available(aMug.tty_console))
		goto empty_rx_buffer;
	
	while (usart_data_available(aMug.tty_console)) {
		if (terminalBytesReceived >= BUFFER_SIZE) {
			terminalStatus = BUFFER_OVERFLOW;
			goto error_rx_buffer;
		}
	
		received_char = usart_read(aMug.tty_console);
		usart_send_byte(aMug.tty_console, received_char);
		
		if (received_char == '\r' || received_char == '\n' || received_char == '\0') {
			if (terminalStatus == WAITING)
				goto empty_rx_buffer;
			
			usart_send_string(aMug.tty_console, "\n\r");
			terminalStatus = parse_result(terminalReceivedBuffer, terminalBytesReceived);
			goto done_parsing;
		}
		terminalReceivedBuffer[terminalBytesReceived++] = received_char;
		terminalStatus = RECEIVING;
	}
	
done_parsing:
error_rx_buffer:
	if (terminalStatus == PARSE_ERROR) {
		usart_send_string(aMug.tty_console, "ERR: parser error\n\r");
		terminalStatus = DONE;
	} else if (terminalStatus == BUFFER_OVERFLOW) {
		usart_send_string(aMug.tty_console, "ERR: buffer overflow\n\r");
		terminalStatus = DONE;
	}
	
empty_rx_buffer:
	;
}

ISR(WDT_vect)
{
	if (aMug.currentState != MUG_STATE_INIT)
		mugSetCurrentState(MUG_STATE_CHECK_BATTERY);
	activityTimer++;
}

int main(int argc, char *argv[])
{	
	char msg[32];

	mugInitializeSystemModules();

	mugReadConfig();
	mugSetupConsole();

	systemMainPower(MAIN_POWER_ON);
	
	OLEDSendCommand(aMug.oled, OLED_CMD_SHOW, OLED_ICON_POWER, OLED_ICON_SHOW);
	OLEDSendCommand(aMug.oled, OLED_CMD_SET_NAME, aMug.config.name);

	snprintf(msg, 32, "Hello %s", aMug.config.name);
	mugDisplayMessage(msg);
	mugDisplayMessage("Setting up\nconnection");
	mugSetupXBee();
	mugInitializeXBee();

	mugSelectMessage(aMug.msgIndex);	
	mugDisplaySelectMessage(aMug.selectedMessage); 

	aMug.gpioAccST = GPIOCreate(ACC_ST_PORT, ACC_ST_PIN);
	GPIOSetDirection(aMug.gpioAccST, GPIO_LOW);

	mugDisplayMessage("Setup complete");
	OLEDConstructMessage(aMug.oled, OLED_CMD_CLR_SCREEN);
	OLEDSendMessage(aMug.oled);

	updateBatteryStatus();
	mugResetActivity();
	mugSetupButton();

	mugSetCurrentState(MUG_STATE_IDLE);

	do {
		if (XBeeHasData(aMug.xbee)) {
			char *msg;
			msg = XBeeRetrieveMessage(aMug.xbee);
			if (strlen(msg) >= 5) {
				memset(aMug.msgReceive, '\0', MUG_MAX_MESSAGE_SIZE);
				strncpy(aMug.msgReceive, msg, MUG_MAX_MESSAGE_SIZE);
				aMug.msgState = MUG_HAS_MSG;
			}
		}

		switch (aMug.currentState) 
		{
			case MUG_STATE_BUTTON_PRESS_LONG:
				mugSetCurrentState(mugHandleLongButtonPress());
				break;

			case MUG_STATE_BUTTON_PRESS_SHORT:
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
					mugSetCurrentState(mugHandleShortButtonPress());
				}
				break;

			case MUG_STATE_GESTURE_IS_VALID:
			case MUG_STATE_SEND_MSG:
				ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
					mugSetCurrentState(mugSendMessage());
				}
				break;

			case MUG_STATE_POWER_DOWN:
				mugPowerDown(5000);
				break;
			
			case MUG_STATE_CHECK_BATTERY:
				updateBatteryStatus();
				mugSetCurrentState(aMug.previousState);
				break;
			
			case MUG_STATE_SWITCH_CONSOLE:
				mugSetCurrentState(switchToConsole(5000));
				break;

			case MUG_STATE_SWITCH_CONSOLE_ACCEPT:
				mugSwitchConsoleTo(TTY_TERMINAL);

				_delay_ms(10);
				usart_send_string(aMug.tty_console, MSG_TERMINAL_WELCOME);
				_delay_ms(10);

				terminalStatus = DONE;
				terminalBytesReceived = 0;
				mugSetCurrentState(MUG_STATE_TERMINAL);
	
				break;

			case MUG_STATE_SWITCH_CONSOLE_CANCEL:
				/* flush */
				while(usart_data_available(aMug.tty_console))
					usart_read(aMug.tty_console);
				
				mugSwitchConsoleTo(TTY_OLED);
				_delay_ms(10);
				mugSetCurrentState(MUG_STATE_IDLE);
				break;

			case MUG_STATE_TERMINAL:
				mugTerminal();
				break;

			case MUG_STATE_IDLE:
			default: 
				;
		}
		mugIdleState();
	} while(true);
}
