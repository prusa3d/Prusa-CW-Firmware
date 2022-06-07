#ifdef SERIAL_COM_DEBUG
#define USB_PRINT(x)		SerialUSB.print(x)
#define USB_PRINTLN(x)		SerialUSB.println(x)
#define USB_PRINTP(x)		SerialUSB.print_P(PSTR(x))
#define USB_PRINTLNP(x)		SerialUSB.println_P(PSTR(x))
#else
#define USB_PRINT(x)
#define USB_PRINTLN(x)
#define USB_PRINTP(x)
#define USB_PRINTLNP(x)
#endif

#define COUNT_ITEMS(x)		(sizeof(x)/sizeof(x[0]))

#define DISPLAY_CHARS		20
#define DISPLAY_LINES		4

#define LAYOUT_TIME_X		2
#define LAYOUT_TIME_Y		2
#define LAYOUT_TIME_LT		0
#define LAYOUT_TIME_GT		8
#define LAYOUT_INFO1_X		12
#define LAYOUT_INFO1_Y		2
#define LAYOUT_INFO2_X		1
#define LAYOUT_INFO2_Y		3

#define BACKSLASH_CHAR		0
#define BACK_CHAR			1
#define RIGHT_CHAR			2
#define PLAY_CHAR			3
#define STOP_CHAR			4

#define FAN1_ERROR_MASK		B001
#define FAN2_ERROR_MASK		B010
#define FAN3_ERROR_MASK		B100

// various constants
#define LED_DELAY			2000	// microseconds
#define LONG_PRESS_TIME		1000	// microseconds
#define MIN_TARGET_TEMP_C	20
#define MAX_TARGET_TEMP_C	60
#define MIN_TARGET_TEMP_F	MIN_TARGET_TEMP_C * 1.8 + 32
#define MAX_TARGET_TEMP_F	MAX_TARGET_TEMP_C * 1.8 + 32
#define MAX_WARMUP_RUNTIME	255		// minutes
#define MAX_CURING_RUNTIME	255		// minutes
#define MAX_DRYING_RUNTIME	255		// minutes
#define MAX_WASHING_RUNTIME	255		// minutes
#define MAX_PREHEAT_RUNTIME	255		// minutes
#define COOLDOWN_RUNTIME	10		// minutes
#define INC_DEC_TIME_STEP	30		// seconds
#define MIN_LED_INTENSITY	1		// 0-100 %
#define MIN_LCD_BRIGHTNESS	5		// 0-100 %
#define MIN_FAN_SPEED		30		// 0-100 %
#define SWITCH_TEST_COUNT	10
#define ROTATION_TEST_TIME	3		// minutes
#define FANS_TEST_TIME		2		// minutes
#define UVLED_TEST_MAX_TIME	10		// minutes
#define UVLED_MAX_TEMP		70		// celsius
#define OPTIMAL_TEMP		35.0	// celsius
#define TEST_TEMP			50		// celsius
#define HEATER_CHECK_DELAY	2500	// microseconds
#define SN_ADDRESS			0x7fe0
#define SN_LENGTH			16
#define MAX_MENU_DEPTH		5
#define ADC_OVRSAMPL		4
// motor speeds (smaller is faster)
#define FAST_SPEED_START	200
#define MIN_FAST_SPEED		70
#define MAX_FAST_SPEED		16
#define MIN_SLOW_SPEED		220
#define MAX_SLOW_SPEED		25
#define DIR_CHANGE_DELAY	2		// seconds

#define MCP_A0	1	// pin 21
#define MCP_A1	2	// pin 22
#define MCP_A2	3	// pin 23
#define MCP_A3	4	// pin 24
#define MCP_A4	5	// pin 25 - DIAG???
#define MCP_A5	6	// pin 26
#define MCP_A6	7	// pin 27
#define MCP_A7	8	// pin 28 - not connected
#define MCP_B0	9	// pin 1
#define MCP_B1	10	// pin 2
#define MCP_B2	11	// pin 3
#define MCP_B3	12	// pin 4
#define MCP_B4	13	// pin 5
#define MCP_B5	14	// pin 6 - not connected
#define MCP_B6	15	// pin 7 - not connected
#define MCP_B7	16	// pin 8 - not connected

#include "pins_board_4.h"
