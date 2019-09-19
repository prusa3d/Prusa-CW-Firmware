# Prusa-CW-Firmware
Original Prusa Curing and Washing machine firmware

## Table of contents

<!--ts-->
   * [Building](#building)
     * [Cmake](#cmake)
       * [Automatic, remote, using travis-ci](#automatic-remote-using-travis-ci)
       * [Automatic, local, using script and prepared tools package](#automatic-local-using-script-and-prepared-tools-package)
       * [Manually with installed tools](#manually-with-installed-tools)
   * [Flashing](#flashing)
   * [Building documentation](#building-documentation)
     
<!--te-->

## Building
### Cmake
#### Automatic, remote, using travis-ci

Create new github user, eg. your_user_name-your_repository_name-travis. This step is not mandatory, only recomended to limit access rights for travis to single repository. Grant this user access to your repository. Register this user on https://travis-ci.org/. Create API key for this user. In Github click on this user, settings, Developer settings, Personal access tokens, Generate new token, select public_repo, click on Generate token. Copy this token.
Login into https://travis-ci.org/ enable build of your repository, click on repository setting, add environment variable ACCESS_TOKEN. As value paste your token.

Each commit is build, but only for tagged commits Prusa-CW-Firmware.hex is attached to your release by travis.

#### Automatic, local, using script and prepared tools package
##### Linux

You need unzip and wget tools.

run ./build.sh

It downloads neccessary tools, extracts it to ../MM-build-env-\<version\>, creates ../Prusa-CW-Firmware-build folder, configures build scripts in it and builds it using ninja.

##### Windows

Download MM-build-env-Win64-<version>.zip from https://github.com/prusa3d/MM-build-env/releases. Unpack it. Run configure.bat. This opens cmake-gui with preconfigured tools paths. Select path where is your source code located, select where you wish to build - out of source build is recomended. Click on generate, select generator - Ninja, or \<Your favourite IDE\> - Ninja.
  
Run build.bat generated in your binary directory.

#### Manually with installed tools

You need cmake, avr-gcc, avr-libc and cmake supported build system (e.g. ninja) installed.

Out of source tree build is recommended, in case of Eclipse CDT project file generation is necceessary. If you don't want out of source tree build, you can skip this step.
~~~
cd ..
mkdir Prusa-CW-Firmware-build
cd Prusa-CW-Firmware-build
~~~
Generate build system - consult cmake --help for build systems generators and IDE project files supported on your platform.
~~~
cmake -G "build system generator" path_to_source
~~~
example 1 - build system only
~~~
cmake -G "Ninja" ../Prusa-CW-Firmware
~~~
example 2 - build system and project file for your IDE
~~~
cmake -G "Eclipse CDT4 - Ninja ../Prusa-CW-Firmware-build
~~~
Invoke build system you selected in previous step. Example:
~~~
ninja
~~~
file MM-control-01.hex is generated.

## Flashing
### Windows
#### Slic3er
Configuration > Flash printer firmware, then fill up the path to generated .hex and hit Flash!
#### Avrdude
Board needs to be reset to bootloader. Bootloader has 5 seconds timeout and then returns to the application.

This can be accomplished manually by clicking reset button on MMU, or programmatically by opening and closing its virtual serial line at baudrate 1500.

Than flash it using following command, replace \<virtual serial port\> with CDC device created by MMU usually com\<nr.\> under Windows and /dev/ttyACM\<nr.\> under Linux. -b baud rate is don't care value, probably doesn't have to be specified at all, as there is no physical uart.
~~~
avrdude -v -p atmega32u4 -c avr109 -P <virtual serial port> -b 57600 -D -U flash:w:Prusa-CW-Firmware-build.hex:i
~~~

### Linux
Same as Windows, but there is known issue with ModemManager:

If you have the modemmanager installed, you either need to deinstall it, or blacklist the Prusa Research USB devices:

~~~
/etc/udev/rules.d/99-mm.rules

# Original Prusa i3 MK3 Multi Material 2.0 upgrade
ATTRS{idVendor}=="2c99", ATTRS{idProduct}=="0007", ENV{ID_MM_DEVICE_IGNORE}="1"
ATTRS{idVendor}=="2c99", ATTRS{idProduct}=="0008", ENV{ID_MM_DEVICE_IGNORE}="1"

$ sudo udevadm control --reload-rules
~~~
A request has been sent to Ubuntu, Debian and ModemManager to blacklist the whole Prusa Research VID space.

https://bugs.launchpad.net/ubuntu/+source/modemmanager/+bug/1781975

https://bugs.debian.org/cgi-bin/pkgreport.cgi?dist=unstable;package=modemmanager

and reported to
https://lists.freedesktop.org/archives/modemmanager-devel/2018-July/006471.html

## Building documentation
Run doxygen in Prusa-CW-Firmware folder.
Documentation is generated in Doc subfolder.
