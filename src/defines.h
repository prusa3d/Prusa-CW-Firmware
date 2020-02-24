//#define SERIAL_COM_DEBUG	//!< Set up for communication through USB

#define LAYOUT_TIME_X		3
#define LAYOUT_TIME_Y		2
#define LAYOUT_TIME_LT		1
#define LAYOUT_TIME_GT		8
#define LAYOUT_TEMP_X		12
#define LAYOUT_TEMP_Y		2

#define FAN1_ERROR_MASK		B001
#define FAN2_ERROR_MASK		B010
#define FAN3_ERROR_MASK		B100

// various constants
#define LED_DELAY			1000
#define LONG_PRESS_TIME		1000
#define	P					10	// 0.5
#define I					0.001
#define MIN_TARGET_TEMP_C	20
#define MAX_TARGET_TEMP_C	40
#define MIN_TARGET_TEMP_F	MIN_TARGET_TEMP_C * 1.8 + 32
#define MAX_TARGET_TEMP_F	MAX_TARGET_TEMP_C * 1.8 + 32
// motor speeds (smaller is faster)
#define WASHING_ROTATION_START	200
#define MIN_WASHING_SPEED		70
#define MAX_WASHING_SPEED		16
#define MIN_CURING_SPEED		220
#define MAX_CURING_SPEED		25

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
