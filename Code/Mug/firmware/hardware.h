#ifndef __HARDWARE_H__
#define __HARDWARE_H__

#define MUG_MAX_MESSAGE_SIZE	64

#define MSG_SWITCH_CONSOLE	"Switch to console?"
#define MSG_SHUTDOWN_NOTIFY	"Shutdown in %d\nseconds"
#define MSG_BTN_TO_CANCEL	"Press button to\n cancel"
#define MSG_SHUTDOWN_CANCELLED	"Shutdown canceled"
#define MSG_SHUTDOWN		"Going down"
#define MSG_TERMINAL_WELCOME	"Dark Roast Travel Mug configuration terminal\n\r"

#define CONFIG_DEFAULT_NAME	"foobar"
#define CONFIG_DEFAULT_ID	"1111"
#define CONFIG_DEFAULT_PAN	"1111"
#define CONFIG_DEFAULT_CHANNEL	"1A"

#define ZIGBEE_UART		USART_0
#define ZIGBEE_BAUDRATE		9600
#define ZIGBEE_RESET_PORT	PORT_C
#define ZIGBEE_RESET_PIN	1
#define ZIGBEE_SLEEP_RQ_PORT	PORT_C
#define ZIGBEE_SLEEP_RQ_PIN	4
#define ZIGBEE_ASSOCIATE_PORT	PORT_C
#define ZIGBEE_ASSOCIATE_PIN	7
#define ZIGBEE_POWER_MODE_PORT	PORT_C
#define ZIGBEE_POWER_MODE_PIN	6
#define ZIGBEE_GPIO_PORT	PORT_C
#define ZIGBEE_GPIO_PIN		5
#define ZIGBEE_RSSI_PORT	PORT_A
#define ZIGBEE_RSSI_PIN		3

#define OLED_CONSOLE_UART	USART_1
#define OLED_CONSOLE_BAUDRATE	19200
#define OLED_RESET_PORT		PORT_C
#define OLED_RESET_PIN		2

#define POWER_ONOFF_PORT	PORT_C
#define POWER_ONOFF_PIN		0
#define POWER_USBHOST_PORT	PORT_B
#define POWER_USBHOST_PIN	1

#define CONSOLE_SWITCH_PORT	PORT_D
#define CONSOLE_SWITCH_PIN	4
#define CONSOLE_ENABLE_PORT	PORT_D
#define CONSOLE_ENABLE_PIN	5

#define BUTTON_PORT		PORT_B
#define BUTTON_PIN		2
#define BUTTON_PRESS_SHORT_MIN	10
#define BUTTON_PRESS_SHORT_MAX	1000
#define BUTTON_PRESS_LONG_MIN	3000
#define BUTTON_PRESS_LONG_MAX	0	/* == 0xffff */

#define BATTERY_UPDATE_INTERVAL	1000
#define BATTERY_HYST_VAL	5
#define BATTERY_EXT_VAL		0
#define BATTERY_L1_VAL		600
#define BATTERY_L2_VAL		630
#define BATTERY_L3_VAL		680
#define BATTERY_L4_VAL		758

#define ACC_WINDOW_SIZE		10	/* derivative window */
#define ACC_FWA_RATIO		10	/* low pass filter ratio (percentages) */
#define ACC_SAMPLE_INTERVAL	20	/* ~ 50Hz */
#define ACC_ST_PORT		PORT_B
#define ACC_ST_PIN		0

#define MUX_ACC_X		0
#define MUX_ACC_Y		1
#define MUX_ACC_Z		2
#define MUX_RSSI		3
#define MUX_BATTERY_LEVEL	4


#endif
