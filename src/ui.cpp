#include "LiquidCrystal_Prusa.h"
#include "defines.h"
#include "config.h"
#include "ui.h"
#include "states.h"

namespace UI {

	/*** menu definitions ***/
	Base back(pgmstr_back, BACK_CHAR);
	Base stop(pgmstr_stop, STOP_CHAR);

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
	Temperature target_temp(pgmstr_drying_warmup_temp, config.target_temp);
	Temperature resin_target_temp(pgmstr_resin_preheat_temp, config.resin_target_temp);
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

	// fans curing speed
	Percent fan1_curing_speed(pgmstr_fan1_curing_speed, config.fans_curing_speed[0], MIN_FAN_SPEED);
	Percent fan2_curing_speed(pgmstr_fan2_curing_speed, config.fans_curing_speed[1], MIN_FAN_SPEED);
	Base* const fans_curing_speed[] = {&back, &fan1_curing_speed, &fan2_curing_speed};
	Menu fans_curing_menu(pgmstr_fans_curing, fans_curing_speed, COUNT_ITEMS(fans_curing_speed));

	// fans drying speed
	Percent fan1_drying_speed(pgmstr_fan1_drying_speed, config.fans_drying_speed[0], MIN_FAN_SPEED);
	Percent fan2_drying_speed(pgmstr_fan2_drying_speed, config.fans_drying_speed[1], MIN_FAN_SPEED);
	Base* const fans_drying_speed[] = {&back, &fan1_drying_speed, &fan2_drying_speed};
	Menu fans_drying_menu(pgmstr_fans_drying, fans_drying_speed, COUNT_ITEMS(fans_drying_speed));

	// fans washing speed
	Percent fan1_washing_speed(pgmstr_fan1_washing_speed, config.fans_washing_speed[0], MIN_FAN_SPEED);
	Percent fan2_washing_speed(pgmstr_fan2_washing_speed, config.fans_washing_speed[1], MIN_FAN_SPEED);
	Base* const fans_washing_speed[] = {&back, &fan1_washing_speed, &fan2_washing_speed};
	Menu fans_washing_menu(pgmstr_fans_washing, fans_washing_speed, COUNT_ITEMS(fans_washing_speed));

	// fans menu speed
	Percent fan1_menu_speed(pgmstr_fan1_menu_speed, config.fans_menu_speed[0], MIN_FAN_SPEED);
	Percent fan2_menu_speed(pgmstr_fan2_menu_speed, config.fans_menu_speed[1], MIN_FAN_SPEED);
	Base* const fans_menu_speed[] = {&back, &fan1_menu_speed, &fan2_menu_speed};
	Menu fans_menu_menu(pgmstr_fans_menu, fans_menu_speed, COUNT_ITEMS(fans_menu_speed));

	// fans menu
	Base* const fans_items[] = {&back, &fans_curing_menu, &fans_drying_menu, &fans_washing_menu, &fans_menu_menu};
	Menu fans_menu(pgmstr_fans, fans_items, COUNT_ITEMS(fans_items));

	// info menu
	SN serial_number(pgmstr_serial_number);
	Text fw_version(pgmstr_fw_version);
	Text build_nr(pgmstr_build_nr);
	Text fw_hash(pgmstr_fw_hash);
#if FW_LOCAL_CHANGES
	Text workspace_dirty(pgmstr_workspace_dirty);
	Base* const info_items[] = {&back, &serial_number, &fw_version, &build_nr, &fw_hash, &workspace_dirty};
#else
	Base* const info_items[] = {&back, &serial_number, &fw_version, &build_nr, &fw_hash};
#endif
	Menu info_menu(pgmstr_information, info_items, COUNT_ITEMS(info_items));

	// config menu
	const char* curing_machine_mode_options[] = {pgmstr_drying_curing, pgmstr_curing, pgmstr_drying};
	Option curing_machine_mode(pgmstr_run_mode, config.curing_machine_mode, curing_machine_mode_options, COUNT_ITEMS(curing_machine_mode_options));
	Percent led_pwm_value(pgmstr_led_intensity, config.led_pwm_value, 1);
	Base* const config_items[] = {&back, &speed_menu, &curing_machine_mode, &temperature_menu, &sound_menu, &fans_menu, &led_pwm_value, &info_menu};
	Menu config_menu(pgmstr_settings, config_items, COUNT_ITEMS(config_items));

	// run menu
	Pause pause(&back);
	Base* const run_items[] = {&pause, &stop, &back};
	Menu run_menu(pgmstr_emptystr, run_items, COUNT_ITEMS(run_items));

	// home menu
	Do_it do_it(pgmstr_emptystr, config.curing_machine_mode, &run_menu);
	State resin_preheat(pgmstr_resin_preheat, &States::warmup_resin, &run_menu);
	Base* const home_items[] = {&do_it, &resin_preheat, &run_time_menu, &config_menu};
	Menu home_menu(pgmstr_emptystr, home_items, COUNT_ITEMS(home_items));

	// selftest menu
	State selftest(pgmstr_selftest, &States::selftest_cover, nullptr);
	Base* const selftest_items[] = {&back, &selftest};
	Menu selftest_menu(pgmstr_emptystr, selftest_items, COUNT_ITEMS(selftest_items));


	/*** menu data ***/
	Base* menu_stack[MAX_MENU_DEPTH];
	uint8_t menu_depth = 0;
	Base* active_menu = &home_menu;

	void init() {
		for (uint8_t i = 0; i < COUNT_ITEMS(SI_changed); ++i) {
			SI_changed[i]->init(config.SI_unit_system);
		}
		home_menu.set_long_press_ui_item(&curing_machine_mode);
		info_menu.set_long_press_ui_item(&selftest_menu);
		active_menu->invoke();
		active_menu->show();
	}

	void loop(Events& events) {
		active_menu->loop();
		Base* new_menu = active_menu->process_events(events);
		if (new_menu == &stop || new_menu == &back || new_menu == active_menu || States::active_state->is_finished()) {
			if (menu_depth) {
				do {
					active_menu->leave();
					active_menu = menu_stack[--menu_depth];
				} while (new_menu == &stop && menu_depth);
				lcd.clear();
				active_menu->show();
			} else {
				USB_PRINTLN("ERROR: back at menu depth 0!");
			}
		} else if (new_menu) {
			if (menu_depth < MAX_MENU_DEPTH) {
				menu_stack[menu_depth++] = active_menu;
				active_menu = new_menu;
				lcd.clear();
				active_menu->invoke();
				active_menu->show();
			} else {
				USB_PRINTLN("ERROR: MAX_MENU_DEPTH reached!");
			}
		}
	}

}
