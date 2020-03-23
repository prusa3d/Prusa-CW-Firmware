#include <stdio.h>
#include <avr/wdt.h>

#include "version.h"
#include "defines.h"
#include "main.h"
#include "hardware.h"
#include "Countimer.h"
#include "USBCore.h"
#include "Selftest.h"
#include "i18n.h"
#include "config.h"
#include "ui.h"
#include "states.h"

const char* pgmstr_serial_number = reinterpret_cast<const char*>(0x7fe0); // see SN_LENGTH!!!
volatile uint16_t* const bootKeyPtr = (volatile uint16_t *)(RAMEND - 1);
static volatile uint16_t bootKeyPtrVal __attribute__ ((section (".noinit")));

uint8_t Back[8] = {
	B00100,
	B01110,
	B11111,
	B00100,
	B11100,
	B00000,
	B00000,
	B00000
};

uint8_t Right[8] = {
	B00000,
	B00100,
	B00010,
	B11111,
	B00010,
	B00100,
	B00000,
	B00000
};

uint8_t Backslash[8] = {
	B00000,
	B10000,
	B01000,
	B00100,
	B00010,
	B00001,
	B00000,
	B00000
};

uint8_t Play[8] = {
	B00000,
	B01000,
	B01100,
	B01110,
	B01100,
	B01000,
	B00000,
	B00000
};

uint8_t Stop[8] = {
	B00000,
	B10001,
	B01010,
	B00100,
	B01010,
	B10001,
	B00000,
	B00000
};


//Selftest selftest;

//const uint8_t max_preheat_run_time = 30;

//bool mode_flag = true;	//helping var for selftesting

//uint8_t last_menu_position = 0;
//uint8_t last_seconds = 0;

//unsigned long time_now = 0;

//unsigned long us_last = 0;
//unsigned long led_time_now = 0;

//long remain = 0;

//bool curing_mode = false;
//bool drying_mode = false;
//bool paused = false;
//bool cover_open = false;
//bool gastro_pan = false;
//bool paused_time = false;
//bool led_start = false;

//bool preheat_complete = false;
//bool pid_mode = false;

// 1ms timer for read controls
void setupTimer0() {
	OCR0A = 0xAF;
	TIMSK0 |= _BV(OCIE0A);
}

ISR(TIMER0_COMPA_vect) {
	hw.encoder_read();
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

/*
void run_stop() {
	pid_mode = false;
	state = HOME;
	paused = false;
	cover_open = false;

	hw.stop_motor();
	hw.stop_heater();
	hw.stop_led();
	tDown.stop();
	tUp.stop();
}
*/

void setup() {

	read_config();

	// FAN tachos
	pinMode(FAN1_TACHO_PIN, INPUT_PULLUP);
	pinMode(FAN2_TACHO_PIN, INPUT_PULLUP);
	pinMode(FAN_HEAT_TACHO_PIN, INPUT_PULLUP);
	attachInterrupt(digitalPinToInterrupt(FAN1_TACHO_PIN), fan_tacho1, RISING);
	attachInterrupt(digitalPinToInterrupt(FAN2_TACHO_PIN), fan_tacho2, RISING);
	attachInterrupt(digitalPinToInterrupt(FAN_HEAT_TACHO_PIN), fan_tacho3, RISING);

	noInterrupts();
	setupTimer0();
	setupTimer3();
	interrupts();

	lcd.createChar(BACKSLASH_CHAR, Backslash);
	lcd.createChar(BACK_CHAR, Back);
	lcd.createChar(RIGHT_CHAR, Right);
	lcd.createChar(PLAY_CHAR, Play);
	lcd.createChar(STOP_CHAR, Stop);

	States::init();
	UI::init();
}
/*
void redraw_selftest_vals() {
	if (selftest.phase == 3 && selftest.vent_test != true) {
		lcd.print(selftest.fan_tacho[0], 7, 1);
		lcd.print(selftest.fan_tacho[1], 7, 2);
	}
	if (selftest.phase == 5 && selftest.heater_test != true) {
		lcd.print(hw.chamber_temp, 5, 1);
		lcd.print_P(config.SI_unit_system ? pgmstr_celsius : pgmstr_fahrenheit);
	}
	if (selftest.phase == 6 && selftest.rotation_test != true) {
		lcd.print((uint8_t)mode_flag, 12, 1);
		lcd.setCursor(14,1);
		if (mode_flag) {
			if (speed_control.curing_speed <= 11)
				lcd.print((uint8_t)(speed_control.curing_speed - 1));
		} else {
			if (speed_control.washing_speed <= 11)
				lcd.print(uint8_t(speed_control.washing_speed - 1));
		}
	}
	if (selftest.phase == 3 || selftest.phase == 4 || selftest.phase == 5) {
		uint8_t lcd_min = selftest.tCountDown.getCurrentMinutes();
		uint8_t lcd_sec = selftest.tCountDown.getCurrentSeconds();
		lcd.printTime(lcd_min, lcd_sec, 7, 3);
	}
}
*/

void loop() {
	if (*bootKeyPtr != MAGIC_KEY) {
		wdt_reset();
	}

	Events events = hw.loop();
	States::loop(events);
	UI::loop(events);
}


/*
	tDown.run();
	tUp.run();

	if (state == SELFTEST) {

		selftest.tCountDown.run();
		static unsigned long ms_last_count = millis();

		if ((millis() - ms_last_count) >= 1000) {
			ms_last_count = millis();
			//redraw_menu = true;
		}

		switch (selftest.phase) {
			case 1:
				selftest.measured_state = !hw.is_cover_closed();
				//redraw_menu = selftest.universal_pin_test();
				break;
			case 2:
				selftest.measured_state = !hw.is_tank_inserted();
				//redraw_menu = selftest.universal_pin_test();
				break;
			case 3:
				selftest.ventilation_test(hw.get_fans_error());
				//hw.set_fans_duty(selftest.fans_speed);
				break;
			case 4:
				selftest.cover_down = hw.is_cover_closed();
				if (selftest.cover_down) {
					if (selftest.is_first_loop()) {
						hw.run_led(config.led_pwm_value);
					}
					if (hw.is_led_on()) {
						if (selftest.led_test == false) {
							selftest.LED_test();
						} else {
							hw.stop_led();
						}
					} else {
						if (selftest.isCounterRunning) {
							selftest.fail_flag = true;
							selftest.tCountDown.stop();
							selftest.isCounterRunning = false;
							selftest.led_test = true;
							hw.stop_led();
						}
					}
				} else {
					if (selftest.isCounterRunning)
						selftest.tCountDown.pause();
				}
				break;

			case 5:
				if (!selftest.heater_test) {
					if (!hw.is_tank_inserted()) {
						selftest.fans_speed[0] = 10;
						if (hw.is_cover_closed()) {
							selftest.fans_speed[1] = 10;
							if (selftest.is_first_loop()) {
								pid_mode = true;
								hw.run_heater();
								//hw.set_fans_duty(fans_menu_speed);
							}
							selftest.heat_test(hw.get_heater_error());
						} else {
							selftest.fans_speed[1] = 0;
							if (selftest.isCounterRunning)
								selftest.heat_test(hw.get_heater_error());
						}
					} else {
						selftest.fans_speed[0] = 0;
					}
				} else if (hw.is_heater_running()) {
					hw.stop_heater();
					//hw.set_fans_duty(fans_menu_speed);
					pid_mode = false;
				}
				break;

			case 6:
				if (!selftest.rotation_test && selftest.motor_rotation_timer()) {
					if (selftest.is_first_loop()) {
						speed_control.curing_speed = 1;
						speed_control.washing_speed = 1;
						speed_control.speed_configuration(mode_flag);
						hw.run_motor();
						selftest.set_first_loop(false);
					} else {
						if (speed_control.curing_speed <= 10 && speed_control.washing_speed <= 10) {
							if (!mode_flag) {
								uint8_t backup = speed_control.microstep_control;	 //needed for smooth gear-up of the motor
								speed_control.speed_configuration(mode_flag);
								speed_control.microstep_control = backup;
							} else {
								speed_control.speed_configuration(mode_flag);
							}
						}
					}

					if (mode_flag)
						speed_control.curing_speed++;
					else
						speed_control.washing_speed++;

					if (mode_flag && speed_control.curing_speed > 11) {
						hw.stop_motor();
						selftest.clean_up();
						speed_control.curing_speed = 1;	 //default value
						mode_flag = false;
					}
					if (!mode_flag && speed_control.washing_speed > 11) {
						hw.stop_motor();
						speed_control.washing_speed = 10; //default value
						selftest.rotation_test = true;
					}
				}
				break;

			default:
				break;
			}
	}

	if (hw.get_heater_error()) {
		//tDown.stop();
		//tUp.stop();
		hw.stop_heater();
		hw.stop_motor();
		//hw.set_fans_duty(fans_menu_speed);
		lcd.print_P(pgmstr_heater_error, 1, 0);
		lcd.print_P(pgmstr_please_restart, 1, 2);
		state = ERROR;
	}

TODO long press event
			switch (state) {
				case INFO:
					state = SELFTEST;
					menu_move(true);
					break;
				default:
					break;
			}
*/
/*
	// FIXME is this needed to fix ESD shock? Any better solution?
	if (millis() > time_now + 5500) {
		if (state == HOME || state == TEMPERATURES || state == SOUND_SETTINGS || state == SPEED_STATE) {
			last_menu_position = menu_position;
		}

		time_now = millis();
		lcd.reinit();
		lcd.createChar(BACKSLASH_CHAR, Backslash);
		lcd.createChar(BACK_CHAR, Back);
		lcd.createChar(RIGHT_CHAR, Right);
		lcd.createChar(PLAY_CHAR, Play);
		lcd.createChar(STOP_CHAR, Stop);
		menu_move(false);

		if (state == HOME || state == TEMPERATURES || state == SOUND_SETTINGS || state == SPEED_STATE) {
			menu_position = last_menu_position;
			print_menu_cursor(menu_position);
		}
	}
*/
/*
void menu_move(bool sound_echo) {

	switch (state) {
		case RUN_MENU:
			if (!curing_mode && paused_time) {
				generic_menu_P(3, paused ? pgmstr_ipa_tank_removed : pgmstr_pause, pgmstr_stop, pgmstr_back);
			} else {
				generic_menu_P(3, paused ? pgmstr_continue : pgmstr_pause, pgmstr_stop, pgmstr_back);
			}
			break;

		case RUNNING:
			lcd.setCursor(1, 0);
			if (curing_mode) {
				if (paused) {
					if (config.heat_to_target_temp || (config.curing_machine_mode == 3) || (preheat_complete == false)) {
						lcd.print_P(paused ? pgmstr_paused : drying_mode ? pgmstr_heating : pgmstr_curing);
					} else {
						lcd.print_P(paused ? pgmstr_paused : drying_mode ? pgmstr_drying : pgmstr_curing);
					}
				} else {
					if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
						if (!preheat_complete) {
							lcd.print_P(cover_open ? pgmstr_cover_is_open : drying_mode ? pgmstr_heating : pgmstr_curing);
						} else {
							lcd.print_P(cover_open ? pgmstr_cover_is_open : drying_mode ? pgmstr_drying : pgmstr_curing);
						}
					} else {
						lcd.print_P(cover_open ? pgmstr_cover_is_open : drying_mode ? pgmstr_drying : pgmstr_curing);
					}
				}
			} else {
				lcd.print_P(cover_open ? pgmstr_cover_is_open : (paused ? pgmstr_paused : pgmstr_washing));
			}
			break;

		case SELFTEST:
			if (selftest.phase == 0) {
				generic_menu_P(2, pgmstr_back, pgmstr_selftest);
				lcd_print_back();
				lcd_print_right(1);
			} else if (selftest.phase == 1) {
				lcd.setCursor(1,0);
				if (!selftest.cover_test) {
					if (!selftest.measured_state)
						lcd.print_P(pgmstr_open_cover);
					else
						lcd.print_P(pgmstr_close_cover);
				} else {
					lcd.print_P(pgmstr_test_success);
					lcd.print_P(pgmstr_press2continue, 1, 2);
				}
			} else if (selftest.phase == 2) {
				lcd.setCursor(1,0);
				if (!selftest.tank_test) {
					if (!selftest.measured_state)
						lcd.print_P(pgmstr_remove_tank);
					else
						lcd.print_P(pgmstr_insert_tank);
				} else {
					lcd.print_P(pgmstr_test_success);
					lcd.print_P(pgmstr_press2continue, 1, 2);
				}
			} else {
				lcd.print_P(selftest.print(), 1, 0);
				lcd.print_P(pgmstr_press2continue, 1, 2);
			}

			if (selftest.phase == 3 && !selftest.vent_test) {
				lcd.print_P(pgmstr_fan1_test, 1, 1);
				lcd.print_P(pgmstr_fan2_test, 1, 2);
			}
			if (selftest.phase == 6 && !selftest.rotation_test) {
				lcd.print_P(pgmstr_mode_gear, 1, 1);
				lcd.print_P(pgmstr_slash, 13, 1);
			}
			redraw_selftest_vals();
			break;

		default:
			break;
	}
}
*/
/*
void machine_running() {

	if (curing_mode) {
		// curing
		if (!hw.is_cover_closed()) {
			if (!cover_open) {
				if (!paused) {
					lcd.print_P(pgmstr_cover_is_open, 1, 0);
				}
				cover_open = true;
			}
		} else {
			if (cover_open) {
				cover_open = false;
			}
		}

		if (cover_open == true) {
			hw.stop_motor();
			hw.speed_configuration(curing_mode);
			hw.stop_heater();
			hw.stop_led();
		} else if (!paused) { // cover closed
			hw.run_motor();
			unsigned long us_now = millis();
			remain -= us_now - us_last ;
			us_last = us_now;
		}

		switch (config.curing_machine_mode) {
			case 3: // Resin preheat
				if (!preheat_complete) {
					if (tUp.isCounterCompleted() == false) {
						if (!drying_mode) {
							drying_mode = true;
						}
						start_drying();
					} else {
						if (drying_mode) {
							//drying_mode = false;
							preheat_complete = true;
							remain = config.resin_preheat_run_time;
							tDown.setCounter(0, remain, 0);
							tDown.start();
						}
					}
				} else {
					if (tDown.isCounterCompleted() == false) {
						if (!drying_mode) {
							drying_mode = true;
						}
						start_drying();
					} else {
						if (drying_mode) {
							drying_mode = false;
						}
						preheat_complete = false;
						stop_curing_drying();
					}
				}

				break;

			case 2: // Drying
				if (!config.heat_to_target_temp) {
					if (tDown.isCounterCompleted() == false) {
						if (!drying_mode) {
							drying_mode = true;
						}
						start_drying();
					} else {
						if (drying_mode) {
							drying_mode = false;
						}
						stop_curing_drying();
					}
				} else {
					if (!preheat_complete) {
						if (tUp.isCounterCompleted() == false) {
							if (!drying_mode) {
								drying_mode = true;
							}
							start_drying();
						} else {
							if (drying_mode) {
								//drying_mode = false;
								preheat_complete = true;
								remain = config.drying_run_time;
								tDown.setCounter(0, remain, 0);
								tDown.start();
							}
						}
					} else {
						if (tDown.isCounterCompleted() == false) {
							if (!drying_mode) {
								drying_mode = true;
							}
							start_drying();
						} else {
							if (drying_mode) {
								drying_mode = false;
							}
							preheat_complete = false;
							stop_curing_drying();
						}
					}
				}
				break;

			case 1: // Curing
				if (tDown.isCounterCompleted() == false) {
					if (drying_mode) {
						drying_mode = false;
					}
					start_curing();
				} else {
					stop_curing_drying();
				}
				break;

			case 0: // Drying and curing
			default:
				if (!config.heat_to_target_temp) {
					if ((drying_mode == true) && (tDown.isCounterCompleted() == false)) {
						start_drying();
					} else {
						if (drying_mode) {
							drying_mode = false;
							remain = config.curing_run_time;
							tDown.setCounter(0, remain, 0);
							tDown.start();
							menu_move(true);
						}
						if (tDown.isCounterCompleted() == false) {
							start_curing();
							//hw.set_fans_duty(config.fans_curing_speed);
						} else {
							stop_curing_drying();
						}
					}
				} else {
					if (!preheat_complete) {
						if (tUp.isCounterCompleted() == false) {
							if (!drying_mode) {
								drying_mode = true;
							}
							start_drying();
						} else {
							if (drying_mode) {
								//drying_mode = false;
								preheat_complete = true;
								remain = config.drying_run_time;
								tDown.setCounter(0, remain, 0);
								tDown.start();
							}
						}
					} else {
						if ((drying_mode == true) && (tDown.isCounterCompleted() == false)) {
							start_drying();
						} else {
							if (drying_mode) {
								drying_mode = false;
								remain = config.curing_run_time;
								tDown.setCounter(0, remain, 0);
								tDown.start();
								menu_move(true);
							}
							if (tDown.isCounterCompleted() == false) {
								start_curing();
							} else {
								stop_curing_drying();
							}
						}
					}
				}
				break;
		}
	} else {
		// washing
		start_washing();
	}
}
*/
/*
void button_press() {
	if (config.sound_response) {
		hw.echo();
	}
	switch (state) {
		case HOME:
			switch (menu_position) {
				case 0:
					if (curing_mode) {
						// curing mode
						hw.speed_configuration(curing_mode);

						switch (config.curing_machine_mode) {
							case 3: // Resin preheat
								pid_mode = true;
								remain = max_preheat_run_time;
								tUp.setCounter(0, remain, 0);
								tUp.start();
								hw.stop_led();
								drying_mode = true;
								preheat_complete = false;
								break;

							case 2: // Drying
								preheat_complete = false;
								drying_mode = true;
								if (!config.heat_to_target_temp) {
									pid_mode = false;
									remain = config.drying_run_time;
									tDown.setCounter(0, remain, 0);
									tDown.start();
									//hw.set_fans_duty(config.fans_drying_speed);
								} else {
									pid_mode = true;
									remain = max_preheat_run_time;
									tUp.setCounter(0, remain, 0);
									tUp.start();
								}
								hw.stop_led();
								drying_mode = true;
								break;

							case 1: // Curing
								pid_mode = false;
								remain = config.curing_run_time;
								tDown.setCounter(0, remain, 0);
								tDown.start();
								//hw.set_fans_duty(config.fans_curing_speed);
								drying_mode = false;
								break;

							case 0: // Drying and curing
							default:
								tDown.stop();
								tUp.stop();
								preheat_complete = false;
								drying_mode = true;
								if (!config.heat_to_target_temp) {
									pid_mode = false;
									remain = config.drying_run_time;
									tDown.setCounter(0, remain, 0);
									tDown.start();
									//hw.set_fans_duty(config.fans_drying_speed);
								} else {
									pid_mode = true;
									remain = max_preheat_run_time;
									tUp.setCounter(0, remain, 0);
									tUp.start();
								}
								break;
						}
					} else {
						// washing mode
						drying_mode = false;
						hw.speed_configuration(curing_mode);
						hw.run_motor();
						remain = config.washing_run_time;
						tDown.setCounter(0, remain, 0);
						tDown.start();
						//hw.set_fans_duty(fans_washing_speed);
					}

					us_last = millis();

					state = RUNNING;
					menu_move(true);
					break;

				default:
					break;
			}
			break;

		case RUN_MENU:
			switch (menu_position) {
				case 0:
					if (curing_mode) {
						//curing mode
						// FIXME WTF curing with gastro pan?
						if (!gastro_pan) {
							paused ^= 1;
							if (paused) {
								hw.stop_motor();
								hw.stop_heater();
								//hw.set_fans_duty(fans_menu_speed);
							} else {
								hw.speed_configuration(curing_mode);
								hw.run_motor();
								//hw.set_fans_duty(config.fans_curing_speed);
							}
							state = RUNNING;
						}
					} else {
						//washing mode
						if (!gastro_pan) {
							paused ^= 1;
							if (paused) {
								hw.stop_motor();
								hw.stop_heater();
							} else {
								hw.speed_configuration(curing_mode);
								hw.run_motor();
								//hw.set_fans_duty(fans_washing_speed);
							}
							state = RUNNING;
						}
					}
					break;

				case 1:
					run_stop();
					break;

				case 2:
					state = RUNNING;
					break;

				default:
					break;
			}
			break;

		case SELFTEST:
			switch (selftest.phase) {
				case 0:
					if (menu_position) {
						selftest.phase++;
					} else {
						state = INFO;
					}
					break;

				case 1:
					if (selftest.cover_test == true) {
						selftest.phase++;
						selftest.clean_up();
					}
					break;
				case 2:
					if (selftest.tank_test == true) {
						selftest.phase++;
						selftest.clean_up();
					}
					break;
				case 3:
					if (selftest.vent_test) {
						if (selftest.fail_flag == true) {
							state = HOME;
						} else {
							selftest.phase++;
						}
						selftest.clean_up();
					}
					break;
				case 4:
					if (selftest.led_test) {
						if (selftest.fail_flag == true) {
							state = HOME;
						} else {
							selftest.phase++;
						}
						selftest.clean_up();
					}
						break;
				case 5:
					if (selftest.heater_test) {
						if (selftest.fail_flag == true) {
							state = HOME;
						} else {
							selftest.phase++;
						}
						selftest.clean_up();
					}
					break;

				case 6:
					if (selftest.rotation_test == true) {
						selftest.phase = 0;
						selftest.clean_up();
					}
					break;

				default:
					break;
			}
			break;

		default:
			break;
	}

	menu_move(true);
	//delay(475);
}
*/
/*
void start_drying() {
	if (cover_open == false && paused == false) {
		if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
			if (!preheat_complete) {
//				preheat(); // turn on heat fan
			} else {
				hw.run_heater();
			}
		} else {
			hw.run_heater();
		}
	}
	if (cover_open == true || paused == true) {
		if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
			if (!paused_time) {
				paused_time = true;
			}
			if (!preheat_complete) {
				tUp.pause();
			} else {
				tDown.pause();
			}

		} else {
			if (!paused_time) {
				paused_time = true;
			}
			tDown.pause();
		}
	} else {
		if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
			if (paused_time) {
				paused_time = false;
				menu_move(true);
			}
			if (!preheat_complete) {
				tUp.start();
			} else {
				tDown.start();
			}
		} else {
			if (paused_time) {
				paused_time = false;
				menu_move(true);
			}
			tDown.start();
		}
	}
	if (hw.is_tank_inserted()) {
		lcd.print_P(pgmstr_remove_tank, 1, 0);
		paused = true;
		if (!paused_time) {
			paused_time = true;
		}
		tDown.pause();
		hw.stop_heater();
		hw.stop_motor();
		if (!gastro_pan) {
			menu_move(true);
			gastro_pan = true;
		}
	} else {
		if (gastro_pan) {
			menu_move(true);
			gastro_pan = false;
		}
	}
	lcd_time_print();
}
*/
/*
void start_curing() {
	hw.stop_heater();
	if (cover_open == false && paused == false) {
		if (!led_start) {
			led_start = true;
			led_time_now = millis();
		}
		if (millis() > led_time_now + LED_DELAY) {
			hw.run_led(config.led_pwm_value);
		}
	} else {
		if (led_start) {
			hw.stop_led();
			led_start = false;
		}
	}
	if (cover_open == true || paused == true) {
		if (!paused_time) {
			paused_time = true;
		}
		tDown.pause();
	} else {
		if (paused_time) {
			paused_time = false;
			menu_move(true);
		}
		tDown.start();
	}
	if (hw.is_tank_inserted()) {
		lcd.print_P(pgmstr_remove_tank, 1, 0);
		paused = true;
		if (!paused_time) {
			paused_time = true;
		}
		tDown.pause();
		hw.stop_heater();
		hw.stop_motor();
		if (!gastro_pan) {
			menu_move(true);
			gastro_pan = true;
		}
	} else {
		if (gastro_pan) {
			menu_move(true);
			gastro_pan = false;
		}
	}
	lcd_time_print();
}
*/
/*
void start_washing() {
	if (cover_open) {
		cover_open = false;
	}
	if (!hw.is_tank_inserted()) {
		lcd.print_P(pgmstr_ipa_tank_removed, 1, 0);
		paused = true;
		if (!paused_time) {
			paused_time = true;
		}
		tDown.pause();
		hw.stop_motor();
		if (!gastro_pan) {
			menu_move(true);
			gastro_pan = true;
		}
	} else {
		if (paused_time) {
			paused_time = false;
		}
		if (gastro_pan) {
			menu_move(true);
			gastro_pan = false;
		}
	}
	if (tDown.isCounterCompleted() == false) {
		if (state == RUNNING) {
			if (!paused && hw.is_tank_inserted()) {
				hw.run_motor();
				tDown.start();
			} else {
				tDown.pause();
			}

			lcd_time_print();
		}
	} else {
		leave_action();
	}
}
*/
/*
void stop_curing_drying() {
	pid_mode = false;
	hw.stop_led();
	leave_action();
}

void leave_action() {
	hw.stop_motor();
	hw.stop_heater();

	//hw.set_fans_duty(fans_menu_speed);
	switch (config.finish_beep_mode) {
		case 2:
			hw.beep();
			state = CONFIRM;
			break;

		case 1:
			hw.beep();
			[[gnu::fallthrough]];
		default:
			state = HOME;
			break;
	}
	menu_move(true);

}
*/
//! @brief Display remaining time
/* DONE
void lcd_time_print() {
	static uint8_t running_count = 0;
	uint8_t mins;
	uint8_t secs;
	if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
		if (drying_mode) {
			if (!preheat_complete) {
				mins = tUp.getCurrentMinutes();
				secs = tUp.getCurrentSeconds();
			} else {
				mins = tDown.getCurrentMinutes();
				secs = tDown.getCurrentSeconds();
			}
		} else {
			mins = tDown.getCurrentMinutes();
			secs = tDown.getCurrentSeconds();
		}
	} else {
		mins = tDown.getCurrentMinutes();
		secs = tDown.getCurrentSeconds();
	}

	if (state == RUNNING && (secs != last_seconds || redraw_ms)) {
		last_seconds = secs;
		lcd.printTime(mins, secs, LAYOUT_TIME_X, LAYOUT_TIME_Y);

		if (!paused && !paused_time) {
			lcd.print_P(pgmstr_space, 19, 1);

			if (curing_mode) {
				if (hw.is_cover_closed()) {
					lcd.print(hw.chamber_temp, LAYOUT_TEMP_X, LAYOUT_TEMP_Y);
					lcd.print_P(config.SI_unit_system ? pgmstr_celsius : pgmstr_fahrenheit);
				}
			}

			lcd.setCursor(19, 0);
			uint8_t c = pgm_read_byte(pgmstr_progress + running_count);
			lcd.write(c);
		}

		if (++running_count > sizeof(pgmstr_progress)) {
			lcd_clear_time_boundaries();
			running_count = 0;
		}
	}
}
*/
/*
void preheat() {
	uint8_t tmpTargetTemp = config.curing_machine_mode == 3 ? config.resin_target_temp : config.target_temp;
	if (hw.chamber_temp < tmpTargetTemp) {
		hw.run_heater();
	} else {
		hw.stop_heater();
		tUp.setCounter(0, 0, 0);
	}
}
*/

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
