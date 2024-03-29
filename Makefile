# CW1S is the default, use `make DEVICE=CW1` for CW1
DEVICE = CW1S
#DEVICE = CW1
PROJECT = Prusa-${DEVICE}-Firmware
DIRS = lib src
DEVICES_DIR = devices
I18N = i18n
LANG = en
BUILD_DIR = build

CC = avr-gcc
CPP = avr-g++
OBJCOPY = avr-objcopy

CSTANDARD = -std=gnu11
CPPSTANDARD = -std=gnu++17

WARN = -Wall -Wextra
CPPTUNING = -fno-exceptions -fno-threadsafe-statics
OPT = -g -Os -ffunction-sections -fdata-sections -flto -fno-fat-lto-objects -funsigned-char -funsigned-bitfields -fshort-enums -fno-inline-small-functions -mcall-prologues -fno-split-wide-types
MCU = -mmcu=atmega32u4

ifeq (${DEVICE}, CW1)
USB_PID = 0x0008
else ifeq (${DEVICE}, CW1S)
USB_PID = 0x000F
else
$(error Use CW1 or CW1S as DEVICE)
endif

DEFS = -DF_CPU=16000000 -DARDUINO=10805 -DUSB_VID=0x2c99 -DUSB_PID=${USB_PID} -DUSB_MANUFACTURER='"Prusa Research prusa3d.com"' -DUSB_PRODUCT='"Original Prusa ${DEVICE}"' -D${DEVICE}_HW
INCLUDE = $(foreach dir, ${DIRS}, -I${dir}) -I${DEVICES_DIR} -I${BUILD_DIR}

CFLAGS = ${OPT} ${WARN} ${CSTANDARD} ${MCU} ${INCLUDE} ${DEFS}
CPPFLAGS = ${OPT} ${WARN} ${CPPSTANDARD} ${CPPTUNING} ${MCU} ${INCLUDE} ${DEFS}
LINKFLAGS = -fuse-linker-plugin -Wl,--relax,--gc-sections,--defsym=__TEXT_REGION_LENGTH__=28k,--defsym=__DATA_REGION_LENGTH__=2560,--print-memory-usage

DEVICE_LOWER := $(shell echo ${DEVICE} | tr '[:upper:]' '[:lower:]')
CSRCS := $(foreach dir, ${DIRS}, $(wildcard ${dir}/*.c))
CPPSRCS := $(foreach dir, ${DIRS}, $(wildcard ${dir}/*.cpp)) ${DEVICES_DIR}/${DEVICE_LOWER}.cpp
OBJS := $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.o, $(CSRCS))) $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.o, $(CPPSRCS)))
DEPS := $(addprefix $(BUILD_DIR)/, $(patsubst %.c, %.d, $(CSRCS))) $(addprefix $(BUILD_DIR)/, $(patsubst %.cpp, %.dd, $(CPPSRCS)))
VERSION := $(shell git describe --abbrev=0 --tags)
VERSION_FILE = ${BUILD_DIR}/version.h
LANG_TEMPLATE = ${I18N}/${PROJECT}-${VERSION}.pot

default: DEFS += -DSERIAL_COM_DEBUG
default: $(addprefix $(BUILD_DIR)/, ${PROJECT}-${LANG}-devel.hex)

dist: $(addprefix $(BUILD_DIR)/, ${PROJECT}-${LANG}-${VERSION}.hex)

.PHONY: clean distclean lang_extract default dist ${VERSION_FILE}.tmp doc

.SECONDARY:

.SECONDEXPANSION:

$(BUILD_DIR)/.:
	@mkdir -p $@

$(BUILD_DIR)%/.:
	@mkdir -p $@

$(BUILD_DIR)/%.hex: ${BUILD_DIR}/%.elf
	${OBJCOPY} -O ihex -R .eeprom $< $@.tmp
	@echo "; device = ${DEVICE_LOWER}" > $@
	@echo >> $@
	cat $@.tmp >> $@
	rm $@.tmp

$(BUILD_DIR)/%.elf: ${OBJS}
	@echo "LINK $@"
	@${CPP} ${CPPFLAGS} ${LINKFLAGS},-Map=${@:%.elf=%.map} $^ -o $@

$(BUILD_DIR)/%.o: %.c | $${@D}/.
	@echo "CC $<"
	@${CC} ${CFLAGS} -c $< -o $@

$(BUILD_DIR)/%.o: %.cpp | $${@D}/.
	@echo "CPP $<"
	@${CPP} ${CPPFLAGS} -c $< -o $@

$(VERSION_FILE): ${VERSION_FILE}.tmp
	@if ! cmp -s $< $@; then cp $< $@; fi

$(VERSION_FILE).tmp: ${BUILD_DIR}/${LANG}.h | $${@D}/.
	@echo "#pragma once" > $@
	@echo -n "#define FW_LOCAL_CHANGES " >> $@
	@git diff-index --quiet HEAD -- && echo 0 >> $@ || echo 1 >> $@
	@echo -n "#define FW_BUILDNR " >> $@
	@echo "\"`git rev-list --count HEAD`\"" >> $@
	@echo -n "#define FW_HASH " >> $@
	@echo "\"`git rev-parse --short=18 HEAD`\"" >> $@
	@echo -n "#define FW_VERSION " >> $@
	@echo "\"${VERSION}\"" >> $@
	@echo -n "#include " >> $@
	@echo "\"${LANG}.h\"" >> $@

clean:
	rm -f $(foreach dir, ${DIRS} ${DEVICES_DIR}, $(wildcard ${BUILD_DIR}/${dir}/*.o)) $(foreach dir, ${DIRS} ${DEVICES_DIR}, $(wildcard ${BUILD_DIR}/${dir}/*.d*)) ${BUILD_DIR}/*.h $(VERSION_FILE).tmp ${BUILD_DIR}/*.sed

distclean: clean
	rm -rf ${BUILD_DIR}/*.hex ${BUILD_DIR}/*.elf ${BUILD_DIR}/*.map ${I18N}/*.pot tags doc

lang_extract: ${LANG_TEMPLATE}

$(LANG_TEMPLATE): ${I18N}/en.h
	touch $@
	xgettext --join-existing --sort-output --keyword=_ --output=$@ $?

$(BUILD_DIR)/%.sed: ${I18N}/%.po
	msgconv --stringtable-output $< |grep -E '".+" ='|sed 's/"\(.*\)" = "\(.*\)";/s~"\1"~"\2"~/'|sed 's~\[~\\\[~g;s~\]~\\\]~g' > $@

$(BUILD_DIR)/en.h: ${I18N}/en.h
	cp $< $@

$(BUILD_DIR)/%.h: ${BUILD_DIR}/%.sed
	sed -f $< ${I18N}/en.h > $@

tags: ${CSRCS} ${CPPSRCS} $(wildcard ${I18N}/*.h)
	arduino-ctags $^

doc:
	doxygen


$(BUILD_DIR)/%.d: %.c ${VERSION_FILE} Makefile | $${@D}/.
	@echo "deps $<"
	@${CC} ${CFLAGS} $< -MM -MT ${@:.d=.o} >$@

$(BUILD_DIR)/%.dd: %.cpp ${VERSION_FILE} Makefile | $${@D}/.
	@echo "deps $<"
	@${CPP} ${CPPFLAGS} $< -MM -MT ${@:.dd=.o} >$@

-include ${DEPS}
