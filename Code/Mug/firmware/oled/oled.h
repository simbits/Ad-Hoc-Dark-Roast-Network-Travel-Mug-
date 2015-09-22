#ifndef __OLED_H__
#define __OLED_H__

#include "usart/usart128.h"

typedef struct OLED OLED;

typedef enum {
	OLED_DISABLED,
	OLED_ENABLED,
} OLEDState;

#define OLED_MAX_MESSAGE_LENGTH	64
#define OLED_EOL		'\n'

#define OLED_MSG_START_TOKEN	'$'
#define OLED_MSG_START_T_POS	0
#define OLED_MSG_LENGTH_POS	1
#define OLED_MSG_CMD_POS	2
#define OLED_MSG_ARGS_START	3

#define OLED_POWER_ICON_EXT	0x00
#define OLED_POWER_ICON_L0	0x01
#define OLED_POWER_ICON_L1	0x02
#define OLED_POWER_ICON_L2	0x03
#define OLED_POWER_ICON_L3	0x04
#define OLED_POWER_ICON_L4	0x05

#define OLED_RSSI_ICON_NS	0x00
#define OLED_RSSI_ICON_L1	0x01
#define OLED_RSSI_ICON_L2	0x02
#define OLED_RSSI_ICON_L3	0x03
#define OLED_RSSI_ICON_WARN	0x04

#define OLED_ICON_SHOW		0x01
#define OLED_ICON_HIDE		0x00
#define OLED_ICON_POWER		0x01
#define OLED_ICON_RSSI		0x02
#define OLED_ICON_TRANSMISSION	0x03

typedef enum {
	OLED_CMD_NOOP		= 0x00,
	OLED_CMD_SHOW		= 0x01,
	OLED_CMD_POWER_LEVEL	= 0x02,
	OLED_CMD_RSSI_LEVEL	= 0x03,
	OLED_CMD_CLEAR_MSG_AREA	= 0x04,
	OLED_CMD_CLEAR_SEL_AREA	= 0x05,
	OLED_CMD_SET_MESSAGE	= 0x06,
	OLED_CMD_DISP_MESSAGE	= 0x07,
	OLED_CMD_WRITE_MESSAGE	= 0x08,
	OLED_CMD_CONTRAST	= 0x09,
	OLED_CMD_CLR_SCREEN	= 0x0A,
	OLED_CMD_SET_NAME	= 0x0B,
	OLED_CMD_LAST,
} OLEDCommand;

OLED *OLEDCreate(struct usart_dev *uart);
void OLEDRelease(OLED *oled);
void OLEDDisable(OLED *oled);
void OLEDEnable(OLED *oled);
uint8_t OLEDConstructMessage(OLED *oled, OLEDCommand command, ...);
uint8_t OLEDWaitForResponse(OLED *oled);
uint8_t OLEDSendMessageNoBlock(OLED *oled);
uint8_t OLEDSendMessage(OLED *oled);
char * OLEDRetrieveMessage(OLED *oled);

#define OLEDSendCommand(__oled, __cmd, ...) 					\
	do {									\
		if (OLEDConstructMessage((__oled), (__cmd), __VA_ARGS__))	\
			OLEDSendMessage((__oled));				\
	} while(0)

#endif /* __OLED_H__ */
