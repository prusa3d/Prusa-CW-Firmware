#include <avr/wdt.h>

#include "hardware.h"
#include "intpol.h"
#include "config.h"

float celsius2fahrenheit(float celsius) {
	return 1.8 * celsius + 32;
}

float fahrenheit2celsius(float fahrenheit) {
	return (fahrenheit - 32) / 1.8;
}

uint8_t round_short(float temp) {
	return (uint8_t)round(temp);
}

float get_configured_temp(float temp) {
	return config.SI_unit_system ? temp : celsius2fahrenheit(temp);
}


const int16_t chamber_temp_table_raw[34] PROGMEM = {
	25, 29, 34, 40, 46, 54, 64, 75, 88, 105, 124, 146, 173, 204, 241, 282, 330, 382, 439, 500,
	563, 625, 687, 744, 796, 842, 882, 915, 941, 963, 979, 992, 1001, 1008
};

const int16_t uvled_temp_table_raw[34] PROGMEM = {
	73, 83, 95, 109, 125, 144, 165, 189, 217, 248, 284, 323, 366, 412, 462, 514, 567, 620, 673, 723,
	770, 813, 851, 885, 913, 937, 957, 973, 986, 995, 1003, 1009, 1013, 1016
};


Hardware::Hardware(uint16_t model_magic)
:
	model_magic(model_magic),
	fan_rpm{0, 0, 0},
	fan_tacho_count{0, 0, 0},
	microstep_control(FAST_SPEED_START),
	chamber_temp_celsius(0.0),
	uvled_temp_celsius(0.0),
	heater_error(false),
	disable_controls(false),
	outputchip(0, 8),
	stepper(CS_PIN),
	lcd_encoder_bits(0),
	rotary_diff(0),
	target_accel_period(FAST_SPEED_START),
	fan_speed{0, 0},
	chamber_target_temp(0),
	accel_ms_last(0),
	one_second_ms_last(0),
	heating_started_ms(0),
	button_timer(0),
	heating_in_progress(false),
	do_acceleration(false),
	cover_closed(false),
	tank_inserted(false),
	button_active(false),
	long_press_active(false),
	adc_channel(false),
	fans_forced(false)
{

	outputchip.begin();
	outputchip.pinMode(0B0000000010010111);
	outputchip.pullupMode(0B0000000010000011);
	outputchip.digitalWrite(ANALOG_SWITCH_A, LOW);
	outputchip.digitalWrite(ANALOG_SWITCH_B, LOW);

	// controls
	pinMode(BTN_EN1, INPUT_PULLUP);
	pinMode(BTN_EN2, INPUT_PULLUP);
	pinMode(BEEPER, OUTPUT);

	// motor
	pinMode(EN_PIN, OUTPUT);
	pinMode(DIR_PIN, OUTPUT);
	pinMode(STEP_PIN, OUTPUT);

	// led
	pinMode(LED_PWM_PIN, OUTPUT);

	// FAN control
	pinMode(FAN1_PWM_PIN, OUTPUT);
	pinMode(FAN2_PWM_PIN, OUTPUT);

	// temperature ADC
	pinMode(THERM_READ_PIN, INPUT);

	stop_led();
	stop_motor();
	stop_heater();

	// stepper driver init
	stepper.init();
	stepper.set_mres(16);				// ({1,2,4,8,16,32,64,128,256}) number of microsteps
	stepper.set_IHOLD_IRUN(10, 10, 0);	// ([0-31],[0-31],[0-5]) sets all currents to maximum
	stepper.set_I_scale_analog(0);		// ({0,1}) 0: I_REF internal, 1: sets I_REF to AIN
	stepper.set_tbl(1);					// ([0-3]) set comparator blank time to 16, 24, 36 or 54 clocks, 1 or 2 is recommended
	stepper.set_toff(8);				// ([0-15]) 0: driver disable, 1: use only with TBL>2, 2-15: off time setting during slow decay phase
	stepper.set_en_pwm_mode(1);			// 0: driver disable PWM mode, 1: driver enable PWM mode

	cover_closed = is_cover_closed();
	tank_inserted = is_tank_inserted();
}

void Hardware::read_adc() {
	if (adc_channel) {
		uvled_temp_celsius = interpolate_i16_ylin_P(read_adc_raw(THERM_READ_PIN) >> 2, 34, uvled_temp_table_raw, 1250, -50) / 10.0;
		USB_PRINTP("UV: ");
		USB_PRINTLN(uvled_temp_celsius);
	} else {
		chamber_temp_celsius = adjust_chamber_temp(interpolate_i16_ylin_P(read_adc_raw(THERM_READ_PIN) >> 2, 34, chamber_temp_table_raw, 1250, -50));
		USB_PRINTP("CH: ");
		USB_PRINTLN(chamber_temp_celsius);
	}
	adc_channel = !adc_channel;
	outputchip.digitalWrite(ANALOG_SWITCH_A, adc_channel);
}

int16_t Hardware::read_adc_raw(uint8_t pin) {
	int16_t raw = 0;
	for (uint8_t i = 0; i < ADC_OVRSAMPL; ++i) {
		raw += analogRead(pin);
	}
	return raw;
}

void Hardware::encoder_read() {
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

void Hardware::run_motor(bool direction) {
	TIMSK3 |= (1 << OCIE3A);	// enable stepper timer
	outputchip.digitalWrite(DIR_PIN, direction);
	enable_stepper();
}

void Hardware::stop_motor() {
	TIMSK3 = 0;					// disable stepper timer
	disable_stepper();
}

void Hardware::enable_stepper() {
	outputchip.digitalWrite(EN_PIN, LOW);
}

void Hardware::disable_stepper() {
	outputchip.digitalWrite(EN_PIN, HIGH);
}

void Hardware::speed_configuration(uint8_t speed, bool fast_mode, bool gear_shifting) {
	if (fast_mode) {
		stepper.set_IHOLD_IRUN(31, 31, 5);
		stepper.set_mres(16);
		if (gear_shifting) {
			microstep_control = map(speed, 1, 10, MIN_FAST_SPEED, MAX_FAST_SPEED);
		} else {
			target_accel_period = map(speed, 1, 10, MIN_FAST_SPEED, MAX_FAST_SPEED);
			microstep_control = FAST_SPEED_START;
			accel_ms_last = millis();
		}
	} else {
		stepper.set_IHOLD_IRUN(10, 10, 0);
		stepper.set_mres(256);
		microstep_control = map(speed, 1, 10, MIN_SLOW_SPEED, MAX_SLOW_SPEED);
	}
	do_acceleration = fast_mode && !gear_shifting;
}

void Hardware::acceleration() {
	if (microstep_control > target_accel_period) {
		// step is 5 to MIN_FAST_SPEED+5, then step is 1
		if (microstep_control > MIN_FAST_SPEED + 5)
			microstep_control -= 4;
		microstep_control--;
	} else {
		do_acceleration = false;
		stepper.set_IHOLD_IRUN(10, 10, 5);
	}
}

void Hardware::run_heater() {
	chamber_temp_celsius = 0.0;
	heating_started_ms = millis();
	heating_in_progress = true;
	wdt_enable(WDTO_4S);
}

void Hardware::stop_heater() {
	heating_in_progress = false;
	wdt_disable();
}

void Hardware::run_led() {
	analogWrite(LED_PWM_PIN, map(config.led_intensity, 0, 100, 0, 255));
	outputchip.digitalWrite(LED_RELE_PIN, HIGH);
}

void Hardware::stop_led() {
	outputchip.digitalWrite(LED_RELE_PIN, LOW);
	digitalWrite(LED_PWM_PIN, LOW);
}

bool Hardware::is_cover_closed() {
	return outputchip.digitalRead(COVER_OPEN_PIN) == LOW;
}

bool Hardware::is_tank_inserted() {
	return outputchip.digitalRead(WASH_DETECT_PIN) == LOW;
}

void Hardware::echo() {
	for (uint8_t i = 0; i < 10; ++i) {
		digitalWrite(BEEPER, HIGH);
		delayMicroseconds(100);
		digitalWrite(BEEPER, LOW);
		delayMicroseconds(100);
	}
}

void Hardware::beep() {
	warning_beep();
	warning_beep();
}

void Hardware::warning_beep() {
	analogWrite(BEEPER, 220);
	delay(50);
	digitalWrite(BEEPER, LOW);
	delay(250);
}

void Hardware::set_chamber_target_temp(uint8_t target_temp) {
	chamber_target_temp = target_temp;
}

void Hardware::force_fan_speed(uint8_t fan_speed_1, uint8_t fan_speed_2) {
	if (fan_speed_1 || fan_speed_2) {
		fans_forced = true;
		set_fan_speed(0, fan_speed_1);
		set_fan_speed(1, fan_speed_2);
	} else {
		fans_forced = false;
	}
}

void Hardware::set_fan_speed(uint8_t fan, uint8_t speed) {
	if (fan_speed[fan] == speed) {
		return;
	}
	fan_speed[fan] = speed;
	USB_PRINTP("fan ");
	USB_PRINT(fan);
	USB_PRINTP("->");
	uint8_t fan_pwm_pins[2] = {FAN1_PWM_PIN, FAN2_PWM_PIN};
	uint8_t fan_enable_pins[2] = {FAN1_PIN, FAN2_PIN};
	if (speed) {
		USB_PRINTLN(speed);
		analogWrite(fan_pwm_pins[fan], map(speed, 0, 100, 255, 0));
		outputchip.digitalWrite(fan_enable_pins[fan], HIGH);
	} else {
		USB_PRINTLNP("OFF");
		outputchip.digitalWrite(fan_enable_pins[fan], LOW);
		digitalWrite(fan_pwm_pins[fan], LOW);
	}
}

void Hardware::cooling() {
	float error = uvled_temp_celsius - OPTIMAL_TEMP;
	uint8_t new_speed = error > 0.0 ? round_short((error < 10.0 ? error : 10.0) * 10) : 0;
/*
	USB_PRINTP("actual t.: ");
	USB_PRINTLN(uvled_temp_celsius);
	USB_PRINTP("target t.: ");
	USB_PRINTLN(OPTIMAL_TEMP);
	USB_PRINTP("error: ");
	USB_PRINTLN(error);
	USB_PRINTP("new_speed: ");
	USB_PRINTLN(new_speed);
*/
	if (new_speed < MIN_FAN_SPEED || heating_in_progress) {
		new_speed = MIN_FAN_SPEED;
	}
	set_cooling_speed(new_speed);
}

void Hardware::fans_rpm() {
	for (uint8_t i = 0; i < 3; ++i) {
		fan_rpm[i] = 60 * fan_tacho_count[i];
//		USB_PRINT(i);
//		USB_PRINTP(": ");
//		USB_PRINT(fan_tacho_count[i]);
//		USB_PRINTP("->");
//		USB_PRINTLN(fan_rpm[i]);
		fan_tacho_count[i] = 0;
	}
}

uint8_t Hardware::loop() {
	unsigned long ms_now = millis();
	if (do_acceleration && ms_now - accel_ms_last >= 50) {
		accel_ms_last = ms_now;
		acceleration();
	}
	if (ms_now - one_second_ms_last >= 1000) {
		one_second_ms_last = ms_now;
		read_adc();
		fans_rpm();
		// after uvled_temp_celsius has been read
		if (!adc_channel and !fans_forced) {
			cooling();
		}
		heating();
	}

	uint8_t events = 0;
	if (disable_controls) {
		return events;
	}

	heater_error = heating_in_progress && handle_heater();
	if (heater_error) {
		stop_heater();
	}

	// cover
	bool cover_closed_now = is_cover_closed();
	if (cover_closed_now != cover_closed) {
		if (cover_closed_now)
			events |= EVENT_COVER_CLOSED;
		else
			events |= EVENT_COVER_OPENED;
		cover_closed = cover_closed_now;
	}

	// tank
	bool tank_inserted_now = is_tank_inserted();
	if (tank_inserted_now != tank_inserted) {
		if (tank_inserted_now)
			events |= EVENT_TANK_INSERTED;
		else
			events |= EVENT_TANK_REMOVED;
		tank_inserted = tank_inserted_now;
	}

	// button
	if (outputchip.digitalRead(BTN_ENC) == LOW) {
		if (!button_active) {
			button_active = true;
			button_timer = millis();
		} else if (!long_press_active && millis() - button_timer > LONG_PRESS_TIME) {
			long_press_active = true;
			events |= EVENT_BUTTON_LONG_PRESS;
		}
	} else if (button_active) {
		if (long_press_active) {
			long_press_active = false;
		} else {
			events |= EVENT_BUTTON_SHORT_PRESS;
		}
		button_active = false;
	}

	// rotary "click" is 4 "micro steps"
	if (rotary_diff > 3) {
		rotary_diff -= 4;
		events |= EVENT_CONTROL_UP;
	} else if (rotary_diff < -3) {
		rotary_diff += 4;
		events |= EVENT_CONTROL_DOWN;
	}

	if (config.sound_response && events & (EVENT_BUTTON_LONG_PRESS | EVENT_BUTTON_SHORT_PRESS | EVENT_CONTROL_DOWN | EVENT_CONTROL_UP)) {
		echo();
	}

	return events;
}
