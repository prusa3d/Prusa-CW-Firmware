cmake_minimum_required(VERSION 3.0)

SET(CMAKE_SYSTEM_NAME Generic)
SET(CMAKE_C_COMPILER avr-gcc)
SET(CMAKE_CXX_COMPILER avr-g++)
find_program(CAT cat)

SET(CSTANDARD "-std=gnu11")
SET(CPPSTANDARD "-std=gnu++11")

SET(WARN "-Wall -Wextra")
SET(CPPTUNING "-fno-exceptions -fno-threadsafe-statics")
SET(OPT "-Os -ffunction-sections -fdata-sections -flto -fno-fat-lto-objects")
SET(MCU "-mmcu=atmega32u4")

add_compile_definitions(
	F_CPU=16000000
	ARDUINO=10805
	USB_VID=0x2c99
	USB_PID=0x0008
	USB_MANUFACTURER="Prusa Research prusa3d.com"
	USB_PRODUCT="Original Prusa CW1"
)

SET(CFLAGS "${OPT} ${WARN} ${CSTANDARD} ${MCU}")
SET(CXXFLAGS "${OPT} ${WARN} ${CPPSTANDARD} ${CPPTUNING} ${MCU}")
SET(ASMFLAGS "${OPT} ${WARN} ${MCU}")

SET(CMAKE_C_FLAGS ${CFLAGS})
SET(CMAKE_CXX_FLAGS ${CXXFLAGS})
SET(CMAKE_ASM_FLAGS ${ASMFLAGS})
SET(CMAKE_EXE_LINKER_FLAGS "-fuse-linker-plugin -Wl,--gc-sections")


project(Prusa-CW-Firmware C CXX ASM)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(GetGitRevisionDescription)

get_git_head_revision(GIT_HEAD_REF GIT_COMMIT_HASH)
git_count_parent_commits(GIT_PARENT_COMMITS)
git_local_changes(GIT_LOCAL_CHANGES)

configure_file(
	${CMAKE_SOURCE_DIR}/Prusa-CW-Firmware/version.h.in
	version.h
)
# The 'real' git information file
set(GITREV_BARE_FILE dirty.h)
# The temporary git information file
set(GITREV_BARE_TMP dirty-tmp.h)
set(GITREV_FILE ${CMAKE_BINARY_DIR}/${GITREV_BARE_FILE})
set(GITREV_TMP ${CMAKE_BINARY_DIR}/${GITREV_BARE_TMP})

add_executable(
	Prusa-CW-Firmware
	Prusa-CW-Firmware/Countimer.cpp
	Prusa-CW-Firmware/main.cpp
	Prusa-CW-Firmware/MCP23S17.cpp
	Prusa-CW-Firmware/thermistor.cpp
	Prusa-CW-Firmware/Trinamic_TMC2130.cpp
	Prusa-CW-Firmware/MenuList.cpp
	libraries/LiquidCrystal.cpp
	libraries/SPI.cpp
	core/WInterrupts.c
	core/hooks.c
	core/wiring.c
	core/wiring_analog.c
	core/wiring_digital.c
	core/CDC.cpp
	core/PluggableUSB.cpp
	core/Print.cpp
	core/USBCore.cpp
	core/WMath.cpp
	core/WString.cpp
	core/main.cpp
	${GITREV_TMP}
)
target_include_directories(Prusa-CW-Firmware PRIVATE board core libraries ${CMAKE_BINARY_DIR})


add_custom_command(
	OUTPUT		Prusa-CW-Firmware.hex
	MAIN_DEPENDENCY	Prusa-CW-Firmware.hex.tmp
	DEPENDS		${CMAKE_SOURCE_DIR}/Prusa-CW-Firmware.hex.in
	COMMAND		${CAT}
	ARGS		${CMAKE_SOURCE_DIR}/Prusa-CW-Firmware.hex.in
			Prusa-CW-Firmware.hex.tmp
			> Prusa-CW-Firmware.hex
	COMMENT		"Generating HEX image"
	VERBATIM
)

add_custom_command(
	OUTPUT		Prusa-CW-Firmware.hex.tmp
	MAIN_DEPENDENCY	Prusa-CW-Firmware
	COMMAND		${CMAKE_OBJCOPY}
	ARGS		-O ihex -R .eeprom
			Prusa-CW-Firmware
			Prusa-CW-Firmware.hex.tmp
	VERBATIM
)

add_custom_target(
	hex ALL
	DEPENDS Prusa-CW-Firmware.hex
)



add_custom_command(
  OUTPUT ${GITREV_TMP}
  COMMAND ${CMAKE_COMMAND} -E echo_append "#define FW_LOCAL_CHANGES " > ${GITREV_TMP}
  COMMAND ${GIT_EXECUTABLE} diff-index --quiet HEAD -- && echo 0 >> ${GITREV_TMP} || echo 1 >> ${GITREV_TMP}
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${GITREV_TMP} ${GITREV_FILE}
  COMMAND ${CMAKE_COMMAND} -E remove ${GITREV_TMP}
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  VERBATIM
)

if ("Windows" STREQUAL CMAKE_HOST_SYSTEM_NAME)
configure_file(
${CMAKE_SOURCE_DIR}/build.bat.in
build.bat
)
endif()
