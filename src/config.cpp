#include "EEPROM.h"
#include "config.h"
#include "hardware.h"

#define EEPROM_OFFSET	128
#define MAGIC_SIZE		6
#define EEPROM_BASE		E2END + 1 - EEPROM_OFFSET
static_assert(sizeof(eeprom_v2_t) <= EEPROM_OFFSET, "eeprom_t doesn't fit in it's reserved space in the memory.");

char config_magic[MAGIC_SIZE] = "CW1v2";

//! @brief configuration
//!
//! Default values definition,
//! it can be overridden by user and stored to
//! and restored from permanent storage.

eeprom_v2_t config = {
	10,			// washing_speed
	1,			// curing_speed
	4,			// washing_run_time
	3,			// curing_run_time
	1,			// finish_beep_mode
	3,			// drying_run_time
	1,			// sound_response
	0,			// curing_machine_mode (0=drying/curing, 1=curing, 2=drying, 3=resin preheat)
	0,			// heat_to_target_temp
	35,			// target_temp (celsius)
	30,			// resin_target_temp (celsius)
	1,			// SI_unit_system

	10,			// resin_preheat_run_time
	100,		// led_pwm_value
	{60, 70},	// fans_curing_speed
	{60, 70},	// fans_drying_speed
	{40, 40},	// fans_preheat_speed
};

void write_config() {
	EEPROM.put(EEPROM_BASE, reinterpret_cast<uint8_t*>(config_magic), MAGIC_SIZE);
	EEPROM.put(EEPROM_BASE + MAGIC_SIZE, reinterpret_cast<uint8_t*>(&config), sizeof(config));
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
	EEPROM.get(EEPROM_BASE, reinterpret_cast<uint8_t*>(test_magic), MAGIC_SIZE);
	if (!strncmp(config_magic, test_magic, MAGIC_SIZE)) {
		// latest magic
		EEPROM.get(EEPROM_BASE + MAGIC_SIZE, reinterpret_cast<uint8_t*>(&config), sizeof(config));
	} else if (!strncmp("CURWA", test_magic, MAGIC_SIZE)) {
		// legacy magic
		uint8_t tmp = config.resin_target_temp;	// remember default
		EEPROM.get(EEPROM_BASE + MAGIC_SIZE, reinterpret_cast<uint8_t*>(&config), sizeof(eeprom_v1_t));
		if (config.SI_unit_system) {
			config.resin_target_temp = tmp;
		} else {
			config.target_temp = round(celsius2fahrenheit(config.target_temp));
			config.resin_target_temp = round(celsius2fahrenheit(tmp));
		}
	}
}
