#pragma once

#include <stdint.h>

#define EEPROM_OFFSET	128
#define CONFIG_START	E2END + 1 - EEPROM_OFFSET

//! @brief legacy configuration store structure
//!
//! It is restored when magic read from eeprom equals magic "CURWA".
//! Do not change.
typedef struct {
	uint8_t washing_speed;
	uint8_t curing_speed;
	uint8_t washing_run_time;
	uint8_t curing_run_time;
	uint8_t finish_beep_mode;
	uint8_t drying_run_time;
	uint8_t sound_response;
	uint8_t curing_machine_mode;
	uint8_t heat_to_target_temp;
	uint8_t target_temp_celsius;
	uint8_t target_temp_fahrenheit;
	uint8_t SI_unit_system;
//	bool heater_failure;	this is not used any more and may be forgotten
} eeprom_v1_t;

//! @brief legacy configuration store structure
//!
//! It is restored when magic read from eeprom equals magic "CW1v2"
//! Do not change.
typedef struct {
	uint8_t washing_speed;
	uint8_t curing_speed;
	uint8_t washing_run_time;
	uint8_t curing_run_time;
	uint8_t finish_beep_mode;
	uint8_t drying_run_time;
	uint8_t sound_response;
	uint8_t curing_machine_mode;
	uint8_t heat_to_target_temp;
	uint8_t target_temp;
	uint8_t resin_target_temp;		// v1 change!
	uint8_t SI_unit_system;
	uint8_t resin_preheat_run_time;
	uint8_t led_intensity;
	uint8_t fans_menu_speed[2];
	uint8_t fans_washing_speed[2];
	uint8_t fans_drying_speed[2];
	uint8_t fans_curing_speed[2];
	uint8_t lcd_brightness;
} eeprom_v2_t;

//! @brief configuration store structure
//!
//! It is restored when magic read from eeprom equals magic "CW1v3"
//! Do not change.
typedef struct {
	uint8_t washing_speed;
	uint8_t curing_speed;
	uint8_t washing_run_time;
	uint8_t curing_run_time;
	uint8_t finish_beep_mode;
	uint8_t drying_run_time;
	uint8_t sound_response;
	uint8_t curing_machine_mode;
	uint8_t heat_to_target_temp;
	uint8_t target_temp;
	uint8_t resin_target_temp;
	uint8_t SI_unit_system;
	uint8_t resin_preheat_run_time;
	uint8_t led_intensity;
	uint8_t lcd_brightness;
	uint8_t wash_cycles;
} eeprom_v3_t;

//! @brief configuration store structure
//!
//! It is restored when magic read from eeprom equals magic "CW1v4"
//! Do not change. If new items needs to be stored, magic needs to be
//! changed, this struct needs to be made legacy and new structure needs
//! to be created.
typedef struct {
	uint8_t washing_speed;
	uint8_t curing_speed;
	uint8_t washing_run_time;
	uint8_t curing_run_time;
	uint8_t finish_beep_mode;
	uint8_t drying_run_time;
	uint8_t sound_response;
	uint8_t curing_machine_mode;
	uint8_t heat_to_target_temp;
	uint8_t target_temp;
	uint8_t resin_target_temp;
	uint8_t SI_unit_system;
	uint8_t resin_preheat_run_time;
	uint8_t led_intensity;
	uint8_t lcd_brightness;
	uint8_t wash_cycles;
	uint8_t filtering_speed;
	uint8_t filtering_run_time;
} eeprom_v4_t;

extern eeprom_v4_t config;

void read_config();
void write_config();
