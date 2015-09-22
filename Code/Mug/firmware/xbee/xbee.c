#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include <util/delay.h>

#include "hardware.h"
#include "gpio.h"
#include "power.h"
#include "xbee.h"

struct XBee {
	GPIO *reset;
	GPIO *sleep_request;
	GPIO *associate;
	GPIO *power_state;
	GPIO *gpio;
	
	/* TODO: ADC *RRSI */
	
	char buffer[128];	
	uint8_t length;
	struct usart_dev *uart;
};

void __setup_xbee_signals(XBee *xbee)
{
	xbee->reset = GPIOCreate(ZIGBEE_RESET_PORT, ZIGBEE_RESET_PIN);
	GPIOSetDirection(xbee->reset, GPIO_HIGH);

	xbee->sleep_request = GPIOCreate(ZIGBEE_SLEEP_RQ_PORT, ZIGBEE_SLEEP_RQ_PIN);
	GPIOSetDirection(xbee->sleep_request, XBEE_SLEEP);

	xbee->associate = GPIOCreate(ZIGBEE_ASSOCIATE_PORT, ZIGBEE_ASSOCIATE_PIN);
	GPIOSetDirection(xbee->associate, GPIO_IN);

	xbee->power_state = GPIOCreate(ZIGBEE_POWER_MODE_PORT, ZIGBEE_POWER_MODE_PIN);
	GPIOSetDirection(xbee->power_state, GPIO_IN);

	xbee->gpio = GPIOCreate(ZIGBEE_GPIO_PORT, ZIGBEE_GPIO_PIN);
	GPIOSetDirection(xbee->gpio, GPIO_TRISTATE);

	/* TODO: RSSI */
}

inline uint8_t XBeeWaitForResponse(XBee *xbee)
{
	if (xbee == NULL || xbee->uart == NULL) {
		return 0;
	}

	xbee->length = 0;

	do {
		char byte = usart_read_wait(xbee->uart);
		if (byte == XBEE_EOL) break;
		xbee->buffer[xbee->length++] = byte;
	} while(1);

	xbee->buffer[xbee->length] = '\0';

	return xbee->length;
}

inline char XBeeRetrieveByte(XBee *xbee)
{
	return usart_read(xbee->uart);
}

uint8_t XBeeSendMessageNoBlock(XBee *xbee, const char *msg)
{
	if (xbee == NULL || xbee->uart == NULL) {
		return 0;
	}

	usart_send_bytes(xbee->uart, msg, strlen(msg));

	return 1;
}

uint8_t XBeeSendMessage(XBee *xbee, const char *msg)
{
	if (!XBeeSendMessageNoBlock(xbee, msg))
		return 0;
	return XBeeWaitForResponse(xbee);
}

void XBeeSendByte(XBee *xbee, char byte)
{
	if (xbee == NULL || xbee->uart == NULL)
		return;
	usart_send_byte(xbee->uart, byte);
}


XBee *XBeeCreate(struct usart_dev *uart)
{
	XBee *xbee = (XBee*)malloc(sizeof(XBee));

	if (uart == NULL) {
		return NULL;
	}

	if (xbee == NULL) {
		return NULL;
	}

	__setup_xbee_signals(xbee);
	xbee->uart = uart;

	return xbee;
}

void XBeeRelease(XBee *xbee)
{
	if (xbee != NULL)  {
		free(xbee);
	}
}

void XBeeEnterSleepMode(XBee *xbee)
{
	if (xbee != NULL)
		GPIOSetDirection(xbee->sleep_request, XBEE_SLEEP);
}

void XBeeLeaveSleepMode(XBee *xbee)
{
	if (xbee != NULL)
		GPIOSetDirection(xbee->sleep_request, XBEE_WAKE);
}

void XBeeEnterCommandMode(XBee *xbee)
{
	_delay_ms(XBEE_GUARD_TIME);
	XBeeSendMessage(xbee, XBEE_AT_CS);
	_delay_ms(XBEE_GUARD_TIME);
}

void XBeeLeaveCommandMode(XBee *xbee)
{
	XBeeSendMessage(xbee, "ATCN\r");
}

uint8_t XBeeHasData(XBee *xbee)
{
	if (xbee == NULL || xbee->uart == NULL)
		return 0;
	return usart_data_available(xbee->uart);
}

char *XBeeRetrieveMessage(XBee *xbee)
{
	if (xbee == NULL || xbee->uart == NULL)
		return NULL;

	XBeeWaitForResponse(xbee);
	return xbee->buffer;
}

void XBeeSetDestination(XBee *xbee, const char *high, const char *low)
{
	char msg[32];

	if (!high || !low)
		return;

	snprintf(msg, 31, "ATDH%s, DL%s\r", high, low);
	XBeeSendMessage(xbee, msg);
}

void XBeeSetMyId(XBee *xbee, const char *id)
{
	char msg[32];

	if (!id)
		return;

	snprintf(msg, 31, "ATMY%s\r", id);
	XBeeSendMessage(xbee, msg);
}

void XBeeSetPanId(XBee *xbee, const char *id)
{
	char msg[32];

	if (!id)
		return;

	snprintf(msg, 31, "ATID%s\r", id);
	XBeeSendMessage(xbee, msg);
}

void XBeeSetChannel(XBee *xbee, const char *channel)
{
	char msg[32];

	if (!channel)
		return;

	snprintf(msg, 31, "ATCH%s\r", channel);
	XBeeSendMessage(xbee, msg);
}

void XBeeSetCoordinatorMode(XBee *xbee, XBeeCoordinatorMode mode)
{
	char msg[8];
	//snprintf(msg, 7, "ATCE%d\r", mode);
	XBeeSendMessage(xbee, "ATCE0\r");
}

void XBeeSetEndDeviceAssociation(XBee *xbee, uint8_t mask)
{
	char msg[8];
	//snprintf(msg, 7, "ATA1%x\r", EDA_MASK & mask);
	XBeeSendMessage(xbee, "ATA10\r");
}
