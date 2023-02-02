#include "states.h"
#include "defines.h"
#include "wrappers.h"
#include "ui.h"

namespace States {

	/*** states definitions ***/
	Base menu;
	Confirm confirm(false);
	Confirm error(true);
	Direction_change washing(
		pgmstr_washing,
		STATE_OPTION_CONTROLS | STATE_OPTION_WASHING,
		&config.wash_cycles,
		&confirm,
		&config.washing_run_time,
		MAX_WASHING_RUNTIME,
		&config.washing_speed);
	Base cleaning(
		pgmstr_cleaning,
		STATE_OPTION_CONTROLS | STATE_OPTION_WASHING,
		&confirm,
		&config.cleaning_run_time,
		MAX_CLEAN_RUNTIME,
		&config.cleaning_speed);
	Base drying(
		pgmstr_drying,
		STATE_OPTION_CONTROLS | STATE_OPTION_HEATER | STATE_OPTION_CHAMB_TEMP,
		&confirm,
		&config.drying_run_time,
		MAX_DRYING_RUNTIME,
		&config.curing_speed);
	Base curing(
		pgmstr_curing,
		STATE_OPTION_CONTROLS | STATE_OPTION_UVLED,
		&confirm,
		&config.curing_run_time,
		MAX_CURING_RUNTIME,
		&config.curing_speed);
	Base resin(
		pgmstr_heating,
		STATE_OPTION_CONTROLS | STATE_OPTION_HEATER | STATE_OPTION_CHAMB_TEMP,
		&confirm,
		&config.resin_preheat_run_time,
		MAX_PREHEAT_RUNTIME,
		&config.curing_speed,
		&config.resin_target_temp);
	Warmup warmup_print(
		pgmstr_warmup,
		nullptr,
		&config.target_temp);
	Warmup warmup_resin(
		pgmstr_warmup,
		&resin,
		&config.resin_target_temp);
	Cooldown cooldown(&confirm);
	Reset reset;

	Test_heater selftest_heater(
		pgmstr_heater_test,
		&confirm);
	Test_uvled selftest_uvled(
		pgmstr_led_test,
		&selftest_heater);
	Test_fans selftest_fans(
		pgmstr_fans_test,
		&selftest_uvled);
	Test_rotation selftest_rotation(
		pgmstr_rotation_test,
		&selftest_fans);
	Test_switch selftest_tank(
		pgmstr_ipatank_test,
		&selftest_rotation,
		pgmstr_remove_tank,
		pgmstr_insert_tank,
		is_tank_inserted);
	Test_switch selftest_cover(
		pgmstr_cover_test,
		&selftest_tank,
		pgmstr_open_cover,
		pgmstr_close_cover,
		is_cover_closed);


	/*** states data ***/
	Base* active_state = &menu;

	void init() {
		active_state->start();
	}

	void loop(uint8_t events) {
		active_state->process_events(events);
		Base* new_state = active_state->loop();
		if (new_state) {
			if (active_state == &menu && new_state == &error) {
				UI::set_menu(&UI::error);
			} else {
				change(new_state);
			}
		}
	}

	void change(Base* new_state) {
		bool handle_heater = new_state->options & STATE_OPTION_HEATER;
		active_state->do_pause(!handle_heater);
		handle_heater = active_state->options & STATE_OPTION_HEATER;
		active_state = new_state;
		active_state->start(!handle_heater);
	}

}
