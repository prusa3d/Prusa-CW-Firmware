#include <avr/wdt.h>
#include "hardware.h"

float celsius2fahrenheit(float celsius) {
	return 1.8 * celsius + 32;
}

float fahrenheit2celsius(float fahrenheit) {
	return (fahrenheit - 32) / 1.8;
}

hardware::hardware() :
		fan_tacho_count{0, 0, 0},
		therm1(THERM_READ_PIN, 5),
		outputchip(0, 8),
		myStepper(CS_PIN),
		lcd_encoder_bits(0),
		rotary_diff(0),
		fans_duty{0, 0, 0},
		fans_pwm_pins{FAN1_PWM_PIN, FAN2_PWM_PIN},
		fans_enable_pins{FAN1_PIN, FAN2_PIN},
		fan_tacho_last_count{0, 0, 0},
		rpm_fan_counter(0),
		fan_errors(0),
		button_timer(0),
		button_active(false),
		long_press_active(false) {

	outputchip.begin();
	outputchip.pinMode(0B0000000010010111);
	outputchip.pullupMode(0B0000000010000011);

	// controls
	pinMode(BTN_EN1, INPUT_PULLUP);
	pinMode(BTN_EN2, INPUT_PULLUP);
	pinMode(BEEPER, OUTPUT);

	// motor
	pinMode(STEP_PIN, OUTPUT);

	// led
	pinMode(LED_PWM_PIN, OUTPUT);

	// FAN control
	pinMode(FAN1_PWM_PIN, OUTPUT);
	pinMode(FAN2_PWM_PIN, OUTPUT);

	stop_led();
	stop_motor();
	stop_heater();

	// stepper driver init
	myStepper.init();
	myStepper.set_mres(16);					// ({1,2,4,8,16,32,64,128,256}) number of microsteps
	myStepper.set_IHOLD_IRUN(10, 10, 0);	// ([0-31],[0-31],[0-5]) sets all currents to maximum
	myStepper.set_I_scale_analog(0);		// ({0,1}) 0: I_REF internal, 1: sets I_REF to AIN
	myStepper.set_tbl(1);					// ([0-3]) set comparator blank time to 16, 24, 36 or 54 clocks, 1 or 2 is recommended
	myStepper.set_toff(8);					// ([0-15]) 0: driver disable, 1: use only with TBL>2, 2-15: off time setting during slow decay phase
	myStepper.set_en_pwm_mode(1);			// 0: driver disable PWM mode, 1: driver enable PWM mode

	cover_closed = is_cover_closed();
	tank_inserted = is_tank_inserted();
}

float hardware::therm1_read() {
	outputchip.digitalWrite(ANALOG_SWITCH_A, LOW);
	outputchip.digitalWrite(ANALOG_SWITCH_B, LOW);
	return therm1.analog2temp();
}

void hardware::run_motor() {
	// enable stepper timer
	TIMSK3 |= (1 << OCIE3A);
	// enable stepper
	outputchip.digitalWrite(EN_PIN, LOW);
}

void hardware::stop_motor() {
	// disable stepper timer
	TIMSK3 = 0;
	// disable stepper
	outputchip.digitalWrite(EN_PIN, HIGH);
}

void hardware::motor_configuration(bool curing_mode) {
	if (curing_mode) {
		myStepper.set_IHOLD_IRUN(10, 10, 0);
		myStepper.set_mres(256);
	} else {
		myStepper.set_IHOLD_IRUN(31, 31, 5);
		myStepper.set_mres(16);
	}
}

void hardware::motor_noaccel_settings() {
	myStepper.set_IHOLD_IRUN(10, 10, 5);
}

void hardware::run_heater() {
	outputchip.digitalWrite(FAN_HEAT_PIN, HIGH);
	fans_duty[2] = 100;
	wdt_enable(WDTO_4S);
}

void hardware::stop_heater() {
	outputchip.digitalWrite(FAN_HEAT_PIN, LOW);
	fans_duty[2] = 0;
	wdt_disable();
}

bool hardware::is_heater_running() {
	return (bool)fans_duty[2];
}

void hardware::run_led(uint8_t pwm) {
	analogWrite(LED_PWM_PIN, map(pwm, 0, 100, 0, 255));
	outputchip.digitalWrite(LED_RELE_PIN, HIGH);
}

void hardware::stop_led() {
	outputchip.digitalWrite(LED_RELE_PIN, LOW);
	digitalWrite(LED_PWM_PIN, LOW);
}

bool hardware::is_led_on() {
	return outputchip.digitalRead(LED_RELE_PIN) == HIGH;
}

bool hardware::is_cover_closed() {
	return outputchip.digitalRead(COVER_OPEN_PIN) == LOW;
}

bool hardware::is_tank_inserted() {
	return outputchip.digitalRead(WASH_DETECT_PIN) == LOW;
}

void hardware::echo() {
	for (uint8_t i = 0; i < 10; ++i) {
		digitalWrite(BEEPER, HIGH);
		delayMicroseconds(100);
		digitalWrite(BEEPER, LOW);
		delayMicroseconds(100);
	}
}

void hardware::beep() {
	analogWrite(BEEPER, 220);
	delay(50);
	digitalWrite(BEEPER, LOW);
	delay(250);
	analogWrite(BEEPER, 220);
	delay(50);
	digitalWrite(BEEPER, LOW);
}

void hardware::warning_beep() {
	analogWrite(BEEPER, 220);
	delay(50);
	digitalWrite(BEEPER, LOW);
	delay(250);
}

void hardware::read_encoder() {
	uint8_t enc = 0;
	if (digitalRead(BTN_EN1) == HIGH) {
		enc |= B01;
	}
	if (digitalRead(BTN_EN2) == HIGH) {
		enc |= B10;
	}
	if (enc != lcd_encoder_bits) {
		switch (enc) {
			case encrot0:
				if (lcd_encoder_bits == encrot3) {
					rotary_diff++;
				} else if (lcd_encoder_bits == encrot1) {
					rotary_diff--;
				}
				break;
			case encrot1:
				if (lcd_encoder_bits == encrot0) {
					rotary_diff++;
				} else if (lcd_encoder_bits == encrot2) {
					rotary_diff--;
				}
				break;
			case encrot2:
				if (lcd_encoder_bits == encrot1) {
					rotary_diff++;
				} else if (lcd_encoder_bits == encrot3) {
					rotary_diff--;
				}
				break;
			case encrot3:
				if (lcd_encoder_bits == encrot2) {
					rotary_diff++;
				} else if (lcd_encoder_bits == encrot0) {
					rotary_diff--;
				}
				break;
		}
		lcd_encoder_bits = enc;
		if (rotary_diff > 124)
			rotary_diff = 124;
		else if (rotary_diff < -124)
			rotary_diff = -124;
	}
}

void hardware::set_fans_duty(uint8_t* duties) {
	for (uint8_t i = 0; i < 2; ++i) {
		if (duties[i] != fans_duty[i]) {
			fans_duty[i] = duties[i];
			if (fans_duty[i]) {
				analogWrite(fans_pwm_pins[i], map(fans_duty[i], 0, 100, 255, 0));
				outputchip.digitalWrite(fans_enable_pins[i], HIGH);
			} else {
				outputchip.digitalWrite(fans_enable_pins[i], LOW);
				digitalWrite(fans_pwm_pins[i], LOW);
			}
		}
	}
}

void hardware::fan_rpm() {
	if (++rpm_fan_counter % 500 == 0) {
		for (uint8_t i = 0; i < 3; ++i) {
			if (fans_duty[i]) {
				fan_errors &= ~(1 << i);
				fan_errors |= (fan_tacho_count[i] <= fan_tacho_last_count[i]) << i;
				if (fan_tacho_count[i] >= 10000) {
					fan_tacho_count[i] = 0;
				}
				fan_tacho_last_count[i] = fan_tacho_count[i];
			}
		}
		rpm_fan_counter = 0;
/*
#ifdef SERIAL_COM_DEBUG
	SerialUSB.print("fan_errors: ");
	SerialUSB.print(fan_errors);
	SerialUSB.print("\r\n");
#endif
*/
	}
}

bool hardware::get_heater_error() {
	return (bool)(fan_errors & FAN3_ERROR_MASK);
}

uint8_t hardware::get_fans_error() {
	return fan_errors;
}

Events hardware::get_events() {
	Events events = {false, false, false, false, false, false, false, false};

	if (get_heater_error())
		return events;

	// cover
	bool cover_closed_now = is_cover_closed();
	if (cover_closed_now != cover_closed) {
		if (cover_closed_now)
			events.cover_closed = true;
		else
			events.cover_opened = true;
		cover_closed = cover_closed_now;
	}

	// tank
	bool tank_inserted_now = is_tank_inserted();
	if (tank_inserted_now != tank_inserted) {
		if (tank_inserted_now)
			events.tank_inserted = true;
		else
			events.tank_removed = true;
		tank_inserted = tank_inserted_now;
	}

	// button
	if (outputchip.digitalRead(BTN_ENC) == LOW) {
		if (!button_active) {
			button_active = true;
			button_timer = millis();
		} else if (!long_press_active && millis() - button_timer > LONG_PRESS_TIME) {
			long_press_active = true;
		}
	} else if (button_active) {
		if (long_press_active) {
			long_press_active = false;
			events.button_long_press = true;
		} else {
			events.button_short_press = true;
		}
		button_active = false;
	}

	// rotary "click" is 4 "micro steps"
	if (rotary_diff > 3) {
		rotary_diff -= 4;
		events.control_down = true;
	} else if (rotary_diff < -3) {
		rotary_diff += 4;
		events.control_up = true;
	}

	return events;
}
