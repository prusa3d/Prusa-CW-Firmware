#ifndef SPEED_CONTROL_H
#define SPEED_CONTROL_H

//#define SERIAL_COM_DEBUG

#include "Arduino.h"

/*! \class CSpeedControl
    \brief A class that handles variables and functions related to rotation speed.

    It keeps all those variables and functions on one spot.
*/

class CSpeedControl {
public:

	CSpeedControl();
	~CSpeedControl();
/**
* Speed_configuration calculates user-defined time period of microsteps and sets up acceleration process (washing mode).
* @param curing_mode - devides function into two states: curing_mode & washing mode.
*/
	void speed_configuration(bool curing_mode);
/**
* acceleration50ms handles acceleration of the motor every 50 ms. There is an option to print out "current.wanted" values through USB (SERIAL_COM_DEBUG macro)
*/
	void acceleration50ms();

	byte microstep_control;		/**< Universal variable for passing current speed of rotation. Must be byte because it is passed in interuption process.*/
	byte washing_speed;			/**< User-defined value from 1 to 10, loaded from eeprom. */
	byte curing_speed;			/**< User-defined value from 1 to 10, loaded from eeprom. */
	bool acceleration_flag;		/**< Activates acceleration process */
	bool motor_running;			/**< Determines if motor is running (for stepper) */

private:

	const uint8_t rotation_start = 200;	/**< Smaller = faster */
	const uint8_t min_curing_speed = 220;	/**< Smaller = faster */
	const uint8_t max_curing_speed = 25;	/**< Smaller = faster */
	const uint8_t min_washing_speed = 70;	/**< Smaller = faster */
	const uint8_t max_washing_speed = 16;	/**< Smaller = faster */
	uint8_t target_curing_period;			/**< Stores wanted time period of microsteps mapped in speed_configuration() */
	uint8_t target_washing_period;			/**< Stores wanted time period of microsteps mapped in speed_configuration() */

};

#endif  //SPEED_CONTROL_H
