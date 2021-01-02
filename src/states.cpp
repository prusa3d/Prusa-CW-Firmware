#include "states.h"
#include "defines.h"

namespace States {

	/*** states definitions ***/
	Base menu;
	Confirm confirm;
	Confirm error;
	Washing washing(
		pgmstr_washing,
		STATE_OPTION_CONTROLS,
		config.fans_washing_speed,
		&confirm,
		&config.washing_run_time,
		&config.washing_speed,
		&config.washing_direction,
		&config.change_direction_time);
	// FIXME - would be better to set PI regulator and manage heater for drying/curing?
	Base drying(
		pgmstr_drying,
		STATE_OPTION_CONTROLS | STATE_OPTION_HEATER | STATE_OPTION_CHAMB_TEMP,
		config.fans_drying_speed,
		&confirm,
		&config.drying_run_time,
		&config.curing_speed);
	Base curing(
		pgmstr_curing,
		STATE_OPTION_CONTROLS | STATE_OPTION_UVLED | STATE_OPTION_CHAMB_TEMP,
		config.fans_curing_speed,
		&confirm,
		&config.curing_run_time,
		&config.curing_speed);
	Base resin(
		pgmstr_heating,
		STATE_OPTION_CONTROLS | STATE_OPTION_HEATER | STATE_OPTION_CHAMB_TEMP,
		config.fans_drying_speed,
		&confirm,
		&config.resin_preheat_run_time,
		&config.curing_speed,
		&config.resin_target_temp);
	uint8_t max_warmup_run_time = MAX_WARMUP_RUNTIME;
	Warmup warmup_print(
		pgmstr_warmup,
		nullptr,
		&max_warmup_run_time,
		&config.curing_speed,
		&config.target_temp);
	Warmup warmup_resin(
		pgmstr_warmup,
		&resin,
		&max_warmup_run_time,
		&config.curing_speed,
		&config.resin_target_temp);
	uint8_t cooldown_time = COOLDOWN_RUNTIME;
	uint8_t cooldown_fans_speed[2] = {100, 100};
	Base cooldown(
		pgmstr_cooldown,
		STATE_OPTION_CONTROLS,
		cooldown_fans_speed,
		&confirm,
		&cooldown_time);

	Test_heater selftest_heater(
		pgmstr_heater_test,
		config.fans_drying_speed,
		&confirm);
	Test_uvled selftest_uvled(
		pgmstr_led_test,
		config.fans_curing_speed,
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
		hw.is_tank_inserted);
	Test_switch selftest_cover(
		pgmstr_cover_test,
		&selftest_tank,
		pgmstr_open_cover,
		pgmstr_close_cover,
		hw.is_cover_closed);


	/*** states data ***/
	Base* active_state = &menu;

	void init() {
		active_state->start();
	}

	void loop(uint8_t events) {
		active_state->process_events(events);
		Base* new_state = active_state->loop();
		if (new_state) {
			change(new_state);
		}
	}

	void change(Base* new_state) {
		active_state->do_pause();
		active_state = new_state;
		active_state->start();
	}

}
