PROJECT = Prusa-CW1-Firmware
DIRS = lib src
I18N = i18n
LANG_TEMPLATE = ${I18N}/${PROJECT}.pot
LANGS = en #cs
BUILD_DIR := build

CC = avr-gcc
CPP = avr-g++
OBJCOPY = avr-objcopy

CSTANDARD = -std=gnu11
CPPSTANDARD = -std=gnu++17

WARN = -Wall -Wextra
CPPTUNING = -fno-exceptions -fno-threadsafe-statics
OPT = -g -Os -ffunction-sections -fdata-sections -flto -fno-fat-lto-objects -funsigned-char -funsigned-bitfields -fshort-enums -fno-inline-small-functions -mcall-prologues -fno-split-wide-types
MCU = -mmcu=atmega32u4

DEFS = -DF_CPU=16000000 -DARDUINO=10805 -DUSB_VID=0x2c99 -DUSB_PID=0x0008 -DUSB_MANUFACTURER='"Prusa Research prusa3d.com"' -DUSB_PRODUCT='"Original Prusa CW1"'
INCLUDE := $(foreach dir, ${DIRS}, -I${dir}) -I${I18N} -I${BUILD_DIR}

CFLAGS = ${OPT} ${WARN} ${CSTANDARD} ${MCU} ${INCLUDE} ${DEFS}
CPPFLAGS = ${OPT} ${WARN} ${CPPSTANDARD} ${CPPTUNING} ${MCU} ${INCLUDE} ${DEFS}
LINKFLAGS = -fuse-linker-plugin -Wl,--relax,--gc-sections,--defsym=__TEXT_REGION_LENGTH__=28k

CSRCS := $(foreach dir, ${DIRS}, $(wildcard ${dir}/*.c))
CPPSRCS := $(foreach dir, ${DIRS}, $(wildcard ${dir}/*.cpp))
OBJS = $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(CSRCS))) $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.o, $(CPPSRCS)))
DEPS = $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.d, $(CSRCS))) $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.dd, $(CPPSRCS)))
HEXS := $(foreach lang, ${LANGS}, $(addprefix $(BUILD_DIR)/, ${PROJECT}-${lang}.hex))
VERSION = ${BUILD_DIR}/version.h

default: ${HEXS}

.PHONY: clean lang_extract default ${VERSION}.tmp doc

.SECONDARY:

.SECONDEXPANSION:

$(BUILD_DIR)/.:
	@mkdir -p $@

$(BUILD_DIR)%/.:
	@mkdir -p $@

$(BUILD_DIR)/%.hex: ${BUILD_DIR}/%.elf
	${OBJCOPY} -O ihex -R .eeprom $< $@.tmp
	@echo -e "; device = cw1\n" > $@
	cat $@.tmp >> $@
	rm $@.tmp

$(BUILD_DIR)/%.elf: ${OBJS}
	${CPP} ${CPPFLAGS} ${LINKFLAGS},-Map=${@:%.elf=%.map} $^ -o $@

$(BUILD_DIR)/%.o: %.c | $${@D}/.
	${CC} ${CFLAGS} -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp | $${@D}/.
	${CPP} ${CPPFLAGS} -c $< -o $@

$(VERSION): ${VERSION}.tmp
	@if ! cmp -s $< $@; then cp $< $@; fi

$(VERSION).tmp: | $${@D}/.
	@echo "#pragma once" > $@
	@echo -n "#define FW_LOCAL_CHANGES " >> $@
	@git diff-index --quiet HEAD -- && echo 0 >> $@ || echo 1 >> $@
	@echo -n "#define FW_BUILDNR " >> $@
	@echo "\"`git rev-list --count HEAD`\"" >> $@
	@echo -n "#define FW_HASH " >> $@
	@echo "\"`git rev-parse --short=18 HEAD`\"" >> $@
	@echo -n "#define FW_VERSION " >> $@
	@echo "\"`git describe --abbrev=0 --tags`\"" >> $@

clean:
	rm -rf ${OBJS} ${DEPS} ${VERSION} ${VERSION}.tmp ${HEXS} ${HEXS:%.hex=%.elf} ${HEXS:%.hex=%.map} ${LANG_TEMPLATE} tags doc

lang_extract: ${LANG_TEMPLATE}

$(LANG_TEMPLATE): $(wildcard ${I18N}/*.h)
	touch $@
	xgettext --join-existing --sort-output --keyword=_ --output=$@ $?

xxx:
	msgconv --stringtable-output cs.po

tags: ${CSRCS} ${CPPSRCS} $(wildcard ${I18N}/*.h)
	arduino-ctags $^

doc:
	doxygen


$(BUILD_DIR)/%.d: %.c ${VERSION} Makefile | $${@D}/.
	@${CC} ${CFLAGS} $< -MM -MT ${@:.d=.o} >$@

$(BUILD_DIR)/%.dd: %.cpp ${VERSION} Makefile | $${@D}/.
	@${CPP} ${CPPFLAGS} $< -MM -MT ${@:.dd=.o} >$@

-include ${DEPS}
