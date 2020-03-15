#pragma once

#include "hardware.h"
#include "i18n.h"

namespace States {

	class Base {
	public:
		Base(uint8_t* fans_duties, uint8_t target_temp = 0);
		virtual void invoke();
		virtual void cancel();
		virtual void loop();
//		virtual void add_leave_callback();
	protected:
	private:
		uint8_t* fans_duties;
		uint8_t target_temp;
	};


	class Menu : public Base {
	public:
		Menu(uint8_t target_temp = 0);
	};


	class Confirm : public Menu {
	public:
		Confirm();
	};


	class Preheat : public Menu {
	public:
		Preheat();
	};


	class Warmup : public Menu {
	public:
		Warmup();
	};


	class Error : public Menu {
	public:
		Error();
	};


	class Drying : public Base {
	public:
		Drying();
	};


	class Curing : public Base {
	public:
		Curing();
	};


	class Washing : public Base {
	public:
		Washing();
	};


	extern Base menu;
	extern Base washing;
	extern Base drying;
	extern Base curing;
	extern Base drying_curing;
	extern Base preheat;

	void init();
	void loop();
	void change(Base* new_state);
}
