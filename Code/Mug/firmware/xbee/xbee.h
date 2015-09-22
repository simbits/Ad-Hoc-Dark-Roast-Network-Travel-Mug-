#ifndef __XBEE_H__
#define __XBEE_H__

#include "usart/usart128.h"

#define XBEE_EOL		'\r'
#define XBEE_GUARD_TIME		1000	/* ms */

#define XBEE_DESTINATION	0xffff
#define XBEE_PAN_ID		0x1111
#define XBEE_CHANNEL		0x1a

#define XBEE_AT_CS		"+++"

#define XBEE_SLEEP		GPIO_HIGH
#define XBEE_WAKE		GPIO_LOW

#define EDA_MASK		0x0f
#define EDA_NO_ASSOCIATION	0x00
#define EDA_ASSOCIATE_ANY_PAN	(1 << 0)
#define EDA_ASSOCIATE_ANY_CH	(1 << 1)
#define EDA_ASSOCIATE_AUTO	(1 << 3)
#define EDA_PIN_WAKE_POLL	(1 << 4)

typedef enum {
	XBEE_END_DEVICE,
	XBEE_COORDINATOR
} XBeeCoordinatorMode;

typedef struct XBee XBee;

XBee *XBeeCreate(struct usart_dev *uart);
void XBeeRelease(XBee *xbee);

uint8_t XBeeWaitForResponse(XBee *xbee);
char XBeeRetrieveByte(XBee *xbee);
uint8_t XBeeSendMessageNoBlock(XBee *xbee, const char *msg);
uint8_t XBeeSendMessage(XBee *xbee, const char *msg);
void XBeeSendByte(XBee *xbee, char byte);
void XBeeEnterSleepMode(XBee *xbee);
void XBeeLeaveSleepMode(XBee *xbee);
void XBeeEnterCommandMode(XBee *xbee);
void XBeeLeaveCommandMode(XBee *xbee);
uint8_t XBeeHasData(XBee *xbee); 
char * XBeeRetrieveMessage(XBee *xbee); 
void XBeeSetDestination(XBee *xbee, const char *high, const char *low);
void XBeeSetMyId(XBee *xbee, const char *id);
void XBeeSetPanId(XBee *xbee, const char *id);
void XBeeSetChannel(XBee *xbee, const char *channel);
void XBeeSetCoordinatorMode(XBee *xbee, XBeeCoordinatorMode mode);
void XBeeSetEndDeviceAssociation(XBee *xbee, uint8_t mask);


#endif /* __XBEE_H__ */
