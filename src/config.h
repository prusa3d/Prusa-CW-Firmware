//#define SERIAL_COM_DEBUG	//!< Set up for communication through USB

#define LAYOUT_TIME_X		3
#define LAYOUT_TIME_Y		2
#define LAYOUT_TIME_LT		1
#define LAYOUT_TIME_GT		8
#define LAYOUT_TEMP_X		12
#define LAYOUT_TEMP_Y		2

// various constants
#define FAN_FREQUENCY		70.0 // Hz
#define LED_DELAY			1000
#define LONG_PRESS_TIME		1000
#define	P					10	// 0.5
#define I					0.001
#define MIN_TARGET_TEMP_C	20
#define MAX_TARGET_TEMP_C	40
#define MIN_TARGET_TEMP_F	MIN_TARGET_TEMP_C * 1.8 + 32
#define MAX_TARGET_TEMP_F	MAX_TARGET_TEMP_C * 1.8 + 32

#define MCP1	1
#define MCP2	2
#define MCP3	3
#define MCP4	4
#define MCP5	5
#define MCP6	6
#define MCP7	7
#define MCP8	8
#define MCP9	9
#define MCP10	10
#define MCP11	11
#define MCP12	12
#define MCP13	13
#define MCP14	14
#define MCP15	15
#define MCP16	16

#include "pins_board_4.h"
