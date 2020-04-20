# Prusa-CW1-Firmware
Original Prusa Curing and Washing machine firmware

## Table of contents

<!--ts-->
   * [Building with make](#building-with-make)
       * [Automatic, remote, using travis-ci](#automatic-remote-using-travis-ci)
       * [Automatic, local, using script and prepared tools package](#automatic-local-using-script-and-prepared-tools-package)
       * [Manually with installed tools](#manually-with-installed-tools)
   * [Flashing](#flashing)
   * [Building documentation](#building-documentation)
     
<!--te-->

## Building with make
### Automatic, remote, using travis-ci

Create new github user, eg. your_user_name-your_repository_name-travis. This step is not mandatory, only recomended to limit access rights for travis to single repository. Grant this user access to your repository. Register this user on https://travis-ci.org/. Create API key for this user. In Github click on this user, settings, Developer settings, Personal access tokens, Generate new token, select public_repo, click on Generate token and copy this token.

Login into https://travis-ci.org/ enable build of your repository, click on repository setting, add environment variable `ACCESS_TOKEN` and paste your token.

Each commit is build, but only for tagged commits `Prusa-CW1-Firmware-en.hex` is attached to your release by travis.

### Automatic, local, using script and prepared tools package

You need to install unzip, wget and make tools, next run the following command:
~~~
./build.sh
~~~
It downloads neccessary tools, creates `build` subfolder, extracts tools to `build/env`, builds firmware using make and puts the result into `build` subfolder.

### Manually with installed tools

You need make, avr-gcc and avr-libc installed.

Invoke make build system:
~~~
make
~~~
The file `build/Prusa-CW1-Firmware-en.hex` is generated.

## Flashing
### PrusaSlicer (previously Slic3er PE)

Configuration > Flash printer firmware, then fill up the path to generated .hex and hit Flash!

### Avrdude

Board needs to be reset to bootloader. Bootloader has 5 seconds timeout and then returns to the application.

This can be accomplished manually by clicking reset button on CW1 (near microUSB port), or programmatically by opening and closing its virtual serial line at baudrate 1500.

Than flash it using following command, replace \<virtual serial port\> with CDC device created by CW1 usually `com<nr>` under Windows and `/dev/ttyACM<nr>` under Linux. `-b` baud rate is don't care value, probably doesn't have to be specified at all, as there is no physical uart.
~~~
avrdude -v -p atmega32u4 -c avr109 -P <virtual serial port> -b 57600 -D -U flash:w:build/Prusa-CW1-Firmware-en.hex:i
~~~

### Linux with ModemManager installed
There is known issue with `ModemManager`: you either need to deinstall it, or blacklist the Prusa Research USB devices:

* create or edit `/etc/udev/rules.d/99-mm.rules` in order to add:
~~~
# Original Prusa CW1
ATTRS{idVendor}=="2c99", ATTRS{idProduct}=="0007", ENV{ID_MM_DEVICE_IGNORE}="1"
ATTRS{idVendor}=="2c99", ATTRS{idProduct}=="0008", ENV{ID_MM_DEVICE_IGNORE}="1"
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
