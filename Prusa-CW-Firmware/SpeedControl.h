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
* Speed_configuration calculates user-defined speed and sets up speeding process (washing mode).
* @param curing_mode - devides function into two states: curing_mode & washing mode.
*/
	void speed_configuration(bool curing_mode);
/**
* Speeding_up handles acceleration. There is an option to print out "current.wanted" values through USB (SERIAL_COM_DEBUG macro)
*/
	void speeding_up();

	byte uni_speed_var;		/**< Universal variable for passing current speed of rotation. Must be byte because it is passed in interuption process.*/
	byte washing_speed;		/**< User-defined value from 1 to 10, loaded from eeprom. */
	byte curing_speed;		/**< User-defined value from 1 to 10, loaded from eeprom. */
	bool speed_up;			/**< Speeding up process flag */

private:

	const unsigned int rotation_start = 200;	/**< Smaller = faster */
	const unsigned int min_curing_speed = 220;	/**< Smaller = faster */
	const unsigned int max_curing_speed = 25;	/**< Smaller = faster */
	const unsigned int min_washing_speed = 70;	/**< Smaller = faster */
	const unsigned int max_washing_speed = 16;	/**< Smaller = faster */
	unsigned int set_curing_speed;			/**< Stores wanted speed mapped in speed_configuration() */
	unsigned int set_washing_speed;			/**< Stores wanted speed mapped in speed_configuration() */

};

#endif  //SPEED_CONTROL_H
