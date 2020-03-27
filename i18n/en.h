// all menu and options
static const char pgmstr_back[] PROGMEM = _("Back");
static const char pgmstr_on[] PROGMEM = _("[on]");
static const char pgmstr_off[] PROGMEM = _("[off]");
static const char pgmstr_percent[] PROGMEM = " %";
static const char pgmstr_xoften[] PROGMEM = "/10";
static const char pgmstr_celsius[] PROGMEM = " \xDF" "C";
static const char pgmstr_fahrenheit[] PROGMEM = " \xDF" "F";
static const char pgmstr_minutes[] PROGMEM = _(" min.");
static const char pgmstr_gt[] PROGMEM = " >";
static const char pgmstr_lt[] PROGMEM = "< ";
static const char pgmstr_emptystr[] PROGMEM = "";

// run-time menu
static const char pgmstr_run_time[] PROGMEM = _("Run-time");
static const char pgmstr_curing_run_time[] PROGMEM = _("Curing run-time");
static const char pgmstr_drying_run_time[] PROGMEM = _("Drying run-time");
static const char pgmstr_washing_run_time[] PROGMEM = _("Washing run-time");
static const char pgmstr_resin_preheat_time[] PROGMEM = _("Resin preheat time");

// speed menu
static const char pgmstr_rotation_speed[] PROGMEM = _("Rotation speed");
static const char pgmstr_curing_speed[] PROGMEM = _("Curing speed");
static const char pgmstr_washing_speed[] PROGMEM = _("Washing speed");

// temperatore menu
static const char pgmstr_temperatures[] PROGMEM = _("Temperatures");
static const char pgmstr_warmup[] PROGMEM = _("Warm-up");
static const char pgmstr_drying_warmup_temp[] PROGMEM = _("Drying warm-up t.");
static const char pgmstr_resin_preheat_temp[] PROGMEM = _("Resin preheat t.");
static const char pgmstr_units[] PROGMEM = _("Temp. units");
static const char pgmstr_celsius_units[] PROGMEM = "[\xDF" "C]";
static const char pgmstr_fahrenheit_units[] PROGMEM = "[\xDF" "F]";

// sound menu
static const char pgmstr_sound[] PROGMEM = _("Sound");
static const char pgmstr_control_echo[] PROGMEM = _("Control echo");
static const char pgmstr_none[] PROGMEM = _("none");
static const char pgmstr_once[] PROGMEM = _("once");
static const char pgmstr_continuous[] PROGMEM = _("continuous");
static const char pgmstr_finish_beep[] PROGMEM = _("Finish beep");

// fans curing speed
static const char pgmstr_fans_curing[] PROGMEM = _("Fans curing speed");
static const char pgmstr_fan1_curing_speed[] PROGMEM = _("Fan1 curing speed");
static const char pgmstr_fan2_curing_speed[] PROGMEM = _("Fan2 curing speed");

// fans drying speed
static const char pgmstr_fans_drying[] PROGMEM = _("Fans drying speed");
static const char pgmstr_fan1_drying_speed[] PROGMEM = _("Fan1 drying speed");
static const char pgmstr_fan2_drying_speed[] PROGMEM = _("Fan2 drying speed");

// fans washing speed
static const char pgmstr_fans_washing[] PROGMEM = _("Fans washing speed");
static const char pgmstr_fan1_washing_speed[] PROGMEM = _("Fan1 washing speed");
static const char pgmstr_fan2_washing_speed[] PROGMEM = _("Fan2 washing speed");

// fans menu speed
static const char pgmstr_fans_menu[] PROGMEM = _("Fans menu speed");
static const char pgmstr_fan1_menu_speed[] PROGMEM = _("Fan1 menu speed");
static const char pgmstr_fan2_menu_speed[] PROGMEM = _("Fan2 menu speed");

// fans menu
static const char pgmstr_fans[] PROGMEM = _("Fans");

// info menu
static const char pgmstr_information[] PROGMEM = _("Information");
static const char pgmstr_fw_version[] PROGMEM = _("FW version: ")  FW_VERSION;
static const char pgmstr_sn[] PROGMEM = "SN: ";
static const char pgmstr_build_nr[] PROGMEM = _("Build: ") FW_BUILDNR;
static const char pgmstr_fw_hash[] PROGMEM = FW_HASH;
#if FW_LOCAL_CHANGES
static const char pgmstr_workspace_dirty[] PROGMEM = _("Workspace dirty");
#endif

// config menu
static const char pgmstr_settings[] PROGMEM = _("Settings");
static const char pgmstr_run_mode[] PROGMEM = _("Run mode");
static const char pgmstr_drying_curing[] PROGMEM = _("Drying/curing");
static const char pgmstr_curing[] PROGMEM = _("Curing");
static const char pgmstr_drying[] PROGMEM = _("Drying");
static const char pgmstr_led_intensity[] PROGMEM = _("LED intensity");

// run menu
static const char pgmstr_pause[] PROGMEM = _("Pause");
static const char pgmstr_continue[] PROGMEM = _("Continue");
static const char pgmstr_stop[] PROGMEM = _("Stop");

// home menu
static const char pgmstr_washing[] PROGMEM = _("Washing");
static const char pgmstr_resin_preheat[] PROGMEM = _("Resin preheat");

// self test menu
static const char pgmstr_selftest[] PROGMEM = _("Selftest");

// state menu
static const char pgmstr_progress[] PROGMEM = { '|', '/', '-', BACKSLASH_CHAR };

// all states
static const char pgmstr_double_gt[] PROGMEM = ">>";
static const char pgmstr_double_lt[] PROGMEM = "<<";
static const char pgmstr_max_symb[] PROGMEM = ">|";
static const char pgmstr_min_symb[] PROGMEM = "|<";
static const char pgmstr_double_space[] PROGMEM = "  ";
static const char pgmstr_paused[] PROGMEM = _("Paused");
static const char pgmstr_close_cover[] PROGMEM = _("Close the cover");
static const char pgmstr_remove_tank[] PROGMEM = _("Remove IPA tank");
static const char pgmstr_heater_error[] PROGMEM = _("Heater fan error");
static const char pgmstr_please_restart[] PROGMEM = _("Please restart");

// confirm state
static const char pgmstr_finished[] PROGMEM = _("Finished");
static const char pgmstr_press2continue[] PROGMEM = _("Press to continue");

// resin state
static const char pgmstr_heating[] PROGMEM = _("Heating");

// washing state
static const char pgmstr_insert_tank[] PROGMEM = _("Insert IPA tank");

// selftest states
static const char pgmstr_heater_test[] PROGMEM = _("Heater test");
static const char pgmstr_led_test[] PROGMEM = _("LED test");
static const char pgmstr_rotation_test[] PROGMEM = _("Rotation test");
static const char pgmstr_fans_test[] PROGMEM = _("Fans test");
static const char pgmstr_ipatank_test[] PROGMEM = _("IPA tank test");
static const char pgmstr_cover_test[] PROGMEM = _("Cover test");


// TODO
static const char pgmstr_fan1_failure[] PROGMEM = _("Fan1 failure");
static const char pgmstr_fan2_failure[] PROGMEM = _("Fan2 failure");
static const char pgmstr_heater_failure[] PROGMEM = _("HEATER failure");
static const char pgmstr_open_cover[] PROGMEM = _("Open the cover");
static const char pgmstr_test_failed[] PROGMEM = _("Test failed");
static const char pgmstr_test_success[] PROGMEM = _("Test successful");
static const char pgmstr_fan1_test[] PROGMEM = _("Fan1:   x/100ms");
static const char pgmstr_fan2_test[] PROGMEM = _("Fan2:   x/100ms");
static const char pgmstr_mode_gear[] PROGMEM = _("Mode/Gear:");
// untranslated constants
static const char pgmstr_slash[] PROGMEM = "/";


