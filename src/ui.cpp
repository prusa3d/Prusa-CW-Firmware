#include "LiquidCrystal_Prusa.h"
#include "defines.h"
#include "config.h"
#include "ui.h"
#include "states.h"

namespace UI {

	/*** menu definitions ***/
	Base back(pgmstr_back, BACK_CHAR);
	Base stop(pgmstr_stop, STOP_CHAR);
	State error(pgmstr_emptystr, &States::error, nullptr);

	// move all following definitions to init() to save RAM instead of PROGMEM

	// run time menu
	Minutes curing_run_time(pgmstr_curing_run_time, config.curing_run_time, MAX_CURING_RUNTIME);
	Minutes drying_run_time(pgmstr_drying_run_time, config.drying_run_time, MAX_DRYING_RUNTIME);
	Minutes washing_run_time(pgmstr_washing_run_time, config.washing_run_time, MAX_WASHING_RUNTIME);
	Minutes resin_preheat_run_time(pgmstr_resin_preheat_time, config.resin_preheat_run_time, MAX_PREHEAT_RUNTIME);
	Base* const run_time_items[] PROGMEM = {&back, &curing_run_time, &drying_run_time, &washing_run_time, &resin_preheat_run_time};
	Menu run_time_menu(pgmstr_run_time, run_time_items, COUNT_ITEMS(run_time_items));

	// rotation menu
	X_of_ten curing_speed(pgmstr_curing_speed, config.curing_speed);
	X_of_ten washing_speed(pgmstr_washing_speed, config.washing_speed);
	X_of_ten wash_dir_changes(pgmstr_wash_dir_changes, config.wash_cycles);
	Base* const speed_items[] PROGMEM = {&back, &curing_speed, &washing_speed, &wash_dir_changes};
	Menu speed_menu(pgmstr_rotation_settings, speed_items, COUNT_ITEMS(speed_items));

	// temperatore menu
	Bool heat_to_target_temp(pgmstr_warmup, config.heat_to_target_temp);
	Temperature target_temp(pgmstr_drying_warmup_temp, config.target_temp);
	Temperature resin_target_temp(pgmstr_resin_preheat_temp, config.resin_target_temp);
	Temperature* const SI_changed[] PROGMEM = {&target_temp, &resin_target_temp};
	SI_switch SI_unit_system(pgmstr_units, config.SI_unit_system, SI_changed, COUNT_ITEMS(SI_changed));
	Base* const temperature_items[] PROGMEM = {&back, &heat_to_target_temp, &target_temp, &resin_target_temp, &SI_unit_system};
	Menu temperature_menu(pgmstr_temperatures, temperature_items, COUNT_ITEMS(temperature_items));

	// sound menu
	Bool sound_response(pgmstr_control_echo, config.sound_response);
	const char* const finish_beep_options[] PROGMEM = {pgmstr_none, pgmstr_once, pgmstr_continuous};
	Option finish_beep(pgmstr_finish_beep, config.finish_beep_mode, finish_beep_options, COUNT_ITEMS(finish_beep_options));
	Base* const sound_items[] PROGMEM = {&back, &sound_response, &finish_beep};
	Menu sound_menu(pgmstr_sound, sound_items, COUNT_ITEMS(sound_items));

	// system info
	SN serial_number(pgmstr_sn);
	Text fw_version(pgmstr_fw_version);
	Text build_nr(pgmstr_build_nr);
	Text fw_hash(pgmstr_fw_hash);
#if FW_LOCAL_CHANGES
	Text workspace_dirty(pgmstr_workspace_dirty);
	Base* const info_items[] PROGMEM = {&back, &serial_number, &fw_version, &build_nr, &fw_hash, &workspace_dirty};
#else
	Base* const info_items[] PROGMEM = {&back, &serial_number, &fw_version, &build_nr, &fw_hash};
#endif
	Info system_info(pgmstr_information, info_items, COUNT_ITEMS(info_items));

	// config menu
	const char* const curing_machine_mode_options[] PROGMEM = {pgmstr_drying_curing, pgmstr_curing, pgmstr_drying};
	Option curing_machine_mode(pgmstr_run_mode, config.curing_machine_mode, curing_machine_mode_options, COUNT_ITEMS(curing_machine_mode_options));
	Percent_with_action lcd_brightness(pgmstr_lcd_brightness, config.lcd_brightness, MIN_LCD_BRIGHTNESS, lcd.setBrightness);
	Base* const config_items[] PROGMEM = {&back, &speed_menu, &curing_machine_mode, &temperature_menu, &sound_menu, &lcd_brightness, &system_info};
	Menu config_menu(pgmstr_settings, config_items, COUNT_ITEMS(config_items));

	// run menu
	Pause pause(&back);
	Base* const run_items[] PROGMEM = {&pause, &stop, &back};
	Menu run_menu(pgmstr_emptystr, run_items, COUNT_ITEMS(run_items));

	// hold platform function
	Base* const hold_platform_items[] PROGMEM = {&back};
	Hold_platform hold_platform_menu(pgmstr_hold_platform, hold_platform_items, COUNT_ITEMS(hold_platform_items));

	// home menu
	Do_it do_it(config.curing_machine_mode, &run_menu);
	State resin_preheat(pgmstr_resin_preheat, &States::warmup_resin, &run_menu);
	Base* const home_items[] PROGMEM = {&do_it, &resin_preheat, &run_time_menu, &hold_platform_menu, &config_menu};
	Menu home_menu(pgmstr_emptystr, home_items, COUNT_ITEMS(home_items));

	// factory reset confirm
	State factory_reset(pgmstr_reset_confirm, &States::reset);
	Base* const reset_items[] PROGMEM = {&back, &factory_reset};
	Menu reset(pgmstr_factory_reset, reset_items, COUNT_ITEMS(reset_items));

	// advanced menu
	Percent led_intensity(pgmstr_led_intensity, config.led_intensity, MIN_LED_INTENSITY);
	State cooldown(pgmstr_cooldown, &States::cooldown, nullptr);
	State selftest(pgmstr_selftest, &States::selftest_cover, nullptr);
	Base* const advanced_items[] PROGMEM = {&back, &led_intensity, &reset, &cooldown, &selftest};
	Menu advanced_menu(pgmstr_emptystr, advanced_items, COUNT_ITEMS(advanced_items));


	/*** menu data ***/
	Base* menu_stack[MAX_MENU_DEPTH];
	uint8_t menu_depth = 0;
	Base* active_menu = &home_menu;

	void init() {
		for (uint8_t i = 0; i < COUNT_ITEMS(SI_changed); ++i) {
			SI_changed[i]->init(config.SI_unit_system);
		}
		home_menu.set_long_press_ui_item(&curing_machine_mode);
		config_menu.set_long_press_ui_item(&advanced_menu);
		active_menu->invoke();
		active_menu->show();
	}

	void set_menu(Base* new_menu) {
		if (menu_depth < MAX_MENU_DEPTH) {
			menu_stack[menu_depth++] = active_menu;
			active_menu = new_menu;
			lcd.clear();
			active_menu->invoke();
			active_menu->show();
		} else {
			USB_PRINTLNP("ERROR: MAX_MENU_DEPTH reached!");
		}
	}

	void loop(uint8_t events) {
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
				USB_PRINTLNP("ERROR: back at menu depth 0!");
			}
		} else if (new_menu) {
			set_menu(new_menu);
		}
	}

}
