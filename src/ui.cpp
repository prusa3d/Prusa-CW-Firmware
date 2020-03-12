#include "LiquidCrystal_Prusa.h"
#include "ui.h"
#include "defines.h"
#include "config.h"

namespace UI {

	// UI:SN
	SN::SN(const char* label) :
		Base(label, 0, true)
	{}

	char* SN::get_menu_label(char* buffer, uint8_t buffer_size) {
		USB_TRACE("SN::get_menu_label()\r\n");
		strncpy_P(buffer, pgmstr_sn, buffer_size);
		// sizeof() != strlen()
		uint8_t bs = buffer_size - (sizeof(pgmstr_sn) - 1);
		return Base::get_menu_label(buffer + sizeof(pgmstr_sn) - 1, bs < SN_LENGTH+1 ? bs : SN_LENGTH+1);
	}


	// UI::SI_switch
	SI_switch::SI_switch(const char* label, uint8_t& value, Temperature* const* to_change, uint8_t to_change_count) :
		Bool(label, value, pgmstr_celsius_units, pgmstr_fahrenheit_units), to_change(to_change), to_change_count(to_change_count)
	{}

	bool SI_switch::in_menu_action() {
		USB_TRACE("SI_switch::in_menu_action()\r\n");
		for (uint8_t i = 0; i < to_change_count; ++i) {
			to_change[i]->units_change(value^1);
		}
		Bool::in_menu_action();
		return true;
	}


	// UI::Do_it
	Do_it::Do_it(const char* label, uint8_t& curing_machine_mode) :
		State(label), curing_machine_mode(curing_machine_mode)
	{}

	char* Do_it::get_menu_label(char* buffer, uint8_t buffer_size) {
		USB_TRACE("Do_it::get_menu_label()\r\n");
		if (hw.is_tank_inserted()) {
			label = pgmstr_washing;
		} else {
			switch (curing_machine_mode) {
				case 2:
					label = pgmstr_drying;
					break;
				case 1:
					label = pgmstr_curing;
					break;
				default:
					label = pgmstr_drying_curing;
					break;
			}
		}
		return State::get_menu_label(buffer, buffer_size);
	}


	/*** menu definitions ***/
	Base back(pgmstr_back, BACK_CHAR);

	// run time menu
	Minutes curing_run_time(pgmstr_curing_run_time, config.curing_run_time);
	Minutes drying_run_time(pgmstr_drying_run_time, config.drying_run_time);
	Minutes washing_run_time(pgmstr_washing_run_time, config.washing_run_time);
	Minutes resin_preheat_run_time(pgmstr_resin_preheat_time, config.resin_preheat_run_time, 30);
	Base* const run_time_items[] = {&back, &curing_run_time, &drying_run_time, &washing_run_time, &resin_preheat_run_time};
	Menu run_time_menu(pgmstr_run_time, run_time_items, COUNT_ITEMS(run_time_items));

	// speed menu
	X_of_ten curing_speed(pgmstr_curing_speed, config.curing_speed);
	X_of_ten washing_speed(pgmstr_washing_speed, config.washing_speed);
	Base* const speed_items[] = {&back, &curing_speed, &washing_speed};
	Menu speed_menu(pgmstr_rotation_speed, speed_items, COUNT_ITEMS(speed_items));

	// temperatore menu
	Bool heat_to_target_temp(pgmstr_warmup, config.heat_to_target_temp);
	Temperature target_temp(pgmstr_drying_warmup_temp, config.target_temp, config.SI_unit_system);
	Temperature resin_target_temp(pgmstr_resin_preheat_temp, config.resin_target_temp, config.SI_unit_system);
	Temperature* const SI_changed[] = {&target_temp, &resin_target_temp};
	SI_switch SI_unit_system(pgmstr_units, config.SI_unit_system, SI_changed, COUNT_ITEMS(SI_changed));
	Base* const temperature_items[] = {&back, &heat_to_target_temp, &target_temp, &resin_target_temp, &SI_unit_system};
	Menu temperature_menu(pgmstr_temperatures, temperature_items, COUNT_ITEMS(temperature_items));

	// sound menu
	Bool sound_response(pgmstr_control_echo, config.sound_response);
	const char* finish_beep_options[] = {pgmstr_none, pgmstr_once, pgmstr_continuous};
	Option finish_beep(pgmstr_finish_beep, config.finish_beep_mode, finish_beep_options, COUNT_ITEMS(finish_beep_options));
	Base* const sound_items[] = {&back, &sound_response, &finish_beep};
	Menu sound_menu(pgmstr_sound, sound_items, COUNT_ITEMS(sound_items));

	// fans menu
	Percent fan1_curing_speed(pgmstr_fan1_curing_speed, config.fans_curing_speed[0]);
	Percent fan1_drying_speed(pgmstr_fan1_drying_speed, config.fans_drying_speed[0]);
	Percent fan2_curing_speed(pgmstr_fan2_curing_speed, config.fans_curing_speed[1]);
	Percent fan2_drying_speed(pgmstr_fan2_drying_speed, config.fans_drying_speed[1]);
	Base* const fans_items[] = {&back, &fan1_curing_speed, &fan1_drying_speed, &fan2_curing_speed, &fan2_drying_speed};
	Menu fans_menu(pgmstr_fans, fans_items, COUNT_ITEMS(fans_items));

	// info menu
	Base fw_version(pgmstr_fw_version, 0, true);
	SN serial_number(pgmstr_serial_number);
	Base build_nr(pgmstr_build_nr, 0, true);
	Base fw_hash(pgmstr_fw_hash, 0, true);
#if FW_LOCAL_CHANGES
	Base workspace_dirty(pgmstr_workspace_dirty, 0, true);
	Base* const info_items[] = {&back, &fw_version, &serial_number, &build_nr, &fw_hash, &workspace_dirty};
#else
	Base* const info_items[] = {&back, &fw_version, &serial_number, &build_nr, &fw_hash};
#endif
	Menu info_menu(pgmstr_information, info_items, COUNT_ITEMS(info_items));

	// config menu
	const char* curing_machine_mode_options[] = {pgmstr_drying_curing, pgmstr_curing, pgmstr_drying};
	Option curing_machine_mode(pgmstr_run_mode, config.curing_machine_mode, curing_machine_mode_options, COUNT_ITEMS(curing_machine_mode_options));
	Percent led_pwm_value(pgmstr_led_intensity, config.led_pwm_value, 1);
	Base* const config_items[] = {&back, &speed_menu, &curing_machine_mode, &temperature_menu, &sound_menu, &fans_menu, &led_pwm_value, &info_menu};
	Menu config_menu(pgmstr_settings, config_items, COUNT_ITEMS(config_items));

	// home menu
	Do_it do_it(pgmstr_emptystr, config.curing_machine_mode);
	State resin_preheat(pgmstr_resin_preheat);
	Base* const home_items[] = {&do_it, &resin_preheat, &run_time_menu, &config_menu};
	Menu home_menu(pgmstr_emptystr, home_items, COUNT_ITEMS(home_items));

	// menu data
	Base* menu_stack[MAX_MENU_DEPTH];
	uint8_t menu_depth = 0;
	Base* active_menu = &home_menu;

	void init() {
		home_menu.set_long_press_ui_item(&curing_machine_mode);
		active_menu->show();
	}

	void loop() {
		Base* new_menu = active_menu->process_events(hw.get_events((bool)config.sound_response));
		if (new_menu == &back || new_menu == active_menu) {
			if (menu_depth) {
				active_menu = menu_stack[--menu_depth];
				lcd.clear();
				active_menu->show();
			} else {
				USB_TRACE("ERROR: back at menu depth 0!\r\n");
			}
		} else if (new_menu) {
			if (menu_depth < MAX_MENU_DEPTH) {
				menu_stack[menu_depth++] = active_menu;
				active_menu = new_menu;
				lcd.clear();
				active_menu->show();
			} else {
				USB_TRACE("ERROR: MAX_MENU_DEPTH reached!\r\n");
			}
		}
	}

}
