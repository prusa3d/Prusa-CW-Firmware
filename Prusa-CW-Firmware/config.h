
//3 ver. 0.3
//4 ver. 0.4

#define BOARD 4

//CW1 rev 0.3
#if (BOARD == 3)

// LCD
#define LCD_PINS_RS					12//82
#define LCD_PINS_ENABLE			    4//61
#define LCD_PINS_D4					10//59
#define LCD_PINS_D5					A0//70
#define LCD_PINS_D6					2//85
#define LCD_PINS_D7					A1//71
// STEPPER
#define CS_PIN 						7//41
#define EN_PIN 						MCP4//29 //enable (CFG6)
#define DIR_PIN 					-1//49 //direction
#define STEP_PIN 					5//9//37 //step
#define STEPPER_PWM_PIN 			-1//46
// controls
#define BTN_EN1						A2//72
#define BTN_EN2						A3//14
#define BTN_ENC						MCP1 //9	// the click
#define BEEPER						9 //84	// Beeper on AUX-4
#define LCD_PWM_PIN            		6
////////////////////////
// Setup Rotary Encoder Bit Values (for two pin encoders to indicate movement)
// These values are independent of which pins are used for EN_A and EN_B indications
// The rotary encoder part is also independent to the chipset used for the LCD
#define encrot0						0
#define encrot1						2
#define encrot2						3
#define encrot3						1

// UV LED rele
#define LED_RELE_PIN				MCP7 //8
#define LED_PWM_PIN					3 //8

// washer detect (pinda)
#define WASH_DETECT_PIN				MCP2 //10

// cover detect
#define COVER_OPEN_PIN				MCP3 //11//12

// FANs
#define FAN1_PIN				13//6
#define FAN2_PIN				11//6

#define FAN_HEAT_PIN				MCP11 //11	// HEATER_PIN

#define THERM_READ_PIN				A4



#define MCP1						1
#define MCP2						2
#define MCP3						3
#define MCP4						4
#define MCP5						5
#define MCP6						6
#define MCP7						7
#define MCP8						8
#define MCP9						9
#define MCP10						10
#define MCP11						11
#define MCP12						12
#define MCP13						13
#define MCP14						14
#define MCP15						15
#define MCP16						16

#endif

//CW1 rev 0.4
#if (BOARD == 4) 

// LCD
#define LCD_PINS_RS					12//82
#define LCD_PINS_ENABLE			    4//61
#define LCD_PINS_D4					10//59
#define LCD_PINS_D5					A0//70
#define LCD_PINS_D6					31//85
#define LCD_PINS_D7					A1//71
// STEPPER
#define CS_PIN 						7//41
#define EN_PIN 						MCP4//29 //enable (CFG6)
#define DIR_PIN 					-1//49 //direction
#define STEP_PIN 					5//9//37 //step
#define STEPPER_PWM_PIN 			-1//46
// controls
#define BTN_EN1						A2//72
#define BTN_EN2						A3//14
#define BTN_ENC						MCP1 //9	// the click
#define BEEPER						9 //84	// Beeper on AUX-4
#define LCD_PWM_PIN            		6
////////////////////////
// Setup Rotary Encoder Bit Values (for two pin encoders to indicate movement)
// These values are independent of which pins are used for EN_A and EN_B indications
// The rotary encoder part is also independent to the chipset used for the LCD
#define encrot0						0
#define encrot1						2
#define encrot2						3
#define encrot3						1

// UV LED rele
#define LED_RELE_PIN				MCP7 //8
#define LED_PWM_PIN					3 //8

// washer detect (pinda)
#define WASH_DETECT_PIN				MCP2 //10

// cover detect
#define COVER_OPEN_PIN				MCP3 //11//12

// FANs
#define FAN1_PIN				MCP12//6
#define FAN2_PIN				MCP13//6

#define FAN_HEAT_PIN				MCP11 //11	// HEATER_PIN

#define THERM_READ_PIN				A4



#define MCP1						1
#define MCP2						2
#define MCP3						3
#define MCP4						4
#define MCP5						5
#define MCP6						6
#define MCP7						7
#define MCP8						8
#define MCP9						9
#define MCP10						10
#define MCP11						11
#define MCP12						12
#define MCP13						13
#define MCP14						14
#define MCP15						15
#define MCP16						16

#endif
