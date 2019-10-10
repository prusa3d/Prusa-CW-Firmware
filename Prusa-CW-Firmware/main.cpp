#include "Main.h"
#include <EEPROM.h>
#include "Trinamic_TMC2130.h"
#include "MCP23S17.h"
#include "config.h"
#include "Countimer.h"
#include "thermistor.h"
#include <avr/wdt.h>
#include <avr/pgmspace.h>
#include "version.h"
#include <USBCore.h>
#include "PrusaLcd.h"
#include "MenuList.h"
#include "Selftest.h"
#include "SpeedControl.h"
//#include "LiquidCrystal_Prusa.h"

using Ter = PrusaLcd::Terminator;

#define FW_VERSION  "2.1.4"

#define EEPROM_OFFSET 128
#define MAGIC_SIZE 6
//#define SERIAL_COM_DEBUG	//!< Set up for communication through USB

typedef char Serial_num_t[20]; //!< Null terminated string for serial number

static const char pgmstr_back[] PROGMEM = "Back";
static const char pgmstr_fan1_curing[] PROGMEM = "FAN1 curing";
static const char pgmstr_fan1_drying[] PROGMEM = "FAN1 drying";
static const char pgmstr_fan2_curing[] PROGMEM = "FAN2 curing";
static const char pgmstr_fan2_drying[] PROGMEM = "FAN2 drying";
static const char pgmstr_curing[] PROGMEM = "Curing";
static const char pgmstr_drying[] PROGMEM = "Drying";
static const char pgmstr_washing[] PROGMEM = "Washing";
static const char pgmstr_resin_preheat[] PROGMEM = "Resin preheat";
static const char pgmstr_rotation_speed[] PROGMEM = "Rotation speed";
static const char pgmstr_run_mode[] PROGMEM = "Run mode";
static const char pgmstr_preheat[] PROGMEM = "Preheat";
static const char pgmstr_sound[] PROGMEM = "Sound";
static const char pgmstr_fans[] PROGMEM = "Fans";
static const char pgmstr_led_intensity[] PROGMEM = "LED intensity";
static const char pgmstr_information_error[] PROGMEM = "Information ->!!";
static const char pgmstr_information[] PROGMEM = "Information";
static const char pgmstr_unit_system[] PROGMEM = "Unit system";
static const char pgmstr_fw_version[] PROGMEM = "FW version: "  FW_VERSION;
static const char pgmstr_fan1_failure[] PROGMEM = "FAN1 failure";
static const char pgmstr_fan2_failure[] PROGMEM = "FAN2 failure";
static const char pgmstr_heater_failure[] PROGMEM = "HEATER failure";
static const char *pgmstr_serial_number = reinterpret_cast<const char *>(0x7fe0);
static const char pgmstr_build_nr[] PROGMEM = "Build: " FW_BUILDNR;
static const char pgmstr_fw_hash[] PROGMEM = FW_HASH;
static const char pgmstr_workspace[] PROGMEM = FW_LOCAL_CHANGES ? "Workspace dirty" : "Workspace clean";
static const char pgmstr_start_resin_preheat[] PROGMEM = "Start resin preheat";
static const char pgmstr_start_washing[] PROGMEM = "Start washing";
static const char pgmstr_run_time[] PROGMEM = "Run-time";
static const char pgmstr_settings_error[] PROGMEM = "Settings ->!!";
static const char pgmstr_settings[] PROGMEM = "Settings";
static const char pgmstr_start_drying[] PROGMEM = "Start drying";
static const char pgmstr_start_curing[] PROGMEM = "Start curing";
static const char pgmstr_start_drying_curing[] PROGMEM = "Start drying/curing";
static const char pgmstr_selftest[] PROGMEM = "Selftest";
static const char pgmstr_curing_speed[] PROGMEM = "Curing speed";
static const char pgmstr_washing_speed[] PROGMEM = "Washing speed";
static const char pgmstr_preheat_enabled[] PROGMEM = "Preheat enabled";
static const char pgmstr_preheat_disabled[] PROGMEM = "Preheat disabled";
static const char pgmstr_drying_curing_temp[] PROGMEM = "Drying/Curing temp";
static const char pgmstr_resin_preheat_temp[] PROGMEM = "Resin preheat temp";
static const char pgmstr_sound_response[] PROGMEM = "Sound response";
static const char pgmstr_finish_beep[] PROGMEM = "Finish beep";
static const char pgmstr_ipa_tank_removed[] PROGMEM = "IPA tank removed";
static const char pgmstr_pause[] PROGMEM = "Pause";
static const char pgmstr_stop[] PROGMEM = "Stop";
static const char pgmstr_continue[] PROGMEM = "Continue";


Countimer tDown;
Countimer tUp;

CSelftest selftest;
CSpeedControl speed_control;

thermistor therm1(A4, 5);

Trinamic_TMC2130 myStepper(CS_PIN);

MCP outputchip(0, 8);


PrusaLcd lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7);

enum menu_state : uint8_t {
  MENU,
  SPEED_STATE,
  SPEED_CURING,
  SPEED_WASHING,
  TIME,
  TIME_CURING,
  TIME_DRYING,
  TIME_WASHING,
  TIME_RESIN_PREHEAT,
  SETTINGS,
  ADVANCED_SETTINGS,
  PREHEAT,
  PREHEAT_ENABLE,
  TARGET_TEMP,
  RESIN_TARGET_TEMP,
  UNIT_SYSTEM,
  RUN_MODE,
  SOUND_SETTINGS,
  FANS,
  LED_INTENSITY,
  FAN1_CURING,
  FAN1_DRYING,
  FAN2_CURING,
  FAN2_DRYING,
  SOUND,
  RUNNING,
  RUN_MENU,
  BEEP,
  INFO,
  CONFIRM,
  ERROR,
  SELFTEST
};

volatile uint16_t *const bootKeyPtr = (volatile uint16_t *)(RAMEND - 1);

menu_state state = MENU;

//long lastJob = 0;

uint8_t menu_position = 0;
uint8_t last_menu_position = 0;
byte max_menu_position = 0;
bool redraw_menu = true;
bool redraw_ms = true;
bool pinda_therm = 0; // 0 - 100K thermistor - ATC Semitec 104GT-2/ 1 - PINDA thermistor

unsigned long time_now = 0;
unsigned long therm_read_time_now = 0;

bool button_released = false;
volatile uint8_t rotary_diff = 128;

typedef struct
{
	char magic[6];
	byte washing_speed;
	byte curing_speed;
	byte washing_run_time;
	byte curing_run_time;
	byte drying_run_time;
	byte finish_beep_mode;
	byte sound_response;
	byte heat_to_target_temp;
	byte target_temp_celsius;
	byte target_temp_fahrenheit;
	byte curing_machine_mode;
	byte SI_unit_system;
	bool heater_failure;

} eeprom_small_t;

typedef struct
{
	char magic[6];
	byte washing_speed;
	byte curing_speed;
	byte washing_run_time;
	byte curing_run_time;
	byte drying_run_time;
	byte finish_beep_mode;
	byte sound_response;
	byte heat_to_target_temp;
	byte target_temp_celsius;
	byte target_temp_fahrenheit;
	byte curing_machine_mode;
	byte SI_unit_system;
	bool heater_failure;
	byte FAN1_CURING_SPEED;
	byte FAN1_DRYING_SPEED;
	byte FAN1_PREHEAT_SPEED;
	byte FAN2_CURING_SPEED;
	byte FAN2_DRYING_SPEED;
	byte FAN2_PREHEAT_SPEED;
	byte resin_preheat_run_time;
	byte resin_target_temp_celsius;

} eeprom_t;

static_assert(sizeof(eeprom_t) <= EEPROM_OFFSET, "eeprom_t doesn't fit in it's reserved space in the memory.");
static constexpr eeprom_t * eeprom_base = reinterpret_cast<eeprom_t*> (E2END + 1 - EEPROM_OFFSET);
static constexpr eeprom_small_t * eeprom_small_base = reinterpret_cast<eeprom_small_t*> (E2END + 1 - EEPROM_OFFSET);

eeprom_t config = {"CURW1", 10, 1, 4, 3, 3, 1, 1, 0, 35, 95, 0, 1, false, 60, 60, 40, 70, 70, 40, 3, 30};

byte max_preheat_run_time = 30;
byte cover_check_enabled = 1;
byte LED_PWM_VALUE = 100;

bool fan1_pwm_State = LOW;
bool fan2_pwm_State = LOW;

int fan_tacho_count[3];
int fan_tacho_last_count[3];

bool fan1_on = false;
bool fan2_on = false;

unsigned long fan1_previous_millis = 0;
unsigned long fan2_previous_millis = 0;

bool heater_error = false;
bool fan_error[2] = {false, false};

// constants won't change:
const uint8_t fan_frequency = 70; //Hz
const float period = (1 / (float)fan_frequency) * 1000; //
uint8_t fan_duty[2]; //%

//fan1_duty = 0-100%
byte FAN1_MENU_SPEED = 30;
byte FAN1_WASHING_SPEED = 60;//70

//fan2_duty = 0-100%
byte FAN2_MENU_SPEED = 30;
byte FAN2_WASHING_SPEED = 70;//70

long remain = 0;
unsigned long us_last = 0;
unsigned long last_remain = 0;
unsigned long last_millis = 0;
uint8_t last_seconds = 0;
unsigned long led_time_now = 0;
const int LED_delay = 1000;
bool heater_running = false;
bool curing_mode = false;
bool drying_mode = false;
bool last_curing_mode = false;
bool paused = false;
bool cover_open = false;
bool gastro_pan = false;
bool paused_time = false;
bool led_start = false;
uint8_t running_count = 0;

const long thermNom_1 = 100000;
const uint8_t refTemp_1 = 25;
const int beta_1 = 4267;
const long thermistor_pullup_1 = 100000;

const long thermNom_2 = 100000;
int refTemp_2 = 25;
int beta_2 = 4250;
const unsigned int thermistor_pullup_2 = 33000;

float chamber_temp_celsius;
float chamber_temp_fahrenheit;
float led_temp;
bool therm_read = false;

volatile int divider = 0;

int ms_counter;
int ams_fan_counter;

unsigned long button_timer = 0;
const int long_press_time = 1000;
bool button_active = false;
bool long_press_active = false;
bool long_press = false;
bool preheat_complete = false;
bool pid_mode = false;

double summErr = 0;
//double oldSpeed = 0;

const uint8_t P = 10;//0.5
const double I = 0.001; //0.001;
const uint8_t D = 1; //0.1;

byte Back[8] = {
  B00100,
  B01110,
  B11111,
  B00100,
  B11100,
  B00000,
  B00000,
  B00000
};

byte Right[8] = {
  B00000,
  B00100,
  B00010,
  B11111,
  B00010,
  B00100,
  B00000,
  B00000
};

static volatile uint16_t bootKeyPtrVal __attribute__ ((section (".noinit")));


const char magic[MAGIC_SIZE] = "CURWA";
const char magic2[MAGIC_SIZE] = "CURW1";


static void motor_configuration();
static void read_config();
static void fan_tacho1();
static void fan_tacho2();
static void fan_tacho3();
static void print_time1();
static void menu_move(bool sound_echo);
static void machine_running();
static void button_press();
static void tDownComplete();
static void start_drying();
static void stop_curing_drying();
static void start_curing();
static void start_washing();
static void tUpComplete();
static void fan_pwm_control();
static void fan_rpm();
static void preheat();
static void lcd_time_print(uint8_t dots_column);
static void therm1_read();

static inline bool is_error()
{
    return (fan_error[0] || fan_error[1] || config.heater_failure);
}

void setupTimer0() { //timmer for fan pwm
  noInterrupts();
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
  interrupts();
}

void setupTimer4() {
  noInterrupts();
  // Clear registers
  TCCR4A = 0;
  TCCR4B = 0;
  TCNT4 = 0;

  // 100.16025641025641 Hz (16000000/((155+1)*1024))
  OCR4A = 155;
  // CTC
  TCCR4A |= (1 << WGM41);
  // Prescaler 1024
  TCCR4B |= (1 << CS42) | (1 << CS41) | (1 << CS40);
  // Output Compare Match A Interrupt Enable
  TIMSK4 |= (1 << OCIE4A);
  interrupts();
}

void setupTimer3() { //timmer for stepper move
  noInterrupts();
  // Clear registers
  TCCR3A = 0;
  TCCR3B = 0;
  TCNT3 = 0;
  // 1 Hz (16000000/((15624+1)*1024))
  OCR3A = 200; // 15-50
  // CTC
  TCCR3B |= (1 << WGM32);
  // Prescaler 1024
  TCCR3B |= (1 << CS31) | (1 << CS30);
  // Output Compare Match A Interrupt Enable
  TIMSK3 |= (1 << OCIE3A);
  interrupts();
}

void enable_timer3() {

  // Output Compare Match A Interrupt Enable
  TIMSK3 |= (1 << OCIE3A);

}

void disable_timer3() {

  // Interrupt Disable
  TIMSK3 = 0;

}

void run_motor() {

  outputchip.digitalWrite(EN_PIN, LOW); // enable driver
  speed_control.motor_running = true;

}

void stop_motor() {

  outputchip.digitalWrite(EN_PIN, HIGH); // disable driver
  speed_control.motor_running = false;

}

void run_heater() {

  outputchip.digitalWrite(FAN_HEAT_PIN, HIGH); // enable driver
  heater_running = true;
  wdt_enable(WDTO_4S);

}

void stop_heater() {

  outputchip.digitalWrite(FAN_HEAT_PIN, LOW); // disable driver
  heater_running = false;
  wdt_disable();

}

void motor_configuration() {

  if (curing_mode == true) {
    myStepper.set_IHOLD_IRUN(10, 10, 0);
    myStepper.set_mres(256);
  } else {
    myStepper.set_IHOLD_IRUN(31, 31, 5);
    myStepper.set_mres(16);
  }
}

void setup() {

  outputchip.begin();
  outputchip.pinMode(0B0000000010010111);
  outputchip.pullupMode(0B0000000010000011);
  read_config();

  outputchip.digitalWrite(EN_PIN, HIGH); // disable driver


  lcd.begin(20, 4);

  // buttons
  pinMode(BTN_EN1, INPUT_PULLUP);
  pinMode(BTN_EN2, INPUT_PULLUP);

  pinMode(DIR_PIN, OUTPUT);
  pinMode(STEP_PIN, OUTPUT);

  pinMode(LCD_PWM_PIN, OUTPUT);
  digitalWrite(LCD_PWM_PIN, HIGH);
  pinMode(BEEPER, OUTPUT);

  pinMode(FAN1_PWM_PIN, OUTPUT);
  pinMode(FAN2_PWM_PIN, OUTPUT);

  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);

  attachInterrupt(2, fan_tacho1, RISING);
  attachInterrupt(1, fan_tacho2, RISING);
  attachInterrupt(3, fan_tacho3, RISING);

  fan_duty[0] = FAN1_MENU_SPEED;
  fan_duty[1] = FAN2_MENU_SPEED;

  outputchip.digitalWrite(LED_RELE_PIN, LOW); // turn off led
  pinMode(LED_PWM_PIN, OUTPUT);
  digitalWrite(LED_PWM_PIN, LOW);


  // stepper driver init
  myStepper.init();
  myStepper.set_mres(16); // ({1,2,4,8,16,32,64,128,256}) number of microsteps
  myStepper.set_IHOLD_IRUN(10, 10, 0); // ([0-31],[0-31],[0-5]) sets all currents to maximum
  myStepper.set_I_scale_analog(0); // ({0,1}) 0: I_REF internal, 1: sets I_REF to AIN
  myStepper.set_tbl(1); // ([0-3]) set comparator blank time to 16, 24, 36 or 54 clocks, 1 or 2 is recommended
  myStepper.set_toff(8); // ([0-15]) 0: driver disable, 1: use only with TBL>2, 2-15: off time setting during slow decay phase
  myStepper.set_en_pwm_mode(1);// 0: driver disable PWM mode, 1: driver enable PWM mode
  // get ready

  setupTimer3();
  //setupTimer1();
  setupTimer0();
  tDown.setInterval(print_time1, 1000);
  tUp.setInterval(print_time1, 1000);
  //pinMode(FAN_HEAT_PIN, OUTPUT);
  stop_heater();

  lcd.createChar(0, Back);
  lcd.createChar(1, Right);
  redraw_menu = true;
  menu_move(true);
}

void write_config() {

	config.washing_speed = speed_control.washing_speed;
	config.curing_speed = speed_control.curing_speed;

	EEPROM.put(*eeprom_base, config);
}
/*! \brief This function loads user-defined values from eeprom.
 *
 *
 *  It loads different amount of variables, depending on the magic variable from eeprom.
 *  If magic is not set in the eeprom, variables keep their default values.
 *  If magic from eeprom is equal to magic, it loads only variables customizable in older firmware and keeps new variables default.
 *  If magic from eeprom is equal to magic2, it loads all variables including those added in new firmware.
 *  It won't load undefined (new) variables after flashing new firmware.
 */
void read_config() {
  char test_magic[MAGIC_SIZE];
  EEPROM.get(eeprom_base->magic, test_magic);
  if (!strncmp(magic, test_magic, MAGIC_SIZE)) {

	EEPROM.get(*eeprom_small_base, *(reinterpret_cast<eeprom_small_t*>(&config)));
	speed_control.washing_speed = config.washing_speed;
	speed_control.curing_speed = config.curing_speed;

  } else if(!strncmp(magic2, test_magic, MAGIC_SIZE)){

	  EEPROM.get(*eeprom_base, config);
	  speed_control.washing_speed = config.washing_speed;
	  speed_control.curing_speed = config.curing_speed;

  }
}

int PID(float & actualTemp, byte targetTemp) {

  double newSpeed = 0, errValue, diffTemp;

  errValue = actualTemp - targetTemp;

  summErr = errValue + summErr;

  if ((summErr > 10000) || (summErr < -10000))
    summErr = 10000;

  diffTemp = actualTemp;	// "- oldSpeed;" which was always 0

  newSpeed = P * errValue + I * summErr + D * diffTemp;
  if (newSpeed > 100) {
    return 100;
  }
  return newSpeed;
}

void print_menu_cursor(uint8_t line)
{
  lcd.setCursor(0, line);
  lcd.print(">");

  for (int i = 0; i <= 3; i++) {
    if ( i != line) {
      lcd.setCursor(0, i);
      lcd.print(" ");
    }
  }
}

void generic_menu_P(byte num, ...) {
  va_list argList;
  va_start(argList, num);
  max_menu_position = 0;
  for (; num; num--) {
    lcd.setCursor(1, max_menu_position++);
    lcd.printClear_P(va_arg(argList, const char *), 19, Ter::none);
  }
  va_end(argList);
  max_menu_position--;

  if (rotary_diff > 128) {
    if (menu_position < max_menu_position) {
      menu_position++;
    }
  } else if (rotary_diff < 128) {
    if (menu_position) {
      menu_position--;
    }
  }
  print_menu_cursor(menu_position);
}

void lcd_print_back() {
  lcd.setCursor(19, 0);
  lcd.print(" ");
  lcd.setCursor(19, 0);
  lcd.write(byte(0));
}

void lcd_print_right(int a) {
  lcd.setCursor(19, a);
  lcd.print(" ");
  lcd.setCursor(19, a);
  lcd.write(byte(1));
}

void generic_value(const char *label, byte *value, byte min, byte max, const char *units, bool conversion) {
  lcd.setCursor(1, 0);
  lcd.print(label);
  if (!conversion) {
    if (rotary_diff > 128) {
      if (*value < max) {
        (*value)++;
      }
    } else if (rotary_diff < 128) {
      if (*value > min) {
        (*value)--;
      }
    }

    if (*value < 10) {
      lcd.setCursor(7, 2);
      lcd.print(" ");
      lcd.setCursor(8, 2);
      lcd.print(*value);
      lcd.print(units);
    } else {
      lcd.setCursor(7, 2);
      lcd.print(*value);
      lcd.print(units);
    }
  }
  else {
    if (rotary_diff > 128) {
      if (*value < max) {
        (*value) += 1.8;
      }
    } else if (rotary_diff < 128) {
      if (*value > min) {
        (*value) -= 1.8;
      }
    }
    if ((((*value) * 1.8) + 32) < 10) {
      lcd.setCursor(7, 2);
      lcd.print(" ");
      lcd.setCursor(8, 2);
      lcd.print(((*value) * 1.8) + 32);
      lcd.print(units);
    } else {
      lcd.setCursor(7, 2);
      lcd.print(((*value) * 1.8) + 32);
      lcd.print(units);
    }
  }
}

void generic_items(const char *label, byte *value, byte num, ...) {
  lcd.setCursor(1, 0);
  lcd.print(label);
  const char *items[num];
  if (*value > num) {
    *value = 0;
  }

  va_list argList;
  va_start(argList, num);
  byte i = 0;
  for (; num; num--) {
    items[i++] = va_arg(argList, const char *);
  }
  va_end(argList);

  if (rotary_diff > 128) {
    if (*value < i - 1) {
      (*value)++;
    }
  } else if (rotary_diff < 128) {
    if (*value) {
      (*value)--;
    }
  }

  if (*value < i) {
    byte len = strlen(items[*value]);
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor((20 - len) / 2, 2);
    lcd.print(items[*value]);
  }
}

void echo(void) {
  for (byte i = 0; i < 10; ++i) {
    digitalWrite(BEEPER, HIGH);
    delayMicroseconds(100);
    digitalWrite(BEEPER, LOW);
    delayMicroseconds(100);
  }
}

void beep(void) {
  analogWrite(BEEPER, 220);
  delay(50);
  digitalWrite(BEEPER, LOW);
  delay(250);
  analogWrite(BEEPER, 220);
  delay(50);
  digitalWrite(BEEPER, LOW);
}

void warning_beep(void) {
  analogWrite(BEEPER, 220);
  delay(50);
  digitalWrite(BEEPER, LOW);
  delay(250);
}

void lcd_blink(void) {
  analogWrite(LCD_PWM_PIN, 200);
  delay(100);
  analogWrite(LCD_PWM_PIN, 255);
  delay(250);
  analogWrite(LCD_PWM_PIN, 200);
  delay(100);
  analogWrite(LCD_PWM_PIN, 255);
}

void loop() {
  if ( *bootKeyPtr != MAGIC_KEY)
  {
    wdt_reset();
  }
  tDown.run();
  tUp.run();

  if(state == SELFTEST){
  	  switch(selftest.phase){
  	  case 1:
  		    selftest.measure_state(outputchip.digitalRead(COVER_OPEN_PIN) == HIGH);
  		    selftest.universal_pin_test();
  	  		break;

  	  case 2:
  		  	selftest.measure_state(outputchip.digitalRead(WASH_DETECT_PIN) == HIGH);
  		    selftest.universal_pin_test();
  		  	break;
  	  case 3:
  		    selftest.ventilation_test(fan_error[0], fan_error[1]);
  		  	fan_duty[0] = selftest.fan1_speed;
  		    fan_duty[1] = selftest.fan2_speed;
  		    break;
  	  case 4:
  		    if(selftest.is_first_loop()){
  		    	outputchip.digitalWrite(LED_RELE_PIN, HIGH); // turn LED on
  		    	int val;
  		    	val = map(LED_PWM_VALUE, 0, 100, 0, 255);
  		    	analogWrite(LED_PWM_PIN, val);
  		    }
  		    if(selftest.led_test == false){
  		    	selftest.LED_test();
  		    } else {
		        outputchip.digitalWrite(LED_RELE_PIN, LOW); // turn LED off
		        digitalWrite(LED_PWM_PIN, LOW);
  		    }
  		    break;

  	  case 5:
  		if(!selftest.heater_test){
  		  	if(selftest.is_first_loop()){
  		  		pid_mode = true;
  		  		run_heater();
  		  		fan_duty[0] = FAN1_MENU_SPEED;
  		  	    fan_duty[1] = FAN2_MENU_SPEED;
  		  	}
  		  	selftest.fan1_speed = outputchip.digitalRead(WASH_DETECT_PIN) == HIGH;	//flag for Wash_tank_pin
  		  	selftest.heat_test(heater_error);
  		} else if (heater_running){
  	 		stop_heater();
  	  		fan_duty[0] = FAN1_MENU_SPEED;
  	  		fan_duty[1] = FAN2_MENU_SPEED;
  	  		pid_mode = false;
  		}
  		    break;
  	  case 6:
  		  if(!selftest.rotation_test && selftest.motor_rotation_timer()){
  		    static bool mode_flag = true;
  		    if(selftest.is_first_loop()){
  		    	if(mode_flag){
  		    		speed_control.curing_speed = 1;
  		    		myStepper.set_IHOLD_IRUN(10, 10, 0);		//motor_configuration for curing mode;
  		    		myStepper.set_mres(256);
  		    	} else {
  		    		speed_control.washing_speed = 1;
  		    		myStepper.set_IHOLD_IRUN(31, 31, 5);			//motor_configuration for washing mode
  		    		myStepper.set_mres(16);
  		    	}
  		    	speed_control.speed_configuration(mode_flag);
  		    	run_motor();
  		    	selftest.set_first_loop(false);
  		    } else {
  		    	if(speed_control.curing_speed <= 10 && speed_control.washing_speed <= 10){
  		    		if(!mode_flag){
  		    			byte backup = speed_control.microstep_control;		//needed for smooth gear-up of the motor
  		    			speed_control.speed_configuration(mode_flag);
  		    			speed_control.microstep_control = backup;
  		    		} else
  		    			speed_control.speed_configuration(mode_flag);
  		    	}
  		    }
  		    lcd.setCursor(1, 1);
  		    lcd.print("Mode/Gear: ");
  		    lcd.print(mode_flag);
  		    lcd.print("/");

  		    if(mode_flag){
  		    	if(speed_control.curing_speed <= 10)
  		    		lcd.print(speed_control.curing_speed);
  		    	speed_control.curing_speed++;
  		    } else {
  		    	if(speed_control.washing_speed <= 10)
  		    		lcd.print(speed_control.washing_speed);
  		    	speed_control.washing_speed++;
  		    }
  		    if(mode_flag && speed_control.curing_speed > 11){
  		    	stop_motor();
  		    	selftest.clean_up();
  		    	speed_control.curing_speed = 1;		//default val
  		    	mode_flag = false;
  		    }
  		    if (!mode_flag && speed_control.washing_speed > 11){
  		    	stop_motor();
  		    	speed_control.washing_speed = 10;	//default val
  		    	selftest.rotation_test = true;
  		    }
  		  }
  		  break;

  	  default:
  		  break;
  	  }
  }

  if (heater_error) {
    if (config.heat_to_target_temp) {
      tDown.stop();
    }
    else {
      tUp.stop();
    }
    stop_heater(); // turn off heater and fan
    stop_motor(); // turn off motor
    fan_duty[0] = FAN1_MENU_SPEED;
    fan_duty[1] = FAN2_MENU_SPEED;
  }

  if (state == MENU) {
    if (pinda_therm == 1) {
      if (outputchip.digitalRead(WASH_DETECT_PIN) == LOW) { //CHECK WASH_DETECT_PIN
        curing_mode = true;
      } else {
        curing_mode = false;
      }
    }
    else {
      if (outputchip.digitalRead(WASH_DETECT_PIN) == HIGH) { //CHECK WASH_DETECT_PIN
        curing_mode = true;
      } else {
        curing_mode = false;
      }
    }
  }

  if (heater_error) {
    lcd.setCursor(1, 0);
    lcd.print("Heater fan error...");
    lcd.setCursor(1, 2);
    lcd.print("Please restart");
    state = ERROR;

  }

  if (state == CONFIRM) {
    unsigned long us_now = millis();
    if (us_now - us_last > 1000) {
      beep();
      us_last = us_now;
    }
  }

  if (last_curing_mode != curing_mode) {
    last_curing_mode = curing_mode;
    redraw_menu = true;
  }

  if (speed_control.acceleration_flag == true) { //stepper motor speed up function
    unsigned long us_now = millis();
    if (us_now - us_last > 50){
    	speed_control.acceleration50ms();
    	us_last = us_now;
    }
    if (speed_control.acceleration_flag == false){
    	myStepper.set_IHOLD_IRUN(10, 10, 5);
    }
  }

  // rotary "click" is 4 "micro steps"
  if (rotary_diff <= 124 || rotary_diff >= 132 || redraw_menu) { //124, 132
    menu_move(true);
  }

  if (state == RUNNING || state == RUN_MENU) {
    machine_running();
  }

  if (outputchip.digitalRead(BTN_ENC) == LOW) {
    if (button_active == false) {
      button_active = true;
      button_timer = millis();
    }
    if ((millis() - button_timer > long_press_time) && (long_press_active == false)) {
      long_press_active = true;
      if (state == MENU || state == SETTINGS || state == ADVANCED_SETTINGS || state == PREHEAT || state == SOUND_SETTINGS || state == TIME || state == SPEED_STATE) {
        state = RUN_MODE;
        long_press = true;
        redraw_menu = true;
        menu_move(true);
 //       if (config.sound_response) {
 //			echo();
 //       }
      }
    }
  } else {
    if (button_active == true) {
      if (long_press_active == true) {
        long_press_active = false;
      } else {
        if (!heater_error) {
          button_press();
        }
      }
      button_active = false;
    }
  }

  if (millis() > time_now + 5500) {
    if (state == MENU || state == ADVANCED_SETTINGS || state == PREHEAT || state == SOUND_SETTINGS || state == SPEED_STATE) {
      last_menu_position = menu_position;
    }
//    if (state == SETTINGS || state == FANS || state == TIME) {
//    }

    time_now = millis();
    lcd.reinit();
    lcd.createChar(0, Back);
    lcd.createChar(1, Right);
    redraw_menu = true;
    menu_move(false);

    if (state == MENU || state == ADVANCED_SETTINGS || state == PREHEAT || state == SOUND_SETTINGS || state == SPEED_STATE) {
      menu_position = last_menu_position;
      print_menu_cursor(menu_position);
    }
//    if (state == SETTINGS || state == FANS || state == TIME) {
//    }
  }

  if (millis() > therm_read_time_now + 2000) {
    therm_read_time_now = millis();
    therm1_read();
  }
  if (millis() > therm_read_time_now + 2000) {
    therm_read_time_now = millis();
    therm1_read();
  }
}

void menu_move(bool sound_echo) {
  if (!redraw_menu) {
    if (sound_echo) {
      if (config.sound_response) {
        echo();
      }
    }
  }
  else {
    lcd.clear();
  }

  redraw_menu = false;

  switch (state) {
    case MENU:

      switch (config.curing_machine_mode) {
        case 3:
          generic_menu_P(3, curing_mode ? pgmstr_start_resin_preheat : pgmstr_start_washing,
                  pgmstr_run_time, is_error() ? pgmstr_settings_error : pgmstr_settings);
          lcd_print_right(1);
          lcd_print_right(2);

            state = MENU;
          break;
        case 2:
          generic_menu_P(3, curing_mode ? pgmstr_start_drying : pgmstr_start_washing, pgmstr_run_time,
                  is_error() ? pgmstr_settings_error : pgmstr_settings);
          lcd_print_right(1);
          lcd_print_right(2);

          state = MENU;
          break;
        case 1:
        	generic_menu_P(3, curing_mode ? pgmstr_start_curing : pgmstr_start_washing, pgmstr_run_time,
        	             is_error() ? pgmstr_settings_error : pgmstr_settings);
        	lcd_print_right(1);
        	lcd_print_right(2);

          state = MENU;
          break;
        case 0:
        default:
        	generic_menu_P(4, curing_mode ? pgmstr_start_drying_curing : pgmstr_start_washing, pgmstr_run_time,
        	             is_error() ? pgmstr_settings_error : pgmstr_settings, pgmstr_selftest);
          lcd_print_right(1);
          lcd_print_right(2);
          lcd_print_right(3);

          state = MENU;
          break;
      }

      break;

    case SPEED_STATE:

      generic_menu_P(3, pgmstr_back, pgmstr_curing_speed, pgmstr_washing_speed);
      lcd_print_back();
      lcd_print_right(1);
      lcd_print_right(2);

      break;

    case SPEED_CURING:

      generic_value("Curing speed", &speed_control.curing_speed, 1, 10, "/10", 0);

      break;

    case SPEED_WASHING:

      generic_value("Washing speed", &speed_control.washing_speed, 1, 10, "/10", 0);

      break;

    case TIME:
      {
        Scrolling_item items[] =
        {
          {pgmstr_back, true, Ter::back},
          {pgmstr_curing, true, Ter::right},
          {pgmstr_drying, true, Ter::right},
          {pgmstr_washing, true, Ter::right},
          {pgmstr_resin_preheat, true, Ter::right},
        };
        menu_position = scrolling_list_P(items);

        break;
      }
    case TIME_CURING:

      generic_value("Curing run-time", &config.curing_run_time, 1, 10, " min", 0);

      break;

    case TIME_DRYING:

      generic_value("Drying run-time", &config.drying_run_time, 1, 10, " min", 0);

      break;

    case TIME_WASHING:

      generic_value("Washing run-time", &config.washing_run_time, 1, 10, " min", 0);

      break;

    case TIME_RESIN_PREHEAT:

      generic_value("Resin preheat time", &config.resin_preheat_run_time, 1, 10, " min", 0);

      break;

    case SETTINGS:
    {
      Scrolling_item items[] =
      {
        {pgmstr_back, true, Ter::back},
        {pgmstr_rotation_speed, true, Ter::right},
        {pgmstr_run_mode, true, Ter::right},
        {pgmstr_preheat, true, Ter::right},
        {pgmstr_sound, true, Ter::right},
        {pgmstr_fans, true, Ter::right},
        {pgmstr_led_intensity, true, Ter::right},
        {is_error() ? pgmstr_information_error : pgmstr_information, true, Ter::right},
        {pgmstr_unit_system, true, Ter::right},
      };
      menu_position = scrolling_list_P(items);
      break;
    }
    case ADVANCED_SETTINGS:
      generic_menu_P(4, pgmstr_back, pgmstr_run_mode, pgmstr_preheat, pgmstr_unit_system);
      lcd_print_back();
      lcd_print_right(1);
      lcd_print_right(2);
      lcd_print_right(3);
      break;

    case PREHEAT:
      if (config.heat_to_target_temp) {
        generic_menu_P(4, pgmstr_back, pgmstr_preheat_enabled, pgmstr_drying_curing_temp, pgmstr_resin_preheat_temp );
      }
      else {
        generic_menu_P(4, pgmstr_back, pgmstr_preheat_disabled, pgmstr_drying_curing_temp, pgmstr_resin_preheat_temp );
      }
      lcd_print_back();
      lcd_print_right(1);
      lcd_print_right(2);
      lcd_print_right(3);
      break;

    case PREHEAT_ENABLE:

      generic_items("Preheat", &config.heat_to_target_temp, 2, "disable >", "< enable");

      break;

    case TARGET_TEMP:
      if (config.SI_unit_system) {
        generic_value("Target temp", &config.target_temp_celsius, 20, 40, " \xDF" "C ", 0);
      }
      else {
        generic_value("Target temp", &config.target_temp_celsius, 20, 40, " \xDF" "F ", 1);
      }
      break;

    case RESIN_TARGET_TEMP:
      if (config.SI_unit_system) {
        generic_value("Target temp", &config.resin_target_temp_celsius, 20, 40, "\xDF" "C", 0);
      }
      else {
        generic_value("Target temp", &config.resin_target_temp_celsius, 20, 40, "\xDF" "F", 1);
      }
      break;

    case UNIT_SYSTEM:
      generic_items("Unit system", &config.SI_unit_system, 2, "IMPERIAL/ US >", "< SI");
      break;

    case RUN_MODE:
      generic_items("Run mode", &config.curing_machine_mode, 4, "Drying & Curing >", "< Curing >", "< Drying >", "< Resin preheat");
      break;

    case SOUND_SETTINGS:
      generic_menu_P(3, pgmstr_back, pgmstr_sound_response, pgmstr_finish_beep );
      lcd_print_back();
      lcd_print_right(1);
      lcd_print_right(2);

      break;

    case SOUND:
      generic_items("Sound response", &config.sound_response, 2, "no >", "< yes");
      break;

    case BEEP:
      generic_items("Finish beep", &config.finish_beep_mode, 3, "none >", "< once > ", "< continuous");
      break;

    case FANS:
      {
        Scrolling_item items[] =
        {
          {pgmstr_back, true, Ter::back},
          {pgmstr_fan1_curing, true, Ter::right},
          {pgmstr_fan1_drying, true, Ter::right},
          {pgmstr_fan2_curing, true, Ter::right},
          {pgmstr_fan2_drying, true, Ter::right},
        };
        menu_position = scrolling_list_P(items);

        break;
      }

    case LED_INTENSITY:

      generic_value("LED intensity", &LED_PWM_VALUE, 1, 100, "% ", 0);

      break;

    case FAN1_CURING:

      generic_value("FAN1 curing speed", &config.FAN1_CURING_SPEED, 0, 100, " %", 0);

      break;

    case FAN1_DRYING:

      generic_value("FAN1 drying speed", &config.FAN1_DRYING_SPEED, 0, 100, " %", 0);

      break;

    case FAN2_CURING:

      generic_value("FAN2 curing speed", &config.FAN2_CURING_SPEED, 0, 100, " %", 0);

      break;

    case FAN2_DRYING:

      generic_value("FAN2 drying speed", &config.FAN2_DRYING_SPEED, 0, 100, " %", 0);

      break;

    case INFO:
      {
        Scrolling_item items[] =
        {
          {pgmstr_fw_version, true, Ter::none},
          {pgmstr_fan1_failure, fan_error[0], Ter::none},
          {pgmstr_fan2_failure, fan_error[1], Ter::none},
          {pgmstr_heater_failure, config.heater_failure, Ter::none},
          {pgmstr_serial_number, true, Ter::serialNumber},
          {pgmstr_build_nr, true, Ter::none},
          {pgmstr_fw_hash, true, Ter::none},
          {pgmstr_workspace, true, Ter::none}
        };
        menu_position = scrolling_list_P(items);

        break;
      }
    case RUN_MENU:
      if (!curing_mode && paused_time) {
        generic_menu_P(3, paused ? pgmstr_ipa_tank_removed : pgmstr_pause, pgmstr_stop, pgmstr_back);
      }
      else {
        generic_menu_P(3, paused ? pgmstr_continue : pgmstr_pause, pgmstr_stop, pgmstr_back);
      }
      break;

    case RUNNING:
      lcd.setCursor(1, 0);
      if (curing_mode) {
        if (paused) {
          if (config.heat_to_target_temp || (config.curing_machine_mode == 3) || (preheat_complete == false)) {
            lcd.print(paused ? "Paused...          " : drying_mode ? "Heating" : "Curing");
          }
          else {
            lcd.print(paused ? "Paused...          " : drying_mode ? "Drying" : "Curing");
          }
        }
        else {
          if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
            if (!preheat_complete) {
              lcd.print(cover_open ? "The cover is open!" : drying_mode ? "Heating" : "Curing");
            }
            else {
              lcd.print(cover_open ? "The cover is open!" : drying_mode ? "Drying" : "Curing");
            }
          }
          else {
            lcd.print(cover_open ? "The cover is open!" : drying_mode ? "Drying" : "Curing");
          }
        }
      }
      else {
        lcd.print(cover_open ? "Open cover!" : (paused ? "Paused...          " : "Washing"));
      }
      if (curing_mode && drying_mode && config.heat_to_target_temp && !preheat_complete) {
        lcd.setCursor(14, 2);
        lcd.print(" ");
        lcd.setCursor(5, 2);
        lcd.print(" ");
      }
      else {
        if (rotary_diff > 128) {
          if (tDown.getCurrentMinutes() <= 9) {
            byte mins = tDown.getCurrentMinutes();
            byte secs = tDown.getCurrentSeconds();
            lcd.setCursor(14, 2);
            lcd.print(">>");
            lcd.setCursor(14, 2);
            if (paused || cover_open) {
              delay(100);
              lcd.setCursor(14, 2);
              lcd.print("  ");
              lcd.setCursor(5, 2);
              lcd.print("  ");
            }

            if (secs <= 30) {
              tDown.setCounter(0, mins, secs + 30, tDown.COUNT_DOWN, tDownComplete);
              running_count = 0;
            }
            else {
              tDown.setCounter(0, mins + 1, 30 - (60 - secs), tDown.COUNT_DOWN, tDownComplete);
              running_count = 0;
            }
          }
          else {
            lcd.setCursor(14, 2);
            lcd.print("X");
            running_count = 0;
            lcd.setCursor(14, 2);
            if (paused || cover_open) {
              delay(100);
              lcd.setCursor(14, 2);
              lcd.print("  ");
              lcd.setCursor(5, 2);
              lcd.print("  ");
            }
          }
        } else if (rotary_diff < 128) {
          if (tDown.getCurrentSeconds() >= 30 || tDown.getCurrentMinutes() >= 1) {
            byte mins = tDown.getCurrentMinutes();
            byte secs = tDown.getCurrentSeconds();
            lcd.setCursor(5, 2);
            lcd.print("<<");
            lcd.setCursor(5, 2);
            if (paused || cover_open) {
              delay(100);
              lcd.setCursor(14, 2);
              lcd.print("  ");
              lcd.setCursor(5, 2);
              lcd.print("  ");
            }

            if (secs >= 30) {
              tDown.setCounter(0, mins, secs - 30, tDown.COUNT_DOWN, tDownComplete);
              running_count = 0;
            }
            else {
              tDown.setCounter(0, mins - 1, 60 - (30 - secs), tDown.COUNT_DOWN, tDownComplete);
              running_count = 0;
            }
          }
          else {
            lcd.setCursor(5, 2);
            lcd.print("X");
            running_count = 0;
            lcd.setCursor(5, 2);
            if (paused || cover_open) {
              delay(100);
              lcd.setCursor(14, 2);
              lcd.print("  ");
              lcd.setCursor(5, 2);
              lcd.print("  ");
            }
          }
        }
      }
      redraw_ms = true; // for print MM:SS part
      break;

    case CONFIRM:
      lcd.setCursor(1, 0);
      lcd.print("Finished...");
      lcd.setCursor(1, 2);
      lcd.print("Press to continue");
      break;

    case SELFTEST:
        if(selftest.phase == 0){
        generic_menu_P(2, pgmstr_back, pgmstr_continue);
        lcd_print_back();
        lcd_print_right(1);
        } else {
        	  lcd.setCursor(1, 0);
        	  lcd.print(selftest.print());
        }
        break;

    default:
      break;
  }
  rotary_diff = 128;
}

void machine_running() {

  if (curing_mode) { //curing mode
    if (outputchip.digitalRead(COVER_OPEN_PIN) == HIGH) { //cover check

      if (!cover_open) {
        if (!paused) {
          lcd.setCursor(1, 0);
          lcd.print("The cover is open! ");
        }
        running_count = 0;
        redraw_menu = true;
        cover_open = true;
      }
    }
    else {
      if (cover_open) {
        redraw_menu = true;
        cover_open = false;
      }
    }
    if (cover_open == true) {
      stop_motor();
      motor_configuration();
      speed_control.speed_configuration(curing_mode);
      stop_heater(); // turn off heat fan
      outputchip.digitalWrite(LED_RELE_PIN, LOW); // turn off led
      digitalWrite(LED_PWM_PIN, LOW);
    }

    else if (!paused) { // cover closed
      run_motor();
      unsigned long us_now = millis();
      remain -= us_now - us_last ;
      us_last = us_now;
    }
    switch (config.curing_machine_mode) {
      case 3: // Resin preheat
        if (!preheat_complete) {
          if (tUp.isCounterCompleted() == false) {
            if (!drying_mode) {
              drying_mode = true;
              redraw_menu = true;
            }
            start_drying();
          }
          else {
            if (drying_mode) {
              //drying_mode = false;
              redraw_menu = true;
              preheat_complete = true;
              remain = config.resin_preheat_run_time;
              tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
              tDown.start();
            }
          }
        }
        else {
          if (tDown.isCounterCompleted() == false) {
            if (!drying_mode) {
              drying_mode = true;
              redraw_menu = true;
            }
            start_drying();
          }
          else {
            if (drying_mode) {
              drying_mode = false;
              redraw_menu = true;
            }
            preheat_complete = false;
            stop_curing_drying ();
          }
        }

        break;
      case 2: // Drying
        if (!config.heat_to_target_temp) {
          if (tDown.isCounterCompleted() == false) {
            if (!drying_mode) {
              drying_mode = true;
              redraw_menu = true;
            }
            start_drying();
          }
          else {
            if (drying_mode) {
              drying_mode = false;
              redraw_menu = true;
            }
            stop_curing_drying ();
          }
        }
        else {
          if (!preheat_complete) {
            if (tUp.isCounterCompleted() == false) {
              if (!drying_mode) {
                drying_mode = true;
                redraw_menu = true;
              }
              start_drying();
            }
            else {
              if (drying_mode) {
                //drying_mode = false;
                redraw_menu = true;
                preheat_complete = true;
                remain = config.drying_run_time;
                tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                tDown.start();
              }
            }
          }
          else {
            if (tDown.isCounterCompleted() == false) {
              if (!drying_mode) {
                drying_mode = true;
                redraw_menu = true;
              }
              start_drying();
            }
            else {
              if (drying_mode) {
                drying_mode = false;
                redraw_menu = true;
              }
              preheat_complete = false;
              stop_curing_drying ();
            }
          }
        }
        break;
      case 1: // Curing
        if (tDown.isCounterCompleted() == false) {
          if (drying_mode) {
            drying_mode = false;
            redraw_menu = true;
          }
          start_curing();
        }
        else {
          stop_curing_drying ();
        }
        break;
      case 0: // Drying and curing
      default:
        if (!config.heat_to_target_temp) {
          if ((drying_mode == true) && (tDown.isCounterCompleted() == false)) {
            start_drying();
          }
          else {
            if (drying_mode) {
              drying_mode = false;
              remain = config.curing_run_time;
              running_count = 0;
              tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
              tDown.start();
              redraw_menu = true;
              menu_move(true);
            }
            if (tDown.isCounterCompleted() == false) {
              start_curing();
              fan_duty[0] = config.FAN1_CURING_SPEED;
              fan_duty[1] = config.FAN2_CURING_SPEED;
            }
            else {
              stop_curing_drying ();
            }
          }
        }
        else {
          if (!preheat_complete) {
            if (tUp.isCounterCompleted() == false) {
              if (!drying_mode) {
                drying_mode = true;
                redraw_menu = true;
              }
              start_drying();
            }
            else {
              if (drying_mode) {
                //drying_mode = false;
                redraw_menu = true;
                preheat_complete = true;
                remain = config.drying_run_time;
                tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                tDown.start();
              }
            }
          }
          else {
            if ((drying_mode == true) && (tDown.isCounterCompleted() == false)) {

              start_drying();
            }
            else {
              if (drying_mode) {
                drying_mode = false;
                remain = config.curing_run_time;
                running_count = 0;
                tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                tDown.start();
                redraw_menu = true;
                menu_move(true);
              }
              if (tDown.isCounterCompleted() == false) {
                start_curing();
              }
              else {
                stop_curing_drying ();
              }
            }
          }
        }

        break;
    }
  }
  if (!curing_mode) { // washing mode
    start_washing();
  }
}

void button_press() {
  if (config.sound_response) {
    echo();
  }
  switch (state) {
    case MENU:
      switch (menu_position) {
        case 0:

          if (curing_mode) { // curing_mode
            motor_configuration();
            speed_control.speed_configuration(curing_mode);
            running_count = 0;

            switch (config.curing_machine_mode) {
              case 3: // Resin preheat
                pid_mode = true;
                remain = max_preheat_run_time;
                tUp.setCounter(0, remain, 0, tUp.COUNT_UP, tUpComplete);
                tUp.start();
                fan_duty[0] = config.FAN1_PREHEAT_SPEED;
                fan_duty[1] = config.FAN2_PREHEAT_SPEED;
                outputchip.digitalWrite(LED_RELE_PIN, LOW); //turn off LED
                digitalWrite(LED_PWM_PIN, LOW);
                drying_mode = true;
                preheat_complete = false;
                break;
              case 2: // Drying
                preheat_complete = false;
                drying_mode = true;
                if (!config.heat_to_target_temp) {
                  pid_mode = false;
                  remain = config.drying_run_time;
                  tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                  tDown.start();
                  fan_duty[0] = config.FAN1_DRYING_SPEED;
                  fan_duty[1] = config.FAN2_DRYING_SPEED;
                }
                else {
                  pid_mode = true;
                  remain = max_preheat_run_time;
                  tUp.setCounter(0, remain, 0, tUp.COUNT_UP, tUpComplete);
                  tUp.start();
                  fan_duty[0] = config.FAN1_PREHEAT_SPEED;
                  fan_duty[1] = config.FAN2_PREHEAT_SPEED;
                }

                outputchip.digitalWrite(LED_RELE_PIN, LOW); //turn off LED
                digitalWrite(LED_PWM_PIN, LOW);
                drying_mode = true;
                break;
              case 1: // Curing
                pid_mode = false;
                remain = config.curing_run_time;
                tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                tDown.start();

                fan_duty[0] = config.FAN1_CURING_SPEED;
                fan_duty[1] = config.FAN2_CURING_SPEED;
                drying_mode = false;

                break;
              case 0: // Drying and curing
              default:
                tDown.stop();
                tUp.stop();
                preheat_complete = false;
                drying_mode = true;
                if (!config.heat_to_target_temp) {
                  pid_mode = false;
                  remain = config.drying_run_time;
                  tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                  tDown.start();
                  fan_duty[0] = config.FAN1_DRYING_SPEED;
                  fan_duty[1] = config.FAN2_DRYING_SPEED;
                }
                else {
                  pid_mode = true;
                  remain = max_preheat_run_time;
                  tUp.setCounter(0, remain, 0, tUp.COUNT_UP, tUpComplete);
                  tUp.start();
                  fan_duty[0] = config.FAN1_PREHEAT_SPEED;
                  fan_duty[1] = config.FAN2_PREHEAT_SPEED;
                }

                break;
            }
          } else { // washing_mode
            drying_mode = false;
            run_motor();
            motor_configuration();
            speed_control.speed_configuration(curing_mode);
            running_count = 0;
            remain = config.washing_run_time;
            tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
            tDown.start();
            fan_duty[0] = FAN1_WASHING_SPEED;
            fan_duty[1] = FAN2_WASHING_SPEED;
          }

          us_last = millis();

          menu_position = 0;
          state = RUNNING;
          redraw_menu = true;
          menu_move(true);
          break;

        case 1:
          menu_position = 0;
          state = TIME;
          break;

        case 2:
          menu_position = 0;
          state = SETTINGS;
          break;

        case 3:
          menu_position = 0;
          state = SELFTEST;
          break;

        default:
          break;
      }
      break;

    case SETTINGS:
      switch (menu_position) {
        case 0:
          menu_position = 2;
          state = MENU;
          break;

        case 1:
          menu_position = 0;
          state = SPEED_STATE;
          break;

        case 2:
          menu_position = 0;
          state = RUN_MODE;
          break;

        case 3:
          menu_position = 0;
          state = PREHEAT;
          break;

        case 4:
          menu_position = 0;
          state = SOUND_SETTINGS;
          break;

        case 5:
          menu_position = 0;
          state = FANS;
          break;

        case 6:
          menu_position = 0;
          state = LED_INTENSITY;
          break;

        case 7:
          menu_position = 0;
          state = INFO;
          break;

        default:
          menu_position = 0;
          state = UNIT_SYSTEM;
          break;
      }
      break;

    case SOUND_SETTINGS:
      switch (menu_position) {
        case 0:
          menu_position = 4;
          state = SETTINGS;
          break;

        case 1:
          menu_position = 0;
          state = SOUND;
          break;

        case 2:
          menu_position = 0;
          state = BEEP;
          break;
      }
      break;

    case FANS:
      switch (menu_position) {
        case 0:
          menu_position = 5;
          state = SETTINGS;
          break;

        case 1:
          menu_position = 0;
          state = FAN1_CURING;
          break;

        case 2:
          menu_position = 0;
          state = FAN1_DRYING;
          break;

        case 3:
          menu_position = 0;
          state = FAN2_CURING;
          break;

        default:
          menu_position = 0;
          state = FAN2_DRYING;
          break;
      }
      break;

    case LED_INTENSITY:
      menu_position = 6;
      write_config();
      state = SETTINGS;
      break;

    case FAN1_CURING:
      menu_position = 1;
      write_config();
      state = FANS;
      break;

    case FAN1_DRYING:
      menu_position = 2;
      write_config();
      state = FANS;
      break;

    case FAN2_CURING:
      menu_position = 3;
      write_config();
      state = FANS;
      break;

    case FAN2_DRYING:
      menu_position = 4;
      write_config();
      state = FANS;
      break;

    case ADVANCED_SETTINGS:
      switch (menu_position) {
        case 0:
          menu_position = 3;
          state = SETTINGS;
          break;

        case 1:
          menu_position = 0;
          state = RUN_MODE;
          break;

        case 2:
          menu_position = 0;
          state = PREHEAT;
          break;

        default:
          menu_position = 0;
          state = UNIT_SYSTEM;
          break;
      }
      break;

    case PREHEAT:
      switch (menu_position) {
        case 0:
          menu_position = 3;
          state = SETTINGS;
          break;

        case 1:
          menu_position = 0;
          state = PREHEAT_ENABLE;
          break;

        case 2:
          menu_position = 0;
          state = TARGET_TEMP;
          break;

        default:
          menu_position = 0;
          state = RESIN_TARGET_TEMP;
          break;
      }
      break;

    case SPEED_STATE:
      switch (menu_position) {
        case 0:
          menu_position = 1;
          state = SETTINGS;
          break;

        case 1:
          menu_position = 1;
          state = SPEED_CURING;
          break;

        default:
          menu_position = 2;
          state = SPEED_WASHING;
          break;
      }
      break;
    case SPEED_CURING:
      write_config();
      state = SPEED_STATE;
      break;

    case SPEED_WASHING:
      write_config();
      state = SPEED_STATE;
      break;


    case TIME:
      switch (menu_position) {
        case 0:
          menu_position = 1;
          state = MENU;
          break;

        case 1:
          menu_position = 0;
          state = TIME_CURING;
          break;

        case 2:
          menu_position = 0;
          state = TIME_DRYING;
          break;

        case 3:
          menu_position = 0;
          state = TIME_WASHING;
          break;

        default:
          menu_position = 0;
          state = TIME_RESIN_PREHEAT;
          break;
      }
      break;
    case BEEP:
      write_config();
      menu_position = 2;
      state = SOUND_SETTINGS;
      break;
    case SOUND:
      write_config();
      menu_position = 1;
      state = SOUND_SETTINGS;
      break;
    case TIME_CURING:
      write_config();
      menu_position = 1;
      state = TIME;
      break;
    case TIME_DRYING:
      write_config();
      menu_position = 2;
      state = TIME;
      break;
    case TIME_WASHING:
      write_config();
      menu_position = 3;
      state = TIME;
      break;
    case TIME_RESIN_PREHEAT:
      write_config();
      menu_position = 4;
      state = TIME;
      break;
    case INFO:
      menu_position = 7;
      state = SETTINGS;
      break;
    case RUN_MODE:
      write_config();
      if (!long_press) {
        menu_position = 2;
        state = SETTINGS;
      }
      else {
        long_press = false;
        menu_position = 0;
        state = MENU;
      }
      break;
    case PREHEAT_ENABLE:
      write_config();
      menu_position = 1;
      state = PREHEAT;
      break;
    case TARGET_TEMP:
      write_config();
      menu_position = 2;
      state = PREHEAT;
      break;

    case RESIN_TARGET_TEMP:
      write_config();
      menu_position = 3;
      state = PREHEAT;
      break;
    case UNIT_SYSTEM:
      write_config();
      menu_position = 8;
      state = SETTINGS;
      break;

    case CONFIRM:
      menu_position = 0;
      state = MENU;
      break;

    case RUN_MENU:
      switch (menu_position) {
        case 0:
          if (curing_mode) { //curing mode
            if (!gastro_pan) {
              paused ^= 1;
              if (paused) {
                stop_motor();
                running_count = 0;
                stop_heater(); // turn off heat fan
                fan_duty[0] = FAN1_MENU_SPEED;
                fan_duty[1] = FAN2_MENU_SPEED;
              } else {
                run_motor();
                motor_configuration();
                speed_control.speed_configuration(curing_mode);
                running_count = 0;

                if (!config.heat_to_target_temp) {
                  fan_duty[0] = config.FAN1_CURING_SPEED;
                  fan_duty[1] = config.FAN2_CURING_SPEED;
                }
                else {
                  fan_duty[0] = config.FAN1_PREHEAT_SPEED;
                  fan_duty[1] = config.FAN2_PREHEAT_SPEED;
                }
              }
              menu_position = 0;
              state = RUNNING;
            }
          }
          else { //washing mode
            if (!gastro_pan) {
              paused ^= 1;
              if (paused) {
                stop_motor();
                running_count = 0;
                stop_heater(); // turn off heat fan
                //fan1_duty = FAN1_MENU_SPEED;
                //fan2_duty = FAN2_MENU_SPEED;
              } else {
                run_motor();
            	motor_configuration();
                speed_control.speed_configuration(curing_mode);
                running_count = 0;

                fan_duty[0] = FAN1_WASHING_SPEED;
                fan_duty[1] = FAN2_WASHING_SPEED;
              }
              menu_position = 0;
              state = RUNNING;
            }
          }
          break;

        case 1:
          menu_position = 0;
          pid_mode = false;
          state = MENU;
          running_count = 0;
          stop_motor();
          paused = false;
          cover_open = false;

          outputchip.digitalWrite(EN_PIN, HIGH); // disable driver
          stop_heater(); // turn off heat fan
          fan_duty[0] = FAN1_MENU_SPEED;
          fan_duty[1] = FAN2_MENU_SPEED;
          outputchip.digitalWrite(LED_RELE_PIN, LOW); // turn off led
          digitalWrite(LED_PWM_PIN, LOW);
          tDown.stop();
          tUp.stop();
          break;

        case 2:
          running_count = 0;
          menu_position = 0;
          state = RUNNING;
          break;

        default:
          break;
      }
      break;

    case RUNNING:
      menu_position = 0;
      state = RUN_MENU;
      break;

    case SELFTEST:
        	switch (selftest.phase) {
        	        case 0:
        	        	if(menu_position){
        	        		selftest.phase++;
        	        	} else
        	        		state = MENU;
        	        	break;

        	        case 1:
        	        	if(selftest.cover_test){
        	        		selftest.phase++;
        	        		selftest.clean_up();
        	        		//state = MENU;
        	        	}
        	        	break;
        	        case 2:
        	            if(selftest.tank_test){
        	            	selftest.phase++;
        	            	selftest.clean_up();
        	            	//state = MENU;
        	            }
        	            break;
        	        case 3:
        	        	if(selftest.vent_test){
        	        		selftest.phase++;
        	        		selftest.clean_up();
        	        		//state = MENU;
        	        	}
        	        	break;
        	        case 4:
        	        	if(selftest.led_test){
        	        		selftest.phase ++;
        	        		selftest.clean_up();
        	        		//state = MENU;
        	        	}
        	            break;
        	        case 5:
        	        	if(selftest.heater_test){
        	        		selftest.phase++;
        	        		selftest.clean_up();
        	        		//state = MENU;
        	        	}
        	        	break;

        	        case 6:
        	        	if(selftest.rotation_test){
        	        		selftest.phase = 0;
        	        		selftest.clean_up();
        	        		//state = MENU;
        	        	}
        	        	break;

        	        default:
        	        	break;
        	}
        	menu_position = 0;
        	break;

    default:
      break;
  }
  scrolling_list_set(menu_position);

  //redraw_menu = true;
  rotary_diff = 128;
  redraw_menu = true;
  menu_move(true);
  //delay(475);
}

ISR(TIMER3_COMPA_vect) { // timmer for stepper move

  if (speed_control.motor_running == true) {
	OCR3A = speed_control.microstep_control;
    digitalWrite(STEP_PIN, HIGH);
    delayMicroseconds(2);
    digitalWrite(STEP_PIN, LOW);
    delayMicroseconds(2);
  }
}

//ISR(TIMER1_COMPA_vect) // timmer for encoder reading
void read_encoder()
{

  static byte lcd_encoder_bits = 0;
  byte enc = 0;
  if (digitalRead(BTN_EN1) == HIGH) {
    enc |= B01;
  }
  if (digitalRead(BTN_EN2) == HIGH) {
    enc |= B10;
  }
  if (enc != lcd_encoder_bits)
  {
    switch (enc)
    {
      case encrot0:
        if (lcd_encoder_bits == encrot3) {
          if (rotary_diff < 255) {
            rotary_diff++;
          }
        } else if (lcd_encoder_bits == encrot1) {
          if (rotary_diff) {
            rotary_diff--;
          }
        }
        break;
      case encrot1:
        if (lcd_encoder_bits == encrot0) {
          if (rotary_diff < 255) {
            rotary_diff++;
          }
        } else if (lcd_encoder_bits == encrot2) {
          if (rotary_diff) {
            rotary_diff--;
          }
        }
        break;
      case encrot2:
        if (lcd_encoder_bits == encrot1) {
          if (rotary_diff < 255) {
            rotary_diff++;
          }
        } else if (lcd_encoder_bits == encrot3) {
          if (rotary_diff) {
            rotary_diff--;
          }
        }
        break;
      case encrot3:
        if (lcd_encoder_bits == encrot2) {
          if (rotary_diff < 255) {
            rotary_diff++;
          }
        } else if (lcd_encoder_bits == encrot0) {
          if (rotary_diff) {
            rotary_diff--;
          }
        }
        break;
    }
    lcd_encoder_bits = enc;
  }
}

SIGNAL(TIMER0_COMPA_vect) //1ms timer
{
  ms_counter++;
  if (!heater_error) {
    read_encoder();
  }

  if (pid_mode) {
    byte tmpTargetTemp;
    if (config.curing_machine_mode == 0 || config.curing_machine_mode == 2 || config.curing_machine_mode == 3 || (selftest.phase == 5 && state == SELFTEST)) {
    	if(config.curing_machine_mode != 3)
    		tmpTargetTemp = config.target_temp_celsius;
    	else
    		tmpTargetTemp = config.resin_target_temp_celsius;

    	if (chamber_temp_celsius >= tmpTargetTemp) {
    		fan_duty[0] = PID(chamber_temp_celsius, tmpTargetTemp);
    		fan_duty[1] = fan_duty[0];
    	} else {
    		fan_duty[0] = FAN1_MENU_SPEED;
    		fan_duty[1] = FAN2_MENU_SPEED;
    	}
    }
  }

  fan_pwm_control();

  fan_rpm();

  if(state == SELFTEST && ms_counter % 1000 == 0){
	  if(selftest.phase == 3 || selftest.phase == 4 || selftest.phase == 5){
		  if(selftest.phase == 3 && selftest.vent_test != true){
			  lcd.setCursor(8, 1);
			  lcd.print(selftest.fan_tacho[0]);
			  lcd.setCursor(8, 2);
			  lcd.print(selftest.fan_tacho[1]);
	      }
		  if(selftest.phase == 5 && selftest.heater_test != true){
			  lcd.setCursor(8, 1);
			  lcd.print(chamber_temp_celsius, 1);
		  }
	      byte lcd_min = selftest.tCountDown.getCurrentMinutes();
	      byte lcd_sec = selftest.tCountDown.getCurrentSeconds();
	      if(lcd_min > 9)
	    	  lcd.setCursor(6, 3);
	      else
	    	  lcd.setCursor(7, 3);
	      lcd.print(lcd_min);
	      lcd.print(":");
	      if(lcd_sec < 10)
	    	  lcd.print("0");
	      lcd.print(lcd_sec);
	  }
  }

  if (ms_counter >= 4000) {

    //lcd.begin_noclear(20, 4);
    //redraw_menu = true;
    //menu_move(true);
    ms_counter = 0;
  } else {
    redraw_menu = false;
  }
}

void tDownComplete()
{
  tDown.stop();
}
void tUpComplete()
{
  tUp.stop();
}

void print_time1()
{
  //Serial.print("tDown: ");
  //Serial.println(tDown.getCurrentTime());

}

void print_time2()
{
  //Serial.print("tUp: ");
  //Serial.println(tUp.getCurrentTime());

}

void start_drying() {
  if (cover_open == false && paused == false) {
    if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
      if (!preheat_complete) {
        preheat(); // turn on heat fan
      }
      else {
        run_heater(); // turn on heat fan
      }
    }
    else {
      run_heater(); // turn on heat fan
    }
  }
  if (cover_open == true || paused == true) {
    if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
      if (!paused_time) {
        paused_time = true;
      }
      if (!preheat_complete) {
        tUp.pause();
      }
      else {
        tDown.pause();
      }

    }
    else {
      if (!paused_time) {
        paused_time = true;
      }
      tDown.pause();
    }
  }
  else {
    if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
      if (paused_time) {
        paused_time = false;
        redraw_menu = true;
        menu_move(true);
      }
      if (!preheat_complete) {
        tUp.start();
      }
      else {
        tDown.start();
      }
    }
    else {
      if (paused_time) {
        paused_time = false;
        redraw_menu = true;
        menu_move(true);
      }
      tDown.start();
    }

  }
  if (outputchip.digitalRead(WASH_DETECT_PIN) == LOW) { //Gastro Pen check
    lcd.setCursor(1, 0);
    lcd.print("Remove IPA tank!");
    running_count = 0;
    paused = true;
    if (!paused_time) {
      paused_time = true;
    }
    tDown.pause();
    stop_heater();
    stop_motor();
    if (!gastro_pan) {
      redraw_menu = true;
      menu_move(true);
      gastro_pan = true;
    }
  }
  else {
    if (gastro_pan) {
      redraw_menu = true;
      menu_move(true);
      gastro_pan = false;
    }
  }
  if (config.heat_to_target_temp || (config.curing_machine_mode == 3))
  {
    if (!preheat_complete) lcd_time_print(8);
    else lcd_time_print(7);
  }
  else
  {
    lcd_time_print(7);
  }
}

void start_curing() {
  stop_heater(); // turn off heat fan
  if (cover_open == false && paused == false) {
    if (!led_start) {
      led_start = true;
      led_time_now = millis();
    }
    if (millis() > led_time_now + LED_delay) {
      outputchip.digitalWrite(LED_RELE_PIN, HIGH); // turn LED on
      int val;
      val = map(LED_PWM_VALUE, 0, 100, 0, 255);
      analogWrite(LED_PWM_PIN, val);
    }
  }
  else {
    if (led_start) {
      outputchip.digitalWrite(LED_RELE_PIN, LOW); // turn LED off
      digitalWrite(LED_PWM_PIN, LOW);
      led_start = false;
    }
  }
  if (cover_open == true || paused == true) {
    if (!paused_time) {
      paused_time = true;
    }
    tDown.pause();
  }
  else {
    if (paused_time) {
      paused_time = false;
      redraw_menu = true;
      menu_move(true);
    }
    tDown.start();
  }
  if (outputchip.digitalRead(WASH_DETECT_PIN) == LOW) { //Gastro Pen check
    lcd.setCursor(1, 0);
    lcd.print("Remove IPA tank!");
    running_count = 0;
    paused = true;
    if (!paused_time) {
      paused_time = true;
    }
    tDown.pause();
    stop_heater();
    stop_motor();
    if (!gastro_pan) {
      redraw_menu = true;
      menu_move(true);
      gastro_pan = true;
    }
  }
  else {
    if (gastro_pan) {
      redraw_menu = true;
      menu_move(true);
      gastro_pan = false;
    }
  }
  lcd_time_print(7);
}

void start_washing() {
  if (pinda_therm == 1) {
    if (outputchip.digitalRead(COVER_OPEN_PIN) == LOW) { //Cover open check

      lcd.setCursor(1, 0);
      lcd.print("Open cover!      ");
      running_count = 0;
      paused = true;
      tDown.pause();
      stop_motor();

      if (outputchip.digitalRead(WASH_DETECT_PIN) == LOW) { //Gastro Pen check
        lcd.setCursor(1, 0);
        lcd.print("IPA tank removed");
        running_count = 0;
        paused = true;
        if (!paused_time) {
          paused_time = true;
          redraw_menu = true;
          menu_move(true);
        }
        tDown.pause();
        stop_motor();
        gastro_pan = true;
      }
      else {
        if (paused_time) {
          paused_time = false;
        }
        if (gastro_pan) {
          redraw_menu = true;
          menu_move(true);
          gastro_pan = false;
        }
      }
    }
    if (tDown.isCounterCompleted() == false)  {
      if (state == RUNNING) {
        if ((paused == false) && (outputchip.digitalRead(WASH_DETECT_PIN) == HIGH)) {
          run_motor();
          tDown.start();
        }

        lcd_time_print(8);
      }
    } else {
      menu_position = 0;
      stop_motor();

      stop_heater(); // turn off heat fan
      redraw_menu = true;
      rotary_diff = 128;
      switch (config.finish_beep_mode) {
        case 2:
          beep();

          state = CONFIRM;
          break;
        case 1:
          beep();
          state = MENU;
          break;

        case 0:
        default:
          state = MENU;
          break;
      }
    }
  }
  else {
    if (cover_open) {
      redraw_menu = true;
      cover_open = false;
    }
    if (outputchip.digitalRead(WASH_DETECT_PIN) == HIGH) { //Gastro Pen check
      lcd.setCursor(1, 0);
      lcd.print("IPA tank removed");
      running_count = 0;
      paused = true;
      if (!paused_time) {
        paused_time = true;
      }
      tDown.pause();
      stop_motor();
      if (!gastro_pan) {
        redraw_menu = true;
        menu_move(true);
        gastro_pan = true;
      }
    }
    else {
      if (paused_time) {
        paused_time = false;
      }
      if (gastro_pan) {
        redraw_menu = true;
        menu_move(true);
        gastro_pan = false;
      }
    }
    if (tDown.isCounterCompleted() == false)  {
      if (state == RUNNING) {
        if ((paused == false) && (outputchip.digitalRead(WASH_DETECT_PIN) == LOW)) {
          run_motor();
          tDown.start();
        }
        else {
          tDown.pause();
        }

        lcd_time_print(8);
      }
    } else {
      menu_position = 0;
      stop_motor();
      fan_duty[0] = FAN1_MENU_SPEED;
      fan_duty[1] = FAN2_MENU_SPEED;
      stop_heater(); // turn off heat fan
      redraw_menu = true;
      rotary_diff = 128;
      switch (config.finish_beep_mode) {
        case 2:
          beep();

          state = CONFIRM;
          break;
        case 1:
          beep();
          state = MENU;
          break;

        case 0:
        default:
          state = MENU;
          break;
      }
      menu_move(true);
    }
  }
}

void stop_curing_drying() {
  pid_mode = false;
  menu_position = 0;
  outputchip.digitalWrite(LED_RELE_PIN, LOW); // turn off led
  digitalWrite(LED_PWM_PIN, LOW);
  stop_motor();
  stop_heater(); // turn off heat fan
  fan_duty[0] = FAN1_MENU_SPEED;
  fan_duty[1] = FAN2_MENU_SPEED;
  redraw_menu = true;
  rotary_diff = 128;
  switch (config.finish_beep_mode) {
    case 2:
      beep();

      state = CONFIRM;
      break;
    case 1:
      beep();
      state = MENU;
      break;

    case 0:
    default:
      state = MENU;
      break;
  }
  menu_move(true);
}

//! @brief Display remaining time
//!
//! @param dots_column Zero indexed column of first line
//! where to to start printing progress dots.
void lcd_time_print(uint8_t dots_column) {
  byte mins;
  byte secs;
  if (config.heat_to_target_temp || (config.curing_machine_mode == 3)) {
    if (drying_mode) {
      if (!preheat_complete) {
        mins = tUp.getCurrentMinutes();
        secs = tUp.getCurrentSeconds();
      }
      else {
        mins = tDown.getCurrentMinutes();
        secs = tDown.getCurrentSeconds();
      }
    }
    else {
      mins = tDown.getCurrentMinutes();
      secs = tDown.getCurrentSeconds();
    }
  }
  else {
    mins = tDown.getCurrentMinutes();
    secs = tDown.getCurrentSeconds();
  }


  if (state == RUNNING && (secs != last_seconds || redraw_ms)) {

    //therm1_read();
    //therm2_read();
    redraw_ms = false;
    last_seconds = secs;

    lcd.setCursor(8, 2);

    if (mins < 10) {
      lcd.print(0);
    }
    lcd.print(mins);
    lcd.print(":");
    if (secs < 10) {
      lcd.print(0);
    }
    lcd.print(secs);
    if (!paused && !paused_time) {
      lcd.setCursor(19, 1);
      lcd.print(" ");

      if (curing_mode) {


        if (outputchip.digitalRead(COVER_OPEN_PIN) == LOW) {
          if (config.SI_unit_system) {
            lcd.setCursor(13, 0);
            lcd.print(chamber_temp_celsius, 1);
            lcd.setCursor(18, 0);
            lcd.print((char)223);
            lcd.setCursor(19, 0);
            lcd.print("C");
          }
          else {
            lcd.setCursor(13, 0);
            lcd.print(chamber_temp_fahrenheit, 1);
            lcd.setCursor(18, 0);
            lcd.print((char)223);
            lcd.setCursor(19, 0);
            lcd.print("F");
          }
        }
      }

      if (running_count == 0) {
        lcd.setCursor(dots_column, 0);
        lcd.print("   ");
      }
      if (running_count == 1) {
        lcd.setCursor(dots_column, 0);
        lcd.print(".  ");
      }
      if (running_count == 2) {
        lcd.setCursor(dots_column, 0);
        lcd.print(".. ");
        lcd.setCursor(14, 2);
        lcd.print("  ");
        lcd.setCursor(5, 2);
        lcd.print("  ");
      }
      if (running_count == 3) {
        lcd.setCursor(dots_column, 0);
        lcd.print("...");
      }

    }
    running_count++;

    if (running_count > 4) {
      lcd.setCursor(14, 2);
      lcd.print("  ");
      lcd.setCursor(5, 2);
      lcd.print("  ");
      running_count = 0;
      redraw_menu = true;
    }
  }
}

void fan_pwm_control() {
#if (BOARD == 3)
  unsigned long currentMillis = millis();
  if (fan_duty[0] > 0) {
    if (fan1_pwm_State == LOW) {
      if (currentMillis - fan1_previous_millis >= ((period) * (1 - ((float)fan_duty[0] / 100)))) {
        fan1_previous_millis = currentMillis;
        PORTC = PORTC | 0x80; //OUTPUT FAN1 HIGH
        fan1_pwm_State = HIGH;
      }
    }
    if (fan1_pwm_State == HIGH) {
      if (currentMillis - fan1_previous_millis >= ((period) * ((float)fan_duty[0] / 100))) {
        fan1_previous_millis = currentMillis;
        PORTC = PORTC & 0x7F; //OUTPUT FAN1 LOW
        fan1_pwm_State = LOW;
      }
    }
  }
  else {
    PORTC = PORTC & 0x7F; //OUTPUT FAN1 LOW
  }
  if (fan_duty[1] > 0) {
    if (fan2_pwm_State == LOW) {
      if (currentMillis - fan2_previous_millis >= ((period) * (1 - ((float)fan_duty[1] / 100)))) {
        fan2_previous_millis = currentMillis;
        PORTB = PORTB | 0x80; //OUTPUT FAN2 HIGH
        fan2_pwm_State = HIGH;
      }
    }
    if (fan2_pwm_State == HIGH) {
      if (currentMillis - fan2_previous_millis >= ((period) * ((float)fan_duty[1] / 100))) {
        fan2_previous_millis = currentMillis;
        PORTB = PORTB & 0x7F; //OUTPUT FAN2 LOW
        fan2_pwm_State = LOW;
      }
    }
  }

  else {
    PORTB = PORTB & 0x7F; //OUTPUT FAN2 LOW
  }
#endif

#if (BOARD == 4)
  //rev 0.4 - inverted PWM FAN1, FAN2
  unsigned long currentMillis = millis();
  if (fan_duty[0] > 0) {

    if (!fan1_on) {
      fan1_on = true;
      outputchip.digitalWrite(FAN1_PIN, HIGH);
    }

    if (fan1_pwm_State == LOW) {
      if (currentMillis - fan1_previous_millis >= ((period) * (1 - ((float)fan_duty[0] / 100)))) {
        fan1_previous_millis = currentMillis;
        PORTC = PORTC & 0x7F; //OUTPUT FAN1 LOW
        fan1_pwm_State = HIGH;
      }
    }
    if (fan1_pwm_State == HIGH) {
      if (currentMillis - fan1_previous_millis >= ((period) * ((float)fan_duty[0] / 100))) {
        fan1_previous_millis = currentMillis;
        PORTC = PORTC | 0x80; //OUTPUT FAN1 HIGH
        fan1_pwm_State = LOW;
      }
    }
  }
  else {
    if (fan1_on) {
      fan1_on = false;
      outputchip.digitalWrite(FAN1_PIN, LOW);
      PORTC = PORTC & 0x7F; //OUTPUT FAN1 LOW
    }
  }
  if (fan_duty[1] > 0) {
    if (!fan2_on) {
      fan2_on = true;
      outputchip.digitalWrite(FAN2_PIN, HIGH);
    }
    if (fan2_pwm_State == LOW) {
      if (currentMillis - fan2_previous_millis >= ((period) * (1 - ((float)fan_duty[1] / 100)))) {
        fan2_previous_millis = currentMillis;
        PORTB = PORTB & 0x7F; //OUTPUT FAN2 LOW
        fan2_pwm_State = HIGH;

      }
    }
    if (fan2_pwm_State == HIGH) {
      if (currentMillis - fan2_previous_millis >= ((period) * ((float)fan_duty[1] / 100))) {
        fan2_previous_millis = currentMillis;
        PORTB = PORTB | 0x80; //OUTPUT FAN2 HIGH
        fan2_pwm_State = LOW;
      }
    }
  }

  else {
    if (fan2_on) {
      fan2_on = false;
      outputchip.digitalWrite(FAN2_PIN, LOW);
      PORTB = PORTB & 0x7F; //OUTPUT FAN2 LOW
    }
  }
#endif
}
void therm1_read() {
  outputchip.digitalWrite(9, LOW);
  outputchip.digitalWrite(10, LOW);

  if (pinda_therm) {
    float voltage;

    voltage = analogRead(THERM_READ_PIN);

    voltage = 1023 / voltage - 1;
    voltage = thermistor_pullup_1 / voltage;
    chamber_temp_celsius = voltage / thermNom_1;         // (R/Ro)
    chamber_temp_celsius = log(chamber_temp_celsius);             // ln(R/Ro)
    chamber_temp_celsius /= beta_1;                    // 1/B * ln(R/Ro)
    chamber_temp_celsius += 1.0 / (refTemp_1 + 273.15); // + (1/To)
    chamber_temp_celsius = 1.0 / chamber_temp_celsius;
    chamber_temp_celsius -= 273.15;                  // conversion from Kelvin to Celsius
    chamber_temp_fahrenheit = (1.8 * chamber_temp_celsius) + 32;
  }
  else {
    chamber_temp_celsius = therm1.analog2temp();
    chamber_temp_fahrenheit = (1.8 * chamber_temp_celsius) + 32;
  }

}

void therm2_read() {
  outputchip.digitalWrite(9, LOW);
  outputchip.digitalWrite(10, HIGH);
  float voltage;

  voltage = analogRead(THERM_READ_PIN);

  voltage = 1023 / voltage - 1;
  voltage = thermistor_pullup_2 / voltage;

  led_temp = voltage / thermNom_2;         // (R/Ro)
  led_temp = log(led_temp);             // ln(R/Ro)
  led_temp /= beta_2;                    // 1/B * ln(R/Ro)
  led_temp += 1.0 / (refTemp_2 + 273.15); // + (1/To)
  led_temp = 1.0 / led_temp;
  led_temp -= 273.15;                  // conversion from Kelvin to Celsius

}

void fan_rpm() {
  ams_fan_counter ++;
  if (ams_fan_counter % 100 == 0) {
	  for(short j = 0; j < 2; j++){
		if (fan_tacho_count[j] <= fan_tacho_last_count[j] ) {
		  	if (fan_duty[j] > 0)
		  		fan_error[j] = true;
		} else
		  		fan_error[j] = false;

		selftest.fan_tacho[j] = fan_tacho_count[j] - fan_tacho_last_count[j];
		fan_tacho_last_count[j] = fan_tacho_count[j];
		if (fan_tacho_count[j] >= 10000) {
			fan_tacho_count[j] = 0;
		  	fan_tacho_last_count[j] = 0;
		}
	  }
	  if(ams_fan_counter >= 1000){
		  if (heater_running) {
			  heater_error = (fan_tacho_count[2] <= fan_tacho_last_count[2]);
			  if(config.heater_failure != heater_error){									//write to EEPROM only if state is changed
				  config.heater_failure = heater_error;
				  write_config();
			  }
			  fan_tacho_last_count[2] = fan_tacho_count[2];
			  if (fan_tacho_count[2] >= 10000) {
				  fan_tacho_count[2] = 0;
				  fan_tacho_last_count[2] = 0;
			  }
		  }
		  ams_fan_counter = 0;
	  }
  }
}

void fan_tacho1() {
  fan_tacho_count[0]++;
}
void fan_tacho2() {
  fan_tacho_count[1]++;
}
void fan_tacho3() {
  fan_tacho_count[2]++;
}

void preheat() {
  if (config.curing_machine_mode == 0 || config.curing_machine_mode == 2) {
    if (config.SI_unit_system) {
      if (chamber_temp_celsius < config.target_temp_celsius)
      {
        run_heater();
      }
      else {
        stop_heater();
        tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
      }
    }
    else {
      if (chamber_temp_fahrenheit < ((config.target_temp_celsius * 1.8) + 32))
      {
        run_heater();
      }
      else {
        stop_heater();
        tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
      }
    }
  }
  if (config.curing_machine_mode == 3) {
    if (config.SI_unit_system) {
      if (chamber_temp_celsius < config.resin_target_temp_celsius)
      {
        run_heater();
      }
      else {
        stop_heater();
        tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
      }
    }
    else {
      if (chamber_temp_fahrenheit < ((config.resin_target_temp_celsius * 1.8) + 32))
      {
        run_heater();
      }
      else {
        //stop_heater();
        tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
      }
    }
  }

}

#if 0
//! @brief Get reset flags
//! @return value of MCU Status Register - MCUSR as it was backed up by bootloader
static uint8_t get_reset_flags()
{
  return bootKeyPtrVal;
}
#endif

#define ATTR_INIT_SECTION(SectionIndex) __attribute__ ((used, naked, section (".init" #SectionIndex )))
void get_key_from_boot(void) ATTR_INIT_SECTION(3);

//! @brief Save the value of the boot key memory before it is overwritten
//!
//! Do not call this function, it is placed in one of the initialization sections,
//! which executes automatically before the main function of the application.
//! Refer to the avr-libc manual for more information on the initialization sections.
void get_key_from_boot(void)
{
  bootKeyPtrVal = *bootKeyPtr;
}
