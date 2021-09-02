#include <avr/wdt.h>

#include "device.h"
#include "config.h"
#include "ui.h"
#include "states.h"
#include "LiquidCrystal_Prusa.h"

const char* pgmstr_serial_number = reinterpret_cast<const char*>(SN_ADDRESS);
volatile uint16_t* const bootKeyPtr = (volatile uint16_t *)(RAMEND - 1);
static volatile uint16_t bootKeyPtrVal __attribute__ ((section (".noinit")));

const uint8_t Back[8] PROGMEM = {
	B00100,
	B01110,
	B11111,
	B00100,
	B11100,
	B00000,
	B00000,
	B00000
};

const uint8_t Right[8] PROGMEM = {
	B00000,
	B00100,
	B00010,
	B11111,
	B00010,
	B00100,
	B00000,
	B00000
};

const uint8_t Backslash[8] PROGMEM = {
	B00000,
	B10000,
	B01000,
	B00100,
	B00010,
	B00001,
	B00000,
	B00000
};

const uint8_t Play[8] PROGMEM = {
	B00000,
	B01000,
	B01100,
	B01110,
	B01100,
	B01000,
	B00000,
	B00000
};

const uint8_t Stop[8] PROGMEM = {
	B00000,
	B10001,
	B01010,
	B00100,
	B01010,
	B10001,
	B00000,
	B00000
};


// 1ms timer for read controls
void setupTimer0() {
	OCR0A = 0xAF;
	TIMSK0 |= _BV(OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
	hw.encoder_read();
	hw.one_ms_tick();
}

// timer for stepper move
void setupTimer3() {
	// Clear registers
	TCCR3A = 0;
	TCCR3B = 0;
	TCNT3 = 0;
	// 1 Hz (16000000/((15624+1)*1024))
	OCR3A = 200;
	// CTC
	TCCR3B |= (1 << WGM32);
	// Prescaler 1024
	TCCR3B |= (1 << CS31) | (1 << CS30);
	// Start with interrupt disabled
	TIMSK3 = 0;
}

ISR(TIMER3_COMPA_vect) {
	OCR3A = hw.microstep_control;
	digitalWrite(STEP_PIN, HIGH);
	delayMicroseconds(2);
	digitalWrite(STEP_PIN, LOW);
	delayMicroseconds(2);
}

void fan_tacho1() {
	hw.fan_tacho_count[0]++;
}

void fan_tacho2() {
	hw.fan_tacho_count[1]++;
}

void fan_tacho3() {
	hw.fan_tacho_count[2]++;
}

void setup() {
	uint16_t model_magic = pgm_read_word_near(SN_ADDRESS);
	if (hw.model_magic != model_magic) {
		lcd.clear();
		lcd.print_P(pgmstr_wrong_model, (20 - strlen_P(pgmstr_wrong_model)) / 2, 1);
		while(true) {
			delay(1000);
		}
	}

	read_config();

	lcd.setBrightness(config.lcd_brightness);
	lcd.createChar(BACKSLASH_CHAR, Backslash);
	lcd.createChar(BACK_CHAR, Back);
	lcd.createChar(RIGHT_CHAR, Right);
	lcd.createChar(PLAY_CHAR, Play);
	lcd.createChar(STOP_CHAR, Stop);

	// FAN tachos
	pinMode(FAN1_TACHO_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(FAN1_TACHO_PIN), fan_tacho1, RISING);

	pinMode(FAN2_TACHO_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(FAN2_TACHO_PIN), fan_tacho2, RISING);

	pinMode(FAN_HEAT_TACHO_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(FAN_HEAT_TACHO_PIN), fan_tacho3, RISING);

	noInterrupts();
	setupTimer0();
	setupTimer3();
	interrupts();
	States::init();
	UI::init();
}

void loop() {
	if (*bootKeyPtr != MAGIC_KEY) {
		wdt_reset();
	}

	uint8_t events = hw.loop();
	States::loop(events);
	UI::loop(events);
}


#if 0
//! @brief Get reset flags
//! @return value of MCU Status Register - MCUSR as it was backed up by bootloader
static uint8_t get_reset_flags() {
	return bootKeyPtrVal;
}
#endif

#define ATTR_INIT_SECTION(SectionIndex) __attribute__ ((used, naked, section (".init" #SectionIndex )))
void get_key_from_boot(void) ATTR_INIT_SECTION(3);

//! @brief Save the value of the boot key memory before it is overwritten
//!
//! Do not call this function, it is placed in one of the initialization sections,
//! which executes automatically before the main function of the application.
//! Refer to the avr-libc manual for more information on the initialization sections.
void get_key_from_boot(void) {
	bootKeyPtrVal = *bootKeyPtr;
}
