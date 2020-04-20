#pragma once

#include <inttypes.h>
#include "defines.h"
#include "simple_print.h"

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00
#define DISPLAY_FUNCTION LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS

class LiquidCrystal_Prusa : public SimplePrint {
public:
	using SimplePrint::print;
	using SimplePrint::print_P;
	LiquidCrystal_Prusa();

	void begin();
	void reinit();
	void clear();
	void home();

	void noDisplay();
	void display();
	void noBlink();
	void blink();
	void noCursor();
	void cursor();
	void scrollDisplayLeft();
	void scrollDisplayRight();
	void leftToRight();
	void rightToLeft();
	void autoscroll();
	void noAutoscroll();

	void createChar(uint8_t, const uint8_t*);
	void setCursor(uint8_t, uint8_t);
	void command(uint8_t);
	void write(uint8_t);
	static void setBrightness(uint8_t);

	void print(uint8_t number, uint8_t col, uint8_t row, uint8_t denom = 100, unsigned char filler = ' ');
	void print(float number, uint8_t col, uint8_t row);
	void printTime(uint16_t time, uint8_t col, uint8_t row);
	void print(const char* str, uint8_t col, uint8_t row);
	void print_P(const char* str, uint8_t col, uint8_t row);
	void clearLine(uint8_t row);

private:
	void send(uint8_t, uint8_t);
	void write4bits(uint8_t);
	void pulseEnable();

	uint8_t _displaycontrol;
	uint8_t _displaymode;

	uint8_t _data_pins[4] = { LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7 };
	uint8_t _row_offsets[4] = { 0x00, 0x40, 0x14, 0x54 };
};

#define ESC_2J		"\x1b[2J"
#define ESC_25h		"\x1b[?25h"
#define ESC_25l		"\x1b[?25l"
#define ESC_H(c,r)	"\x1b["#r";"#c"H"

extern LiquidCrystal_Prusa lcd;
