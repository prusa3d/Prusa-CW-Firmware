#include "states.h"
#include "defines.h"
#include "config.h"

namespace States {

	Base::Base(uint8_t* fans_duties, uint8_t target_temp) :
		fans_duties(fans_duties), target_temp(target_temp)
	{}

	void Base::invoke() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
		USB_PRINTLN(fans_duties[0]);
		USB_PRINTLN(fans_duties[1]);
		USB_PRINTLN(target_temp);
		hw.set_fans(fans_duties, target_temp);
	}

	void Base::cancel() {
//		USB_PRINTLN(__PRETTY_FUNCTION__);
	}

	void Base::loop() {
	}


	Menu::Menu(uint8_t target_temp) :
		Base(config.fans_menu_speed, target_temp)
	{}


	Confirm::Confirm() :
		Menu()
	{}


	Preheat::Preheat() :
		Menu(config.resin_target_temp)
	{}


	Warmup::Warmup() :
		Menu(config.target_temp)
	{}


	Error::Error() :
		Menu()
	{}


	Drying::Drying() :
		Base(config.fans_drying_speed)
	{}


	Curing::Curing() :
		Base(config.fans_curing_speed)
	{}


	Washing::Washing() :
		Base(config.fans_washing_speed)
	{}


	/*** states definitions ***/
	Base menu = Menu();
	Base washing = Washing();
	Base drying = Drying();
	Base curing = Curing();
	Base drying_curing = Drying();	// TODO
	Base preheat = Preheat();

	// states data
	Base* active_state = &menu;

	void init() {
		active_state->invoke();
	}

	void loop() {
		active_state->loop();
	}

	void change(Base* new_state) {
		active_state->cancel();
		active_state = new_state;
		active_state->invoke();
	}
}
