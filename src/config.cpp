#include "EEPROM.h"
#include "config.h"
#include "hardware.h"

#define MAGIC_SIZE		6
static_assert(sizeof(eeprom_v2_t) <= EEPROM_OFFSET, "eeprom_t doesn't fit in it's reserved space in the memory.");

const char legacy_magic1[MAGIC_SIZE] PROGMEM = "CURWA";
const char legacy_magic2[MAGIC_SIZE] PROGMEM = "CW1v2";
const char legacy_magic3[MAGIC_SIZE] PROGMEM = "CW1v3";
const char config_magic[MAGIC_SIZE]  PROGMEM = "CW1v4";

//! @brief configuration
//!
//! Default values definition,
//! it can be overridden by user and stored to
//! and restored from permanent storage.

eeprom_v4_t config = {
	10,			// washing_speed
	1,			// curing_speed
	4,			// washing_run_time
	3,			// curing_run_time
	1,			// finish_beep_mode (0=none, 1=once, 2=continuous)
	3,			// drying_run_time
	1,			// sound_response
	0,			// curing_machine_mode (0=drying/curing, 1=curing, 2=drying)
	0,			// heat_to_target_temp
	35,			// target_temp (celsius)
	30,			// resin_target_temp (celsius)
	1,			// SI_unit_system

	10,			// resin_preheat_run_time
	100,		// led_intensity
	100,		// lcd_brightness
	1,			// wash_cycles

	10,			// filtering_speed
	45,			// filtering_run_time
};

void write_config() {
	char magic[MAGIC_SIZE];
	strncpy_P(magic, config_magic, MAGIC_SIZE);
	EEPROM.put(CONFIG_START, reinterpret_cast<uint8_t*>(magic), MAGIC_SIZE);
	EEPROM.put(CONFIG_START + MAGIC_SIZE, reinterpret_cast<uint8_t*>(&config), sizeof(config));
}

/*! \brief This function loads user-defined values from eeprom.
 *
 *	It loads different amount of variables, depending on the magic variable from eeprom.
 *	If magic is not set in the eeprom, variables keep their default values.
 *	If magic from eeprom is equal to lagacy magic, it loads only variables customizable in older firmware and keeps new variables default.
 *	If magic from eeprom is equal to config_magic, it loads all variables including those added in new firmware.
 *	It won't load undefined (new) variables after flashing new firmware.
 */
void read_config() {
	char test_magic[MAGIC_SIZE];
	EEPROM.get(CONFIG_START, reinterpret_cast<uint8_t*>(test_magic), MAGIC_SIZE);
	if (!strncmp_P(test_magic, config_magic, MAGIC_SIZE)) {
		// latest magic
		EEPROM.get(CONFIG_START + MAGIC_SIZE, reinterpret_cast<uint8_t*>(&config), sizeof(config));
	} else if (!strncmp_P(test_magic, legacy_magic3, MAGIC_SIZE)) {
		// legacy magic
		EEPROM.get(CONFIG_START + MAGIC_SIZE, reinterpret_cast<uint8_t*>(&config), sizeof(eeprom_v3_t));
	} else if (!strncmp_P(test_magic, legacy_magic2, MAGIC_SIZE)) {
		// legacy magic
		EEPROM.get(CONFIG_START + MAGIC_SIZE, reinterpret_cast<uint8_t*>(&config), sizeof(eeprom_v2_t) - 9);
		EEPROM.get(CONFIG_START + MAGIC_SIZE + sizeof(eeprom_v2_t) - 1, reinterpret_cast<uint8_t*>(&config.lcd_brightness), 1);
	} else if (!strncmp_P(test_magic, legacy_magic1, MAGIC_SIZE)) {
		// legacy magic
		uint8_t tmp = config.resin_target_temp;	// remember default
		EEPROM.get(CONFIG_START + MAGIC_SIZE, reinterpret_cast<uint8_t*>(&config), sizeof(eeprom_v1_t));
		if (config.SI_unit_system) {
			config.resin_target_temp = tmp;
		} else {
			config.target_temp = round_short(celsius2fahrenheit(config.target_temp));
			config.resin_target_temp = round_short(celsius2fahrenheit(tmp));
		}
	}
}
