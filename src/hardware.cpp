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

const int16_t chamber_temp_table_raw[34] PROGMEM = {
	25, 29, 34, 40, 46, 54, 64, 75, 88, 105, 124, 146, 173, 204, 241, 282, 330, 382, 439, 500,
	563, 625, 687, 744, 796, 842, 882, 915, 941, 963, 979, 992, 1001, 1008
};

const int16_t uvled_temp_table_raw[34] PROGMEM = {
	73, 83, 95, 109, 125, 144, 165, 189, 217, 248, 284, 323, 366, 412, 462, 514, 567, 620, 673, 723,
	770, 813, 851, 885, 913, 937, 957, 973, 986, 995, 1003, 1009, 1013, 1016
};


uint16_t Hardware::fan_rpm[3] = {0, 0, 0};
volatile uint8_t Hardware::fan_tacho_count[3] = {0, 0, 0};
volatile uint8_t Hardware::microstep_control(FAST_SPEED_START);
float Hardware::chamber_temp(0.0);
float Hardware::uvled_temp(0.0);
MCP Hardware::outputchip(0, 8);
Trinamic_TMC2130 Hardware::myStepper(CS_PIN);
uint8_t Hardware::lcd_encoder_bits(0);
volatile int8_t Hardware::rotary_diff(0);
uint8_t Hardware::target_accel_period(FAST_SPEED_START);
uint8_t Hardware::fan_duty[3] = {0, 0, 0};
uint8_t Hardware::fan_pwm_pins[2] = {FAN1_PWM_PIN, FAN2_PWM_PIN};
uint8_t Hardware::fan_enable_pins[2] = {FAN1_PIN, FAN2_PIN};
uint8_t Hardware::fans_target_temp(0);
uint8_t Hardware::fan_errors(0);
unsigned long Hardware::accel_us_last(0);
unsigned long Hardware::fans_us_last(0);
unsigned long Hardware::adc_us_last(0);
unsigned long Hardware::button_timer(0);
double Hardware::PI_summ_err(0.0);
bool Hardware::do_acceleration(false);
bool Hardware::cover_closed(false);
bool Hardware::tank_inserted(false);
bool Hardware::button_active(false);
bool Hardware::long_press_active(false);
bool Hardware::heater_error(false);
bool Hardware::adc_channel(false);


Hardware::Hardware() {

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

void Hardware::read_adc() {
	float value;
	if (adc_channel) {
		value = interpolate_i16_ylin_P(read_adc_raw(THERM_READ_PIN) >> 2, 34, uvled_temp_table_raw, 1250, -50) / 10.0;
		uvled_temp = config.SI_unit_system ? value : celsius2fahrenheit(value);
	} else {
		value = interpolate_i16_ylin_P(read_adc_raw(THERM_READ_PIN) >> 2, 34, chamber_temp_table_raw, 1250, -50) / 10.0;
		chamber_temp = config.SI_unit_system ? value : celsius2fahrenheit(value);
	}
	adc_channel ^= 1;
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

void Hardware::run_motor() {
	// enable stepper timer
	TIMSK3 |= (1 << OCIE3A);
	// enable stepper
	outputchip.digitalWrite(EN_PIN, LOW);
}

void Hardware::stop_motor() {
	// disable stepper timer
	TIMSK3 = 0;
	// disable stepper
	outputchip.digitalWrite(EN_PIN, HIGH);
}

void Hardware::speed_configuration(uint8_t speed, bool slow_mode, bool gear_shifting) {
	if (slow_mode) {
		myStepper.set_IHOLD_IRUN(10, 10, 0);
		myStepper.set_mres(256);
		microstep_control = map(speed, 1, 10, MIN_SLOW_SPEED, MAX_SLOW_SPEED);
	} else {
		myStepper.set_IHOLD_IRUN(31, 31, 5);
		myStepper.set_mres(16);
		if (gear_shifting) {
			microstep_control = map(speed, 1, 10, MIN_FAST_SPEED, MAX_FAST_SPEED);
		} else {
			target_accel_period = map(speed, 1, 10, MIN_FAST_SPEED, MAX_FAST_SPEED);
			microstep_control = FAST_SPEED_START;
			accel_us_last = millis();
		}
	}
	do_acceleration = !slow_mode;
}

void Hardware::acceleration() {
	if (microstep_control > target_accel_period) {
		// step is 5 to MIN_FAST_SPEED+5, then step is 1
		if (microstep_control > MIN_FAST_SPEED + 5)
			microstep_control -= 4;
		microstep_control--;
#ifdef SERIAL_COM_DEBUG
		SerialUSB.print("acceleration: ");
		SerialUSB.print(microstep_control);
		SerialUSB.print("->");
		SerialUSB.print(target_accel_period);
		SerialUSB.print("\r\n");
#endif
	} else {
		do_acceleration = false;
		myStepper.set_IHOLD_IRUN(10, 10, 5);
	}
}

void Hardware::run_heater() {
	outputchip.digitalWrite(FAN_HEAT_PIN, HIGH);
	fan_duty[2] = 100;
	wdt_enable(WDTO_4S);
}

void Hardware::stop_heater() {
	outputchip.digitalWrite(FAN_HEAT_PIN, LOW);
	fan_duty[2] = 0;
	wdt_disable();
}

bool Hardware::is_heater_running() {
	return (bool)fan_duty[2];
}

void Hardware::run_led() {
	analogWrite(LED_PWM_PIN, map(config.led_intensity, 0, 100, 0, 255));
	outputchip.digitalWrite(LED_RELE_PIN, HIGH);
}

void Hardware::stop_led() {
	outputchip.digitalWrite(LED_RELE_PIN, LOW);
	digitalWrite(LED_PWM_PIN, LOW);
}

bool Hardware::is_led_on() {
	// FIXME? this is not check if LED works or not
	return outputchip.digitalRead(LED_RELE_PIN) == HIGH;
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
	analogWrite(BEEPER, 220);
	delay(50);
	digitalWrite(BEEPER, LOW);
	delay(250);
	analogWrite(BEEPER, 220);
	delay(50);
	digitalWrite(BEEPER, LOW);
}

void Hardware::warning_beep() {
	analogWrite(BEEPER, 220);
	delay(50);
	digitalWrite(BEEPER, LOW);
	delay(250);
}

void Hardware::set_fans(uint8_t* duties) {
	fan_duty[0] = duties[0];
	fan_duty[1] = duties[1];
	fans_target_temp = 0;
	fans_duty();
}

void Hardware::set_target_temp(uint8_t target_temp) {
	fans_target_temp = target_temp;
	PI_summ_err = 0;
}

void Hardware::set_fan1_duty(uint8_t duty) {
	fans_duty(0, duty);
}

void Hardware::set_fan2_duty(uint8_t duty) {
	fans_duty(1, duty);
}

void Hardware::fans_duty() {
	for (uint8_t i = 0; i < 2; ++i) {
		fans_duty(i, fan_duty[i]);
	}
}

void Hardware::fans_duty(uint8_t fan, uint8_t duty) {
//	USB_PRINTP("fan ");
//	USB_PRINT(fan);
//	USB_PRINTP("->");
	if (duty) {
//		USB_PRINTLN(duty);
		analogWrite(fan_pwm_pins[fan], map(duty, 0, 100, 255, 0));
		outputchip.digitalWrite(fan_enable_pins[fan], HIGH);
	} else {
//		USB_PRINTLNP("OFF");
		outputchip.digitalWrite(fan_enable_pins[fan], LOW);
		digitalWrite(fan_pwm_pins[fan], LOW);
	}
}

void Hardware::fans_PI_regulator() {
	// FIXME this is not working as expected :(
//	USB_PRINTP("actual: ");
//	USB_PRINTLN(chamber_temp);
//	USB_PRINTP("target: ");
//	USB_PRINTLN(fans_target_temp);
	double err_value = chamber_temp - fans_target_temp;
//	USB_PRINTP("err: ");
//	USB_PRINTLN(err_value);
	PI_summ_err += err_value;
//	USB_PRINTP("sum: ");
//	USB_PRINTLN(PI_summ_err);

	if ((PI_summ_err > 10000) || (PI_summ_err < -10000)) {
		PI_summ_err = 10000;
	}

	double new_speed = P * err_value + I * PI_summ_err;		// TODO uint8_t?
//	USB_PRINTP("PI new value: ");
//	USB_PRINTLN(new_speed);
	if (new_speed > 100) {
		new_speed = 100;
	} else if (new_speed < MIN_FAN_SPEED) {
		new_speed = MIN_FAN_SPEED;
	}

	if (new_speed != fan_duty[0] || new_speed != fan_duty[1]) {
		fan_duty[0] = new_speed;
		fan_duty[1] = new_speed;
		fans_duty();
	}
}

void Hardware::fans_check() {
	for (uint8_t i = 0; i < 3; ++i) {
		if (fan_duty[i]) {
			fan_errors &= ~(1 << i);
			fan_errors |= !fan_tacho_count[i] << i;
			fan_rpm[i] = 60000 / FAN_CHECK_PERIOD * fan_tacho_count[i];
			fan_tacho_count[i] = 0;
//			USB_PRINT(i);
//			USB_PRINTP(": ");
//			USB_PRINTLN(fan_rpm[i]);
		} else {
			fan_rpm[i] = 0;
		}
	}
//	USB_PRINTP("fan_errors: ");
//	USB_PRINTLN(fan_errors);
}

bool Hardware::get_heater_error() {
	return heater_error;
}

uint8_t Hardware::get_fans_error() {
	return fan_errors;
}


Events Hardware::loop() {
	unsigned long us_now = millis();
	if (do_acceleration && us_now - accel_us_last >= 50) {
		accel_us_last = us_now;
		acceleration();
	}
	if (us_now - fans_us_last >= FAN_CHECK_PERIOD) {
		fans_us_last = us_now;
		fans_check();
	}
	if (us_now - adc_us_last >= 500) {
		adc_us_last = us_now;
		read_adc();
		if (fans_target_temp) {
			fans_PI_regulator();
		}
	}


	Events events = {false, false, false, false, false, false, false, false};

	if (heater_error)
		return events;

	// failed once, failed every time
	heater_error = (bool)(fan_errors & FAN3_ERROR_MASK);

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
			events.button_long_press = true;
		}
	} else if (button_active) {
		if (long_press_active) {
			long_press_active = false;
		} else {
			events.button_short_press = true;
		}
		button_active = false;
	}

	// rotary "click" is 4 "micro steps"
	if (rotary_diff > 3) {
		rotary_diff -= 4;
		events.control_up = true;
	} else if (rotary_diff < -3) {
		rotary_diff += 4;
		events.control_down = true;
	}

	if (config.sound_response && (events.button_long_press || events.button_short_press || events.control_down || events.control_up)) {
		echo();
	}

	return events;
}

Hardware hw;
