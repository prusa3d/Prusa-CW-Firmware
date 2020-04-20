#include "LiquidCrystal_Prusa.h"
#include "Arduino.h"
#include "i18n.h"

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
//		DL = 1; 8-bit interface data
//		N = 0; 1-line display
//		F = 0; 5x8 dot character font
// 3. Display on/off control:
//		D = 0; Display off
//		C = 0; Cursor off
//		B = 0; Blinking off
// 4. Entry mode set:
//		I/D = 1; Increment by 1
//		S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that it's in that state when a sketch starts (and the
// LiquidCrystal_Prusa constructor is called).
//
//	LCD_PINS_RS - LOW: command / HIGH: character.
//	LCD_PINS_ENABLE - activated by a HIGH pulse.
//	LCD_PWM_PIN - brightness control pin


LiquidCrystal_Prusa::LiquidCrystal_Prusa() : SimplePrint()
{
	pinMode(LCD_PINS_RS, OUTPUT);
	pinMode(LCD_PWM_PIN, OUTPUT);
	digitalWrite(LCD_PWM_PIN, HIGH);
	pinMode(LCD_PINS_ENABLE, OUTPUT);
	begin();
}

void LiquidCrystal_Prusa::begin() {
	// SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
	// according to datasheet, we need at least 40ms after power rises above 2.7V
	// before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
	delayMicroseconds(50000);
	// Now we pull both RS and R/W low to begin commands
	digitalWrite(LCD_PINS_RS, LOW);
	digitalWrite(LCD_PINS_ENABLE, LOW);

	// 4 bit mode
	// this is according to the hitachi HD44780 datasheet
	// figure 24, pg 46
	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03);
	delayMicroseconds(4500); // wait min 4.1ms

	// second try
	write4bits(0x03);
	delayMicroseconds(4500); // wait min 4.1ms

	// third go!
	write4bits(0x03);
	delayMicroseconds(150);

	// finally, set to 4-bit interface
	write4bits(0x02);

	// finally, set # lines, font size, etc.
	command(LCD_FUNCTIONSET | DISPLAY_FUNCTION);
	delayMicroseconds(60);
	// turn the display on with no cursor or blinking default
	_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
	display();
	delayMicroseconds(60);
	// clear it off
	clear();
	delayMicroseconds(3000);
	// Initialize to default text direction (for romance languages)
	_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
	// set the entry mode
	command(LCD_ENTRYMODESET | _displaymode);
	delayMicroseconds(60);
}


void LiquidCrystal_Prusa::reinit() {
	// we start in 8bit mode, try to set 4 bit mode
	write4bits(0x03);
	//delayMicroseconds(50); // wait min 4.1ms

	// second try
	write4bits(0x03);
	//delayMicroseconds(50); // wait min 4.1ms

	// third go!
	write4bits(0x03);
	//delayMicroseconds(50);

	// finally, set to 4-bit interface
	write4bits(0x02);

	// finally, set # lines, font size, etc.
	command(LCD_FUNCTIONSET | DISPLAY_FUNCTION);
	//delayMicroseconds(60);

	command(LCD_ENTRYMODESET | _displaymode);
	//delayMicroseconds(60);
	display();

	command(LCD_CURSORSHIFT | LCD_ENTRYSHIFTDECREMENT);
	command(LCD_CURSORSHIFT | LCD_ENTRYSHIFTDECREMENT);
	write(' ');
	write(' ');
}


/********** high level commands, for the user! */
void LiquidCrystal_Prusa::clear() {
	command(LCD_CLEARDISPLAY);	// clear display, set cursor position to zero
	delayMicroseconds(1600);	// this command takes a long time
}

void LiquidCrystal_Prusa::home() {
	command(LCD_RETURNHOME);	// set cursor position to zero
	delayMicroseconds(1600);	// this command takes a long time!
}

void LiquidCrystal_Prusa::setCursor(uint8_t col, uint8_t row) {
	command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void LiquidCrystal_Prusa::noDisplay() {
	_displaycontrol &= ~LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_Prusa::display() {
	_displaycontrol |= LCD_DISPLAYON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal_Prusa::noCursor() {
	_displaycontrol &= ~LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_Prusa::cursor() {
	_displaycontrol |= LCD_CURSORON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal_Prusa::noBlink() {
	_displaycontrol &= ~LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void LiquidCrystal_Prusa::blink() {
	_displaycontrol |= LCD_BLINKON;
	command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal_Prusa::scrollDisplayLeft(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidCrystal_Prusa::scrollDisplayRight(void) {
	command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal_Prusa::leftToRight(void) {
	_displaymode |= LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal_Prusa::rightToLeft(void) {
	_displaymode &= ~LCD_ENTRYLEFT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal_Prusa::autoscroll(void) {
	_displaymode |= LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal_Prusa::noAutoscroll(void) {
	_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
	command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal_Prusa::createChar(uint8_t location, const uint8_t* charmap) {
	location &= 0x7; // we only have 8 locations 0-7
	command(LCD_SETCGRAMADDR | (location << 3));
	for (uint8_t i = 0; i < 8; i++) {
		write(pgm_read_byte(charmap++));
	}
}

void LiquidCrystal_Prusa::setBrightness(uint8_t brightness) {
	analogWrite(LCD_PWM_PIN, map(brightness, 0, 100, 0, 255));
}

void LiquidCrystal_Prusa::print(uint8_t number, uint8_t col, uint8_t row, uint8_t denom, unsigned char filler) {
	setCursor(col, row);
	SimplePrint::print(number, denom, filler);
}

void LiquidCrystal_Prusa::print(float number, uint8_t col, uint8_t row) {
	setCursor(col, row);
	SimplePrint::print(number);
}

void LiquidCrystal_Prusa::printTime(uint16_t time, uint8_t col, uint8_t row) {
	setCursor(col, row);
	SimplePrint::printTime(time);
}

void LiquidCrystal_Prusa::print(const char *str, uint8_t col, uint8_t row) {
	setCursor(col, row);
	SimplePrint::print(str);
}

void LiquidCrystal_Prusa::print_P(const char *str, uint8_t col, uint8_t row) {
	setCursor(col, row);
	SimplePrint::print_P(str);
}

void LiquidCrystal_Prusa::clearLine(uint8_t row) {
	setCursor(0, row);
	for (uint8_t i = 0; i < DISPLAY_CHARS; i++) {
		write(' ');
	}
}

/*********** mid level commands, for sending data/cmds */

inline void LiquidCrystal_Prusa::command(uint8_t value) {
	send(value, LOW);
}

inline void LiquidCrystal_Prusa::write(uint8_t value) {
	send(value, HIGH);
}


/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LiquidCrystal_Prusa::send(uint8_t value, uint8_t mode) {
	digitalWrite(LCD_PINS_RS, mode);
	write4bits(value>>4);
	write4bits(value);
}

void LiquidCrystal_Prusa::pulseEnable(void) {
	digitalWrite(LCD_PINS_ENABLE, LOW);
	delayMicroseconds(1);
	digitalWrite(LCD_PINS_ENABLE, HIGH);
	delayMicroseconds(1);		// enable pulse must be >450ns
	digitalWrite(LCD_PINS_ENABLE, LOW);
	delayMicroseconds(50);		// commands need > 37us to settle
}

void LiquidCrystal_Prusa::write4bits(uint8_t value) {
	for (uint8_t i = 0; i < 4; i++) {
		pinMode(_data_pins[i], OUTPUT);
		digitalWrite(_data_pins[i], (value >> i) & 0x01);
	}
	pulseEnable();
}


LiquidCrystal_Prusa lcd;
