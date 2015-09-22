#ifndef __POWER_H__
#define __POWER_H__

#define MAIN_POWER_ON		GPIO_HIGH
#define MAIN_POWER_OFF		GPIO_LOW

typedef enum {
	POWER_MODE_SUSPENDED,
	POWER_MODE_AWAKE,
} PowerMode;

void systemInitPower(void);
void systemMainPower(uint8_t state);
uint8_t systemUsbHostConnected(void);

#endif /* __POWER_H__ */
