# Prusa CW1/CW1S Firmware
Original Prusa Curing and Washing machine firmware

## Table of contents

<!--ts-->
   * [Function notes](#function-notes)
       * [Hidden menu](#hidden-menu)
       * [Selftest](#selftest)
       * [Cooling](#cooling)
       * [Temperature display on CW1](#temperature-display-on-cw1)
       * [Hold the platform](#hold-the-platform)
   * [Building with make](#building-with-make)
       * [Automatic, local, using script and prepared tools package](#automatic-local-using-script-and-prepared-tools-package)
       * [Manually with installed tools](#manually-with-installed-tools)
   * [Flashing](#flashing)
   * [Building documentation](#building-documentation)

<!--te-->

## Function notes
### Hidden menu

You can get to the “hidden” settings menu by long pressing the rotator in the
settings menu. Contains UVLED intensity, Factory reset, Cooldown and Selftest.

### Selftest

You can use the self-test to test the functionality of your device. The
semi-automatic tests require your cooperation and are the lid switch test, the
IPA container detection test, and the motor test (which visually assesses that
the platform is moving). This is followed by fan, UV LED and heating tests.
They will run by themselves, all you have to do is close the lid.

### Cooling

The fans are controlled in such a way that the device is cooled as best as
possible. In some cases, the fans can run at full power, which can cause more
noise. That's normal.

### Temperature display on CW1

Due to the location of the thermistor in CW1 (not applicable to CW1S), the
temperature measurement inside the device is relatively imprecise and is
subsequently compensated. For this reason, the temperature is shown on the
display after 2 minutes from the start of heating. Until then, “-- ˚C” is
displayed.

### Hold the platform

Original Prusa CW1S or CW1 platform upgrade brings a removable platform
protected with an FEP film. To help align the platform back to its original
position, the user can activate “Hold the platform” from the menu. The motor
gets “locked” (rotation is disabled) by the system, temporarily allowing the
user to align the platform properly. Once the platform is properly seated, exit
the menu to “unlock” the motor.

## Building with make
### Automatic, local, using script and prepared tools package

You need to install unzip, wget and make tools, next run the following command:
~~~
./build.sh
~~~
It downloads neccessary tools, creates `build` subfolder, extracts tools to `build/env`, builds firmware using make and puts the result into `build` subfolder.

### Manually with installed tools

You need make, avr-gcc and avr-libc installed.

You can use the DEVICE=CW1 or DEVICE=CW1S parameter to select which device you want to build, the default is CW1S.

To build version with debug turned on use:
~~~
make
~~~
or with language code:
~~~
make LANG=xx
~~~
The file `build/Prusa-CW1S-Firmware-LANG-devel.hex` will be generated.

To build version without debug use:
~~~
make dist
~~~
or with language code:
~~~
make DEVICE=CW1 LANG=xx dist
~~~
The file `build/Prusa-CW1-Firmware-LANG-GIT_TAG.hex` will be generated.

## Flashing
### PrusaSlicer (previously Slic3er PE)

Configuration > Flash printer firmware, then fill up the path to generated .hex and hit Flash!

### Avrdude

Board needs to be reset to bootloader. Bootloader has 5 seconds timeout and then returns to the application.

This can be accomplished manually by pressing reset button on CW1/CW1S (near microUSB port), or programmatically by opening and closing its virtual serial line at baudrate 1200.
Linux command (replace \<virtual serial port\> with CDC device created by CW1/CW1S usually `/dev/ttyACM<nr>`):
~~~
stty -F <virtual serial port> 1200 crtscts; while :; do sleep 0.5; [ -c <virtual serial port> ] && break; done;
~~~
Then flash it using following command, replace \<virtual serial port\> with CDC device created by CW1/CW1S usually `com<nr>` under Windows and `/dev/ttyACM<nr>` under Linux. `-b` baud rate is don't care value, probably doesn't have to be specified at all, as there is no physical uart.
~~~
avrdude -v -p atmega32u4 -c avr109 -P <virtual serial port> -b 57600 -D -U flash:w:build/Prusa-DEVICE-Firmware-LANG-GIT_TAG.hex:i
~~~

### Linux with ModemManager installed
There is known issue with `ModemManager`: you either need to deinstall it, or blacklist the Prusa Research USB devices:

* create or edit `/etc/udev/rules.d/99-mm.rules` in order to add:
~~~
# Original Prusa CW1
ATTRS{idVendor}=="2c99", ATTRS{idProduct}=="0007", ENV{ID_MM_DEVICE_IGNORE}="1"
ATTRS{idVendor}=="2c99", ATTRS{idProduct}=="0008", ENV{ID_MM_DEVICE_IGNORE}="1"
# Original Prusa CW1S
ATTRS{idVendor}=="2c99", ATTRS{idProduct}=="000E", ENV{ID_MM_DEVICE_IGNORE}="1"
ATTRS{idVendor}=="2c99", ATTRS{idProduct}=="000F", ENV{ID_MM_DEVICE_IGNORE}="1"
~~~
* Then reload udev rules :
~~~
$ sudo udevadm control --reload-rules
~~~

A request has been sent to Ubuntu, Debian and ModemManager to blacklist the whole Prusa Research VID space.
* https://bugs.launchpad.net/ubuntu/+source/modemmanager/+bug/1781975
* https://bugs.debian.org/cgi-bin/pkgreport.cgi?dist=unstable;package=modemmanager
* and reported to https://lists.freedesktop.org/archives/modemmanager-devel/2018-July/006471.html

## Building documentation

You need make and doxygen installed.
~~~
make doc
~~~
The documentation is generated in `doc/` subfolder.
