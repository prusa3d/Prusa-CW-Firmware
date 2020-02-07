PROJECT = Prusa-CW-Firmware
DIRS = Prusa-CW-Firmware board core libraries
I18N = i18n
LANG_TEMPLATE = ${I18N}/${PROJECT}.pot
LANGS = en #cs

CC = avr-gcc
CPP = avr-g++
OBJCOPY = avr-objcopy

CSTANDARD = -std=gnu11
CPPSTANDARD = -std=gnu++11

WARN = -Wall -Wextra
CPPTUNING = -fno-exceptions -fno-threadsafe-statics
OPT = -g -Os -ffunction-sections -fdata-sections -flto -fno-fat-lto-objects -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -fno-inline-small-functions -mcall-prologues -fno-split-wide-types
MCU = -mmcu=atmega32u4

DEFS = -DF_CPU=16000000 -DARDUINO=10805 -DUSB_VID=0x2c99 -DUSB_PID=0x0008 -DUSB_MANUFACTURER='"Prusa Research prusa3d.com"' -DUSB_PRODUCT='"Original Prusa CW1"'
INCLUDE := $(foreach dir, ${DIRS}, -I${dir}) -I${I18N} -I.

CFLAGS = ${OPT} ${WARN} ${CSTANDARD} ${MCU} ${INCLUDE} ${DEFS}
CPPFLAGS = ${OPT} ${WARN} ${CPPSTANDARD} ${CPPTUNING} ${MCU} ${INCLUDE} ${DEFS}
LINKFLAGS = -fuse-linker-plugin -Wl,--relax,--gc-sections,--defsym=__TEXT_REGION_LENGTH__=28k

CSRCS := $(foreach dir, ${DIRS}, $(wildcard ${dir}/*.c))
CPPSRCS := $(foreach dir, ${DIRS}, $(wildcard ${dir}/*.cpp))
OBJS = ${CSRCS:%.c=%.o} ${CPPSRCS:%.cpp=%.o}
DEPS = ${CSRCS:.c=.d} ${CPPSRCS:%.cpp=%.dd}
HEXS := $(foreach lang, ${LANGS}, ${PROJECT}-${lang}.hex)

COMPILE.c = ${CC} ${CFLAGS} -c
COMPILE.cpp = ${CPP} ${CPPFLAGS} -c

.PHONY: version-tmp.h clean lang_extract default

.PRECIOUS: %.elf

default: ${HEXS}

%.hex: %.elf
	${OBJCOPY} -O ihex -R .eeprom $< $@.tmp
	cat ${PROJECT}.hex.in $@.tmp > $@
	rm $@.tmp

%.elf: version.h Makefile ${OBJS}
	${CPP} ${CPPFLAGS} ${LINKFLAGS},-Map=${@:%.elf=%.map} -o $@ ${OBJS}

version.h: version-tmp.h
	@if ! cmp -s $< $@; then cp $< $@; fi
	@rm $<

version-tmp.h:
	@echo "#pragma once" > $@
	@echo -n "#define FW_LOCAL_CHANGES " >> $@
	@git diff-index --quiet HEAD -- && echo 0 >> $@ || echo 1 >> $@
	@echo -n "#define FW_BUILDNR " >> $@
	@echo "\"`git rev-list --count HEAD`\"" >> $@
	@echo -n "#define FW_HASH " >> $@
	@echo "\"`git rev-parse HEAD`\"" >> $@
	@echo -n "#define FW_VERSION " >> $@
	@echo "\"`git describe --abbrev=0 --tags`\"" >> $@

clean:
	rm -rf ${OBJS} ${DEPS} version.h $(wildcard *.map) $(wildcard *.elf) $(wildcard *.hex) ${LANG_TEMPLATE}

lang_extract: ${LANG_TEMPLATE}

$(LANG_TEMPLATE): $(wildcard ${I18N}/*.h)
	touch $@
	xgettext --join-existing --sort-output --keyword=_ --output=$@ $?

xxx:
	msgconv --stringtable-output cs.po

%.d: %.c version.h
	@${CC} ${CFLAGS} $< -MM -MT ${@:.d=.o} >$@

%.dd: %.cpp version.h
	@${CPP} ${CPPFLAGS} $< -MM -MT ${@:.d=.o} >$@

-include ${DEPS}
