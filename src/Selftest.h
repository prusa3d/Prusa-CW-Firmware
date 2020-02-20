#pragma once

#ifndef SELFTEST_H
#define SELFTEST_H

#include "Countimer.h"
#include "hardware.h"

/*! \class Selftest
    \brief A class runs series of tests that should check, with help of an operator, that all vital elements are fully functioning.

    It will be used as the initial test before CW1 is passed to a customer.
    There are few test where only someone operating the machine can tell if it was successful or if the test failed. !LCD will always say success in these tests!!
    LCD display has about 3sec delay so be patient (if you don't need it, all sensors work with no delay).
    One test lead to the next one and button is activated only, when the test is finished.

    How to operate tests:
    	Open/Close cover - Open/Close the cover 6x and if it doesn't says "Test successful", there is something wrong with cover pin.
    	Insert/Remove IPA tank - Insert/Remove the tank	6x and if it doesn't says "Test successful", there is something wrong with IPA tank pin.
    	Ventilation test - It speeds up FAN1 & FAN2 rotation every 10sec until 0:20 when it stays on 90% until the end. You can see how many interupts from the sensors it gets every 100ms.
    					   If any of those fans trigger fan_error, test ends with "Test Failed".
		LED test - It lights up for 10mins, then it lights out after count-down finishes. Test will notice if LEDs switched off before timer finished. Cover needs to be closed.
		Heater test - It won't start testing with IPA tank. There is a timer at 10mins and thermometer. Cover should be closed but it will run anyway.
					  It triggers heater_error and says "Test Failed" if FAN3 doesn't work properly.
		Rotation speed test - LCD screen says "Mode/Gear". Mode = 1 (curing mode == true), Mode = 0 (washing mode == true), Gear increments from 1 to 10 every 10 secs.
							  !LCD ALWAYS SAYS SUCCESS! TESTER NEEDED to see if motor spins the plate properly and rotations rise properly.
*/

class Selftest {
public:
/**
 * Constructor sets everything to false apart from fans_speed, because ventilation test starts on 10%
 */
	Selftest();
/**
* Universal pin test triggers change in sensors state and counts it up. It is used on Cover test and IPA tank test.
*/
	bool universal_pin_test();
/**
* Ventilation test counts down 1 minute and every 10sec it rises fan1 and fan2 duty by 20% (starting at 10%). When it triggers fan error, it stops the test.
* Possible improvements in the future: Print out which fan failed the test (measure_state, prev_measure_state).
* @param fan_error - triggers wrong fan behavior
*/
	void ventilation_test(bool);
/**
 * According to selftest's phase and state of the test it sends appropriate message
 * @return it returns message right in lcd.print();
 */
	const char* print();
/**
 * Cleans up handy variables between the phases of the selftest. Enables to recycle variables across tests.
 */
	void clean_up();
/**
 * Counts down 10sec time periods in Rotation test.
 */
	bool motor_rotation_timer();
/**
 * Counts down 10 minute of UV light.
 */
	void LED_test();
/**
 * Useful for each test setup
 * @return true if it is first loop of the test.
 */
	bool is_first_loop();
/**
 * Sets first_loop value
 * @param True or False
 */
	void set_first_loop(const bool);
/**
 * Counts down 10min of heater test and stops the test if heater_error(fan3 error) is triggered.
 * @param heater_error - is true if fan3 doesn't work properly.
 */
	void heat_test(bool);

	uint8_t phase;				/**< Stores which test("phase" of selftest) is running*/
	uint8_t fan_tacho[2];		/**< Stores measured rotation per 1ms for ventilation_test*/
	bool cover_test;			/**< Stores if cover test is finished*/
	bool tank_test;				/**< Stores if IPA tank test is finished*/
	bool vent_test;				/**< Stores if ventilation test is finished*/
	bool heater_test;			/**< Stores if heater test is finished*/
	bool rotation_test;			/**< Stores if rotation test is finished*/
	bool led_test;				/**< Stores if LED test is finished*/
	uint8_t fans_speed[2];		/**< Stores duty of fans (0-100%)*/
	Countimer tCountDown;		/**< Counts down time periods in tests, where is needed*/
	bool cover_down;			/**< Tells if cover is down(eye's safe measurement)*/
	bool fail_flag;				/**< Tells whether test passed or failed*/
	bool measured_state;		/**< Universal bool variable for measurements and flags*/
	bool helper;

private:

	bool first_loop;			/**< Stores true if it is the first loop of the test. Used to set up tests*/
	bool prev_measured_state;	/**< Universal bool variable for measurements and flags*/
	uint8_t counter;			/**< Universal uint8_t variable for counting*/

};


#endif	//SELFTEST_H
