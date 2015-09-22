#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "hardware.h"
#include "usart/usart128.h"
#include "gpio.h"
#include "oled.h"

#define OLED_ENABLE	GPIO_HIGH
#define OLED_DISABLE	GPIO_LOW

#define __oled_checksum(__x) (~(__x) + 1)

struct OLED {
	OLEDState state;	
	GPIO *reset;
	struct usart_dev *uart;
	char buffer[OLED_MAX_MESSAGE_LENGTH+1];
	uint8_t length;

	/*
	struct {
		void (*write)(const char *bytes, uint8_t len);
		void (*sleep)(uint8_t mode);
		void (*wake)(void);
	} vtable;
	*/
};

static inline uint8_t __calculate_checksum(const char *msg, uint8_t bytes)
{
	uint8_t i, checksum;
	const char *m = msg + OLED_MSG_CMD_POS;
	bytes -= OLED_MSG_CMD_POS;
	
	checksum = 0;
	for (i=0; i<bytes; i++) {
		checksum += m[i];
	}

	return __oled_checksum(checksum);
}

char *OLEDRetrieveMessage(OLED *oled)
{
	if (oled == NULL)
		return NULL;

	return oled->buffer;
}

uint8_t OLEDConstructMessage(OLED *oled, OLEDCommand command, ...)
{
	va_list ap;
	char *msg;

	if (oled == NULL) {
		return 0;
	}

	va_start(ap, command);
	
	msg = oled->buffer;
	oled->length = OLED_MSG_CMD_POS;
	msg[oled->length] = command;
	oled->length = OLED_MSG_ARGS_START;
	
	switch (command) {
		case OLED_CMD_SHOW:
			msg[oled->length++] = (uint8_t)va_arg(ap, int);
			msg[oled->length++] = (uint8_t)va_arg(ap, int);	
			break;
		case OLED_CMD_POWER_LEVEL:
		case OLED_CMD_RSSI_LEVEL:
		case OLED_CMD_CONTRAST:
			msg[oled->length++] = (uint8_t)va_arg(ap, int);	
			break;
		case OLED_CMD_CLEAR_MSG_AREA:
		case OLED_CMD_CLEAR_SEL_AREA:
		case OLED_CMD_DISP_MESSAGE:
		case OLED_CMD_CLR_SCREEN:
			break;
		case OLED_CMD_WRITE_MESSAGE:
		case OLED_CMD_SET_MESSAGE:
		case OLED_CMD_SET_NAME:
			{
				uint8_t slen;
				uint8_t length = OLED_MAX_MESSAGE_LENGTH - oled->length - 2;
				char * m;

				m = va_arg(ap, char *);

				slen = strlen(m);
				strncpy((char*)msg+oled->length, m, length);
				if (slen <= length) {
					length = slen;
				} 
				oled->length += length;	
				msg[oled->length++] = '\0';	
			}
			break;
		case OLED_CMD_NOOP:
		case OLED_CMD_LAST:
		default:
			break;
	}

	va_end(ap);

	msg[OLED_MSG_START_T_POS] = OLED_MSG_START_TOKEN;
	msg[OLED_MSG_LENGTH_POS] = oled->length - OLED_MSG_LENGTH_POS;
	msg[oled->length] = __calculate_checksum(msg, oled->length);
	oled->length++;

	return oled->length;
}

uint8_t OLEDWaitForResponse(OLED *oled)
{
	if (oled == NULL || oled->uart == NULL) {
		return 0;
	}

	oled->length = 0;

	do {
		if (usart_data_available(oled->uart)) {
			char byte = usart_read_wait(oled->uart);
			if (byte == OLED_EOL) break;
			oled->buffer[oled->length++] = byte;
		}
	} while(1);

	oled->buffer[oled->length] = '\0';

	return oled->length;
}

uint8_t OLEDSendMessageNoBlock(OLED *oled)
{
	if (oled == NULL || oled->uart == NULL) {
		return 0;
	}

	usart_send_bytes(oled->uart, oled->buffer, oled->length);
	return 1;
}

uint8_t OLEDSendMessage(OLED *oled)
{
	if (!OLEDSendMessageNoBlock(oled))
		return 0;
	return OLEDWaitForResponse(oled);
}

void OLEDDisable(OLED *oled)
{
	GPIOSetDirection(oled->reset, OLED_DISABLE);
	oled->state = OLED_DISABLED;
}

void OLEDEnable(OLED *oled)
{
	GPIOSetDirection(oled->reset, OLED_ENABLE);
	oled->state = OLED_ENABLED;
}

OLED *OLEDCreate(struct usart_dev *uart)
{
	OLED *oled = (OLED*)malloc(sizeof(OLED));

	if (uart == NULL) {
		return NULL;
	}

	if (oled == NULL) {
		return NULL;
	}

	oled->uart = uart;

	oled->reset = GPIOCreate(OLED_RESET_PORT, OLED_RESET_PIN);
	OLEDDisable(oled);

	return oled;
}

void OLEDRelease(OLED *oled)
{
	if (oled != NULL)  {
		free(oled);
	}
}
