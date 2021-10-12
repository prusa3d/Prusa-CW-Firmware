# 3.2.0

## Summary
* Code refactoring and optimizations
* Automatic fans control
* New heating algorithm for both devices
* Set all timers to max. 60 minutes
* Do not handle info pages as menus
* Change motor direction while washing
* Factory reset
* Improved self-test
* Updated documentation

## Detailed description
### Code refactoring and optimizations
* The code for CW1 and CW1S was splitted into separate files for easier modifications.
* The Makefile was improved to support the build of different devices via command line option.
* Pluggable USB in USBCore library was dissabled to save space. It was not used.
* Menu with live values of fans RPM and temperatures was removed to save space.
  It was used only for debugging purposes and had no importance for the user.

### Automatic fans control
Prior to this version, the fans were set to a fixed speed. The fans are now
controlled according to the cooling demand. In most cases, they can remain at
the lowest possible speed and increase to the maximum only when the temperature
rises. This may affect device noise during certain operations.

### New heating algorithm for both devices
The heating method was unified for both devices. Newly, it tries to maintain
the set temperature in the chamber. CW1S allows a smooth change of heating and
the temperature is therefore relatively constant. CW1 can only switch the
heating on and off and, as a result, the temperature in the chamber fluctuates
more. These adjustments were necessary for longer heating times, when CW1
significantly overheated the chamber and CW1S did not heat the chamber
sufficiently. If you have had your favorite heating temperatures, you need to
revise these settings. Please note that the maximum allowed temperature of
60 ˚C (140 ˚F) is the limit and may affect the life of the device.

### Set all timers to max. 60 minutes
All operations (curing, drying, washing, resin preheating) can now take up to
60 minutes.

### Do not handle info pages as menus
The information page is displayed as scrolling text only and allows you to view
the entire serial number.

### Change motor direction while washing
For washing, it is now possible to set the number of washing cycles. One cycle
is the rotation of the impeller in one direction. The length of one cycle is
calculated from the total washing time so that all cycles last the same time.
This could improve the efficiency of washing prints. The default value is one
cycle, i.e. washing without changing direction as before.

### Factory reset
In the advanced settings (long press in the settings), a factory reset is now
available, which applies the default settings of all configurable values.

### Improved self-test
The UV LED and heater test run only for the necessary time instead of a fixed
time.

### Updated documentation
The README has been updated and a CHANGELOG has been created.

# 3.1.0

## Summary
* Support for CW1S
* Drying temperature increased (CW1S)
* Drying session extended
* Hold platform (new feature)
* Improved translations

## Detailed description
This is the final release of firmware 3.1.0, the first firmware release with
support for both the CW1 and the new CW1S. `Attention, the firmware is not
universal, each device requires its own firmware file!`

### Support for CW1S
This release of the firmware brings support for the recently announced Original
Prusa Curing and Washing Machine CW1S. The CW1S features a reworked heating
system, which required a completely redesigned heating algorithm.

### Drying temperature increased (CW1S)
As mentioned above, the CW1S brings reworked internals with the aim to improve
its drying capabilities. This firmware release enables CW1S owners to increase
the target temperature up to 60 °C. Note that the limit for CW1 is 40 °C.

### Drying session extended
Previously, the system allowed for a 10 minutes drying session. This is now
extended to 60 minutes for both CW1 and CW1S.

### Hold platform (new feature)
Original Prusa CW1S brings a removable platform protected with an FEP film.
Once there is a bigger amount of residue on the film, it might be necessary to
replace it with a new one. To help align the platform back to its original
position, the user can activate “Hold platform” from the menu.

The motor gets “locked” (rotation is disabled) by the system, temporarily
allowing the user to align the platform properly. Once the platform is properly
seated, exit the menu to “unlock” the motor.

### Improved translations
Since there are some changes to firmware 3.1.0, the translators have rechecked
all the language strings. Should you find any incorrect translations, please
let us know.


# 3.0.0

## Summary
* Code refactoring and optimizations
* Firmware localized into 7 languages
* Resin preheat
* Extended curing time
* Factory self-test
* Updated user manual

## Detailed description
### Code refactoring and optimizations
The firmware for the CW1 was refactored in order to provide more space in the
memory for new features and also to lower the CPU load. Most of the work on the
code is not visible to the user, however, it was needed for the new functions
introduced in this release.

### Firmware localized into 7 languages
Until now, the firmware for the Original Prusa CW1 was available only in the
English language. With this release, we are adding support for more languages.
Apart from English, users can now also select:
* Czech
* German
* Spanish
* French
* Italian
* Polish

Each language is distributed as a separate firmware file. For example to
install the German language select the file
`Prusa-CW1-Firmware-de-v3.0.0-rc.1.hex`, connect the CW1 to your computer and
flash the file over PrusaSlicer.

### Resin preheat
Some advanced resins, for example, those used in the dental laboratories,
require elevated temperatures in order to improve the viscosity. Without
raising the temperature of the resin, the print will in many cases fail or the
model won't be exposed to the UV light properly.

Based on the feedback from our customers, we have added this preheat feature to
the CW1. Insert the resin in a bottle in the CW1, close the cover and from the
menu select "Resin preheat". Based on the requirements of your resin, you can
adjust the time in the range of 1 - 30 minutes and the temperature 20 - 40 °C.

### Extended curing time
Advanced resins sometimes require longer curing periods and for this reason,
the time was extended to 1 hour (60 minutes). Several resins were tested in our
laboratory and the recommended curing times with CW1 will be included in the
resin datasheet (RDS) on our eshop.

### Factory self-test
Before every unit of CW1 is sent to the customer, a series of tests is
performed to ensure the machine works as expected. The testing procedure is now
part of a semi-automated test routine, with the following parts:
* Cover switch (semi-automatic)
* IPA tank detection (semi-automatic)
* Motor test - platform rotation (semi-automatic)
* Cooling fans (automatic)
* UV LED (automatic)
* Heater (automatic)

### Updated user manual
The technical documentation was updated and is covering all the changes and new
features introduced in this firmware. You can download an online copy of the
CW1 user manual from here https://prusa3d.com/drivers


# 2.1.4

## Summary
* Fan speed optimization

## Detailed description
The RPMs of the fan were optimized to lower the level of noise.


# 2.1.3

## Summary
* Initial version release
