// CW1 board rev 0.4

// LCD
#define LCD_PINS_RS					12
#define LCD_PINS_ENABLE				4
#define LCD_PINS_D4					10
#define LCD_PINS_D5					18
#define LCD_PINS_D6					31
#define LCD_PINS_D7					19
#define LCD_PWM_PIN					6
// STEPPER
#define CS_PIN						7
#define EN_PIN						MCP_A3
#define DIR_PIN						MCP_A5
#define STEP_PIN					5
// controls
#define BTN_EN1						20
#define BTN_EN2						21
#define BTN_ENC						MCP_A0
#define BEEPER						9
////////////////////////
// Setup Rotary Encoder Bit Values (for two pin encoders to indicate movement)
// These values are independent of which pins are used for EN_A and EN_B indications
// The rotary encoder part is also independent to the chipset used for the LCD
#define encrot0						0
#define encrot1						2
#define encrot2						3
#define encrot3						1

#define LED_RELE_PIN				MCP_A6
#define LED_PWM_PIN					3
#define WASH_DETECT_PIN				MCP_A1
#define COVER_OPEN_PIN				MCP_A2

#define FAN1_PIN					MCP_B3
#define FAN1_PWM_PIN				13
#define FAN1_TACHO_PIN				0

#define FAN2_PIN					MCP_B4
#define FAN2_PWM_PIN				11
#define FAN2_TACHO_PIN				2

#define FAN_HEAT_PIN				MCP_B2
#define FAN_HEAT_TACHO_PIN			1

#define ANALOG_SWITCH_A				MCP_B1
#define ANALOG_SWITCH_B				MCP_B0
#define THERM_READ_PIN				22
