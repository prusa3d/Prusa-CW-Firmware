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
//#include "LiquidCrystal_Prusa.h"

using Ter = PrusaLcd::Terminator;

typedef char Serial_num_t[20]; //!< Null terminated string for serial number


Countimer tDown;
Countimer tUp;

thermistor therm1(A4, 5);

Trinamic_TMC2130 myStepper(CS_PIN);

MCP outputchip(0, 8);


PrusaLcd lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7);

enum menu_state {
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
  ERROR
};

#define FW_VERSION  "2.1.4"
volatile uint16_t *const bootKeyPtr = (volatile uint16_t *)(RAMEND - 1);

menu_state state = MENU;

long lastJob = 0;

uint8_t menu_position = 0;
uint8_t last_menu_position = 0;
byte max_menu_position = 0;
bool redraw_menu = true;
bool redraw_ms = true;
bool speed_up = false;
bool pinda_therm = 0; // 0 - 100K thermistor - ATC Semitec 104GT-2/ 1 - PINDA thermistor

unsigned long time_now = 0;
unsigned long therm_read_time_now = 0;

bool button_released = false;
volatile uint8_t rotary_diff = 128;

typedef struct
{
	const char magic[6];
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

static constexpr eeprom_t const * eeprom_base = reinterpret_cast<eeprom_t*> (EEPROM.length() - EEPROM_OFFSET);

byte washing_speed = 10;
byte curing_speed = 1;
byte washing_run_time = 4;
byte curing_run_time = 3;
byte drying_run_time = 3;
byte resin_preheat_run_time = 3;
byte finish_beep_mode = 1;
byte sound_response = 1;
byte heat_to_target_temp = 0;
byte target_temp_celsius = 35;
byte target_temp_fahrenheit = 95;
byte resin_target_temp_celsius = 30;
byte curing_machine_mode;
byte SI_unit_system = 1;

int max_washing_speed = 16; //15 //smaller = faster
int min_washing_speed = 70; //100 //smaller = faster
int max_curing_speed = 25; //50 //smaller = faster
int min_curing_speed = 220; //smaller = faster
int set_washing_speed;
int set_curing_speed;

byte max_preheat_run_time = 30;
byte cover_check_enabled = 1;
byte LED_PWM_VALUE = 100;

int fan1_pwm_State = LOW;
int fan2_pwm_State = LOW;

long fan1_tacho_count;
long fan2_tacho_count;
long fan3_tacho_count;

bool fan1_on = false;
bool fan2_on = false;

long fan1_tacho_last_count;
long fan2_tacho_last_count;
long fan3_tacho_last_count;

unsigned long fan1_previous_millis = 0;
unsigned long fan2_previous_millis = 0;

bool heater_failure = false;
bool heater_error = false;
bool fan1_error = false;
bool fan2_error = false;

// constants won't change:
int fan_frequency = 70; //Hz
float period = (1 / (float)fan_frequency) * 1000; //
int fan1_duty; //%
int fan2_duty; //%

//fan1_duty = 0-100%
byte FAN1_MENU_SPEED = 30;
byte FAN1_CURING_SPEED = 60;//70
byte FAN1_WASHING_SPEED = 60;//70
byte FAN1_DRYING_SPEED = 60;//70
byte FAN1_PREHEAT_SPEED = 40;//40

//fan2_duty = 0-100%
byte FAN2_MENU_SPEED = 30;
byte FAN2_CURING_SPEED = 70;//30
byte FAN2_WASHING_SPEED = 70;//70
byte FAN2_DRYING_SPEED = 70;//70
byte FAN2_PREHEAT_SPEED = 40;//40

long remain = 0;
unsigned long us_last = 0;
unsigned long last_remain = 0;
unsigned long last_millis = 0;
unsigned int last_seconds = 0;
unsigned long led_time_now = 0;
unsigned long LED_delay = 1000;
bool motor_running = false;
bool heater_running = false;
bool curing_mode = false;
bool drying_mode = false;
bool last_curing_mode = false;
bool paused = false;
bool cover_open = false;
bool gastro_pan = false;
bool paused_time = false;
bool led_start = false;
int var_speed = 0;
int running_count = 0;

long thermNom_1 = 100000;
int refTemp_1 = 25;
int beta_1 = 4267;
long thermistor_pullup_1 = 100000;

long thermNom_2 = 100000;
int refTemp_2 = 25;
int beta_2 = 4250;
long thermistor_pullup_2 = 33000;

float chamber_temp_celsius;
float chamber_temp_fahrenheit;
float led_temp;
bool therm_read = false;

volatile int divider = 0;

long ams_counter;
long ms_counter;
long ams_fan_counter;

unsigned long button_timer = 0;
const unsigned long long_press_time = 1000;
bool button_active = false;
bool long_press_active = false;
bool long_press = false;
bool preheat_complete = false;
bool pid_mode = false;

double actualTemp = 0;
double errValue = 0;
double summErr = 0;
double diffTemp = 0;
double oldSpeed = 0;
double targetTemp = 30;
double newSpeed = 0;

double P = 10;//0.5
double I = 0.001; //0.001;
double D = 1; //0.1;

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


#define EEPROM_OFFSET 128
#define MAGIC_SIZE 6
const char magic[MAGIC_SIZE] = "CURWA";
const char magic2[MAGIC_SIZE] = "CURW1";


static void motor_configuration();
static void read_config(unsigned int address);
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
static void fan_heater_rpm();
static void fan_rpm();
static void preheat();
static void lcd_time_print(uint8_t dots_column);
static void therm1_read();
static void get_serial_num(Serial_num_t &sn);

static inline bool is_error()
{
    return (fan1_error || fan2_error || heater_failure);
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
  motor_running = true;

}

void stop_motor() {

  outputchip.digitalWrite(EN_PIN, HIGH); // disable driver
  motor_running = false;

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

void speed_configuration() {

  if (curing_mode == true) {
    set_curing_speed = map(curing_speed, 1, 10, min_curing_speed, max_curing_speed);
    motor_configuration();
    OCR3A = set_curing_speed;
  }

  else {
    set_washing_speed = map(washing_speed, 1, 10, min_washing_speed, max_washing_speed);
    motor_configuration();
    var_speed = min_washing_speed;
    speed_up = true;
  }
}

void motor_configuration() {

  if (curing_mode == true) {
    myStepper.set_IHOLD_IRUN(10, 10, 0);
    setupTimer3();
    OCR3A = min_curing_speed; //smaller = faster
    myStepper.set_mres(256);
  }

  else {
    myStepper.set_IHOLD_IRUN(31, 31, 5);
    setupTimer3();
    OCR3A = 100; //smaller = faster
    myStepper.set_mres(16);

  }
}

void setup() {

  //Serial.begin(115200);
  outputchip.begin();
  outputchip.pinMode(0B0000000010010111);
  outputchip.pullupMode(0B0000000010000011);
  read_config(EEPROM.length() - EEPROM_OFFSET);

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

  fan1_duty = FAN1_MENU_SPEED;
  fan2_duty = FAN2_MENU_SPEED;

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

void write_config(unsigned int address) {		//useless address param
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->magic)), magic2);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->washing_speed)), washing_speed);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->curing_speed)), curing_speed);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->washing_run_time)), washing_run_time);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->curing_run_time)), curing_run_time);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->finish_beep_mode)), finish_beep_moded);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->drying_run_time)), drying_run_time);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->sound_response)), sound_response);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->curing_machine_mode)), curing_machine_mode);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->heat_to_target_temp)), heat_to_target_temp);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->target_temp_celsius)), target_temp_celsius);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->target_temp_fahrenheit)), target_temp_fahrenheit);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->SI_unit_system)), SI_unit_system);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->heater_failure)), heater_failure);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->FAN1_CURING_SPEED)), FAN1_CURING_SPEED);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->FAN1_DRYING_SPEED)), FAN1_DRYING_SPEED);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->FAN1_PREHEAT_SPEED)), FAN1_PREHEAT_SPEED);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->FAN2_CURING_SPEED)), FAN2_CURING_SPEED);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->FAN2_DRYING_SPEED)), FAN2_DRYING_SPEED);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->FAN2_PREHEAT_SPEED)), FAN2_PREHEAT_SPEED);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->resin_preheat_run_time)), resin_preheat_run_time);
  EEPROM.put(reinterpret_cast<int>(&(eeprom_base->resin_target_temp_celsius)), resin_target_temp_celsius);
}

void read_config(unsigned int address) {
  char test_magic[MAGIC_SIZE];
  EEPROM.get(reinterpret_cast<int>(&(eeprom_base->magic)), test_magic);
  if (!strncmp(magic2, test_magic, MAGIC_SIZE)) {
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->washing_speed)), washing_speed);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->curing_speed)), curing_speed);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->washing_run_time)), washing_run_time);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->curing_run_time)), curing_run_time);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->finish_beep_mode)), finish_beep_moded);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->drying_run_time)), drying_run_time);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->sound_response)), sound_response);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->curing_machine_mode)), curing_machine_mode);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->heat_to_target_temp)), heat_to_target_temp);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->target_temp_celsius)), target_temp_celsius);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->target_temp_fahrenheit)), target_temp_fahrenheit);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->SI_unit_system)), SI_unit_system);
    EEPROM.get(reinterpret_cast<int>(&(eeprom_base->heater_failure)), heater_failure);
    if(!strncmp(magic, test_magic, MAGIC_SIZE)){
    	EEPROM.get(reinterpret_cast<int>(&(eeprom_base->FAN1_CURING_SPEED)), FAN1_CURING_SPEED);
    	EEPROM.get(reinterpret_cast<int>(&(eeprom_base->FAN1_DRYING_SPEED)), FAN1_DRYING_SPEED);
    	EEPROM.get(reinterpret_cast<int>(&(eeprom_base->FAN1_PREHEAT_SPEED)), FAN1_PREHEAT_SPEED);
    	EEPROM.get(reinterpret_cast<int>(&(eeprom_base->FAN2_CURING_SPEED)), FAN2_CURING_SPEED);
    	EEPROM.get(reinterpret_cast<int>(&(eeprom_base->FAN2_DRYING_SPEED)), FAN2_DRYING_SPEED);
    	EEPROM.get(reinterpret_cast<int>(&(eeprom_base->FAN2_PREHEAT_SPEED)), FAN2_PREHEAT_SPEED);
    	EEPROM.get(reinterpret_cast<int>(&(eeprom_base->resin_preheat_run_time)), resin_preheat_run_time);
    	EEPROM.get(reinterpret_cast<int>(&(eeprom_base->resin_target_temp_celsius)), resin_target_temp_celsius);
    }
  }
}

void PID() {

  newSpeed = 0;

  errValue = actualTemp - targetTemp;

  summErr = errValue + summErr;

  if ((summErr > 10000) || (summErr < -10000))
    summErr = 10000;

  diffTemp = actualTemp - oldSpeed;
  newSpeed = P * errValue + I * summErr + D * diffTemp;
  if (newSpeed > 100) {
    newSpeed = 100;
  }
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

void generic_menu(byte num, ...) {
  va_list argList;
  va_start(argList, num);
  max_menu_position = 0;
  for (; num; num--) {
    lcd.setCursor(1, max_menu_position++);
    lcd.print(va_arg(argList, const char *));
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

  if (heater_error) {
    if (heat_to_target_temp) {
      tDown.stop();
    }
    else {
      tUp.stop();
    }
    stop_heater(); // turn off heat fan
    stop_motor(); // turn off motor
    fan1_duty = FAN1_MENU_SPEED;
    fan2_duty = FAN2_MENU_SPEED;
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

  if (speed_up == true) { //stepper motor speed up function
    unsigned long us_now = millis();
    if (us_now - us_last > 50) {
      if (var_speed >= set_washing_speed) {
        var_speed--;
        setupTimer3();
        OCR3A = var_speed;
        us_last = us_now;
      }
    }
    if (var_speed == set_washing_speed) {
      speed_up = false;
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
        if (sound_response) {
          //echo();
        }
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
    if (state == SETTINGS || state == FANS || state == TIME) {
    }

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
    if (state == SETTINGS || state == FANS || state == TIME) {
    }
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
      if (sound_response) {
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

      switch (curing_machine_mode) {
        case 3:
          generic_menu(3, curing_mode ? "Start resin preheat" : "Start washing      ", "Run-time",
                  is_error() ? "Settings ->!!" : "Settings          ");
          lcd_print_right(1);
          lcd_print_right(2);

          state = MENU;
          break;
        case 2:
          generic_menu(3, curing_mode ? "Start drying       " : "Start washing      ", "Run-time",
                  is_error() ? "Settings ->!!" : "Settings          ");
          lcd_print_right(1);
          lcd_print_right(2);

          state = MENU;
          break;
        case 1:
          generic_menu(3, curing_mode ? "Start curing       " : "Start washing      ", "Run-time",
                  is_error() ? "Settings ->!!" : "Settings          ");
          lcd_print_right(1);
          lcd_print_right(2);

          state = MENU;
          break;
        case 0:
        default:
          generic_menu(3, curing_mode ? "Start drying/curing" : "Start washing", "Run-time",
                  is_error() ? "Settings ->!!" : "Settings          ");
          lcd_print_right(1);
          lcd_print_right(2);

          state = MENU;
          break;
      }

      break;

    case SPEED_STATE:

      generic_menu(3, "Back              ", "Curing speed", "Washing speed");
      lcd_print_back();
      lcd_print_right(1);
      lcd_print_right(2);

      break;

    case SPEED_CURING:

      generic_value("Curing speed", &curing_speed, 1, 10, "/10", 0);

      break;

    case SPEED_WASHING:

      generic_value("Washing speed", &washing_speed, 1, 10, "/10", 0);

      break;

    case TIME:
      {
        Scrolling_items items =
        {
          {"Back", true, Ter::back},
          {"Curing", true, Ter::right},
          {"Drying", true, Ter::right},
          {"Washing", true, Ter::right},
          {"Resin preheat", true, Ter::right},
        };
        menu_position = scrolling_list(items);

        break;
      }
    case TIME_CURING:

      generic_value("Curing run-time", &curing_run_time, 1, 10, " min", 0);

      break;

    case TIME_DRYING:

      generic_value("Drying run-time", &drying_run_time, 1, 10, " min", 0);

      break;

    case TIME_WASHING:

      generic_value("Washing run-time", &washing_run_time, 1, 10, " min", 0);

      break;

    case TIME_RESIN_PREHEAT:

      generic_value("Resin preheat time", &resin_preheat_run_time, 1, 10, " min", 0);

      break;

    case SETTINGS:
    {
      Scrolling_items items =
      {
        {"Back", true, Ter::back},
        {"Rotation speed", true, Ter::right},
        {"Run mode", true, Ter::right},
        {"Preheat", true, Ter::right},
        {"Sound", true, Ter::right},
        {"Fans", true, Ter::right},
        {"LED intensity", true, Ter::right},
        {is_error() ? "Information ->!!" : "Information", true, Ter::right},
        {"Unit system", true, Ter::right},
      };
      menu_position = scrolling_list(items);
      break;
    }
    case ADVANCED_SETTINGS:
      generic_menu(4, "Back              ", "Run mode", "Preheat", "Unit system");
      lcd_print_back();
      lcd_print_right(1);
      lcd_print_right(2);
      lcd_print_right(3);
      break;

    case PREHEAT:
      if (heat_to_target_temp) {
        generic_menu(4, "Back              ", "Preheat enabled", "Drying/Curing temp", "Resin preheat temp" );
      }
      else {
        generic_menu(4, "Back              ", "Preheat disabled", "Drying/Curing temp", "Resin preheat temp" );
      }
      lcd_print_back();
      lcd_print_right(1);
      lcd_print_right(2);
      lcd_print_right(3);
      break;

    case PREHEAT_ENABLE:

      generic_items("Preheat", &heat_to_target_temp, 2, "disable >", "< enable");

      break;

    case TARGET_TEMP:
      if (SI_unit_system) {
        generic_value("Target temp", &target_temp_celsius, 20, 40, " \xDF" "C ", 0);
      }
      else {
        generic_value("Target temp", &target_temp_celsius, 20, 40, " \xDF" "F ", 1);
      }
      break;

    case RESIN_TARGET_TEMP:
      if (SI_unit_system) {
        generic_value("Target temp", &resin_target_temp_celsius, 20, 40, "\xDF" "C", 0);
      }
      else {
        generic_value("Target temp", &resin_target_temp_celsius, 20, 40, "\xDF" "F", 1);
      }
      break;

    case UNIT_SYSTEM:
      generic_items("Unit system", &SI_unit_system, 2, "IMPERIAL/ US >", "< SI");
      break;

    case RUN_MODE:
      generic_items("Run mode", &curing_machine_mode, 4, "Drying & Curing >", "< Curing >", "< Drying >", "< Resin preheat");
      break;

    case SOUND_SETTINGS:
      generic_menu(3, "Back              ", "Sound response", "Finish beep" );
      lcd_print_back();
      lcd_print_right(1);
      lcd_print_right(2);

      break;

    case SOUND:
      generic_items("Sound response", &sound_response, 2, "no >", "< yes");
      break;

    case BEEP:
      generic_items("Finish beep", &finish_beep_mode, 3, "none >", "< once > ", "< continuous");
      break;

    case FANS:
      {
        Scrolling_items items =
        {
          {"Back", true, Ter::back},
          {"FAN1 curing", true, Ter::right},
          {"FAN1 drying", true, Ter::right},
          {"FAN2 curing", true, Ter::right},
          {"FAN2 drying", true, Ter::right},
        };
        menu_position = scrolling_list(items);

        break;
      }

    case LED_INTENSITY:

      generic_value("LED intensity", &LED_PWM_VALUE, 1, 100, "% ", 0);

      break;

    case FAN1_CURING:

      generic_value("FAN1 curing speed", &FAN1_CURING_SPEED, 0, 100, " %", 0);

      break;

    case FAN1_DRYING:

      generic_value("FAN1 drying speed", &FAN1_DRYING_SPEED, 0, 100, " %", 0);

      break;

    case FAN2_CURING:

      generic_value("FAN2 curing speed", &FAN2_CURING_SPEED, 0, 100, " %", 0);

      break;

    case FAN2_DRYING:

      generic_value("FAN2 drying speed", &FAN2_DRYING_SPEED, 0, 100, " %", 0);

      break;

    case INFO:
      {
        Serial_num_t sn;
        get_serial_num(sn);
        Scrolling_items items =
        {
          {"FW version: "  FW_VERSION, true, Ter::none},
          {"FAN1 failure", fan1_error, Ter::none},
          {"FAN2 failure", fan2_error, Ter::none},
          {"HEATER failure", heater_failure, Ter::none},
          {sn, true, Ter::none},
          {"Build: " FW_BUILDNR, true, Ter::none},
          {FW_HASH, true, Ter::none},
          {FW_LOCAL_CHANGES ? "Workspace dirty" : "Workspace clean", true, Ter::none}
        };
        menu_position = scrolling_list(items);

        break;
      }
    case RUN_MENU:
      if (!curing_mode && paused_time) {
        generic_menu(3, paused ? "IPA tank removed" : "Pause", "Stop", "Back");
      }
      else {
        generic_menu(3, paused ? "Continue" : "Pause", "Stop", "Back");
      }
      break;

    case RUNNING:
      lcd.setCursor(1, 0);
      if (curing_mode) {
        if (paused) {
          if (heat_to_target_temp || (curing_machine_mode == 3) || (preheat_complete == false)) {
            lcd.print(paused ? "Paused...          " : drying_mode ? "Heating" : "Curing");
          }
          else {
            lcd.print(paused ? "Paused...          " : drying_mode ? "Drying" : "Curing");
          }
        }
        else {
          if (heat_to_target_temp || (curing_machine_mode == 3)) {
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
      if (curing_mode && drying_mode && heat_to_target_temp && !preheat_complete) {
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
      speed_configuration();
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
    switch (curing_machine_mode) {
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
              remain = resin_preheat_run_time;
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
        if (!heat_to_target_temp) {
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
                remain = drying_run_time;
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
        if (!heat_to_target_temp) {
          if ((drying_mode == true) && (tDown.isCounterCompleted() == false)) {
            start_drying();
          }
          else {
            if (drying_mode) {
              drying_mode = false;
              remain = curing_run_time;
              running_count = 0;
              tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
              tDown.start();
              redraw_menu = true;
              menu_move(true);
            }
            if (tDown.isCounterCompleted() == false) {
              start_curing();
              fan1_duty = FAN1_CURING_SPEED;
              fan2_duty = FAN2_CURING_SPEED;
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
                remain = drying_run_time;
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
                remain = curing_run_time;
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
  if (sound_response) {
    echo();
  }
  switch (state) {
    case MENU:
      switch (menu_position) {
        case 0:

          if (curing_mode) { // curing_mode
            motor_configuration();
            speed_configuration();
            running_count = 0;

            switch (curing_machine_mode) {
              case 3: // Resin preheat
                pid_mode = true;
                remain = max_preheat_run_time;
                tUp.setCounter(0, remain, 0, tUp.COUNT_UP, tUpComplete);
                tUp.start();
                fan1_duty = FAN1_PREHEAT_SPEED;
                fan2_duty = FAN2_PREHEAT_SPEED;
                outputchip.digitalWrite(LED_RELE_PIN, LOW); //turn off LED
                digitalWrite(LED_PWM_PIN, LOW);
                drying_mode = true;
                preheat_complete = false;
                break;
              case 2: // Drying
                preheat_complete = false;
                drying_mode = true;
                if (!heat_to_target_temp) {
                  pid_mode = false;
                  remain = drying_run_time;
                  tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                  tDown.start();
                  fan1_duty = FAN1_DRYING_SPEED;
                  fan2_duty = FAN2_DRYING_SPEED;
                }
                else {
                  pid_mode = true;
                  remain = max_preheat_run_time;
                  tUp.setCounter(0, remain, 0, tUp.COUNT_UP, tUpComplete);
                  tUp.start();
                  fan1_duty = FAN1_PREHEAT_SPEED;
                  fan2_duty = FAN2_PREHEAT_SPEED;
                }

                outputchip.digitalWrite(LED_RELE_PIN, LOW); //turn off LED
                digitalWrite(LED_PWM_PIN, LOW);
                drying_mode = true;
                break;
              case 1: // Curing
                pid_mode = false;
                remain = curing_run_time;
                tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                tDown.start();

                fan1_duty = FAN1_CURING_SPEED;
                fan2_duty = FAN2_CURING_SPEED;
                drying_mode = false;

                break;
              case 0: // Drying and curing
              default:
                tDown.stop();
                tUp.stop();
                preheat_complete = false;
                drying_mode = true;
                if (!heat_to_target_temp) {
                  pid_mode = false;
                  remain = drying_run_time;
                  tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                  tDown.start();
                  fan1_duty = FAN1_DRYING_SPEED;
                  fan2_duty = FAN2_DRYING_SPEED;
                }
                else {
                  pid_mode = true;
                  remain = max_preheat_run_time;
                  tUp.setCounter(0, remain, 0, tUp.COUNT_UP, tUpComplete);
                  tUp.start();
                  fan1_duty = FAN1_PREHEAT_SPEED;
                  fan2_duty = FAN2_PREHEAT_SPEED;
                }

                break;
            }
          } else { // washing_mode
            drying_mode = false;
            motor_configuration();
            run_motor();
            speed_configuration();
            running_count = 0;
            remain = washing_run_time;
            tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
            tDown.start();
            fan1_duty = FAN1_WASHING_SPEED;
            fan2_duty = FAN2_WASHING_SPEED;
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
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = SETTINGS;
      break;

    case FAN1_CURING:
      menu_position = 1;
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = FANS;
      break;

    case FAN1_DRYING:
      menu_position = 2;
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = FANS;
      break;

    case FAN2_CURING:
      menu_position = 3;
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = FANS;
      break;

    case FAN2_DRYING:
      menu_position = 4;
      write_config(EEPROM.length() - EEPROM_OFFSET);
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
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = SPEED_STATE;
      break;

    case SPEED_WASHING:
      write_config(EEPROM.length() - EEPROM_OFFSET);
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
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 2;
      state = SOUND_SETTINGS;
      break;
    case SOUND:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 1;
      state = SOUND_SETTINGS;
      break;
    case TIME_CURING:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 1;
      state = TIME;
      break;
    case TIME_DRYING:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 2;
      state = TIME;
      break;
    case TIME_WASHING:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 3;
      state = TIME;
      break;
    case TIME_RESIN_PREHEAT:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 4;
      state = TIME;
      break;
    case INFO:
      menu_position = 7;
      state = SETTINGS;
      break;
    case RUN_MODE:
      write_config(EEPROM.length() - EEPROM_OFFSET);
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
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 1;
      state = PREHEAT;
      break;
    case TARGET_TEMP:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 2;
      state = PREHEAT;
      break;

    case RESIN_TARGET_TEMP:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      menu_position = 3;
      state = PREHEAT;
      break;
    case UNIT_SYSTEM:
      write_config(EEPROM.length() - EEPROM_OFFSET);
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
                fan1_duty = FAN1_MENU_SPEED;
                fan2_duty = FAN2_MENU_SPEED;
              } else {
                run_motor();
                speed_configuration();
                running_count = 0;

                if (!heat_to_target_temp) {
                  fan1_duty = FAN1_CURING_SPEED;
                  fan2_duty = FAN2_CURING_SPEED;
                }
                else {
                  fan1_duty = FAN1_PREHEAT_SPEED;
                  fan2_duty = FAN2_PREHEAT_SPEED;
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
                speed_configuration();
                running_count = 0;

                fan1_duty = FAN1_WASHING_SPEED;
                fan2_duty = FAN2_WASHING_SPEED;
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
          fan1_duty = FAN1_MENU_SPEED;
          fan2_duty = FAN2_MENU_SPEED;
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

    default:
      break;
  }
  scrolling_list_set(menu_position);

  redraw_menu = true;
  rotary_diff = 128;
  redraw_menu = true;
  menu_move(true);
  //delay(475);
}

ISR(TIMER3_COMPA_vect) { // timmer for stepper move

  if (motor_running == true) {
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

  if (heater_running) {
    fan_heater_rpm();
  }
  else
  {
    ams_counter = 0;
  }

  if (pid_mode) {
    if (curing_machine_mode == 0 || curing_machine_mode == 2) {
      if (chamber_temp_celsius >= target_temp_celsius) {
        actualTemp = chamber_temp_celsius;
        targetTemp = target_temp_celsius;
        PID();
        fan1_duty = newSpeed;
        fan2_duty = newSpeed;
      }
      else {
        fan1_duty = FAN1_MENU_SPEED;
        fan2_duty = FAN2_MENU_SPEED;
      }
    }
    if (curing_machine_mode == 3) {
      if (chamber_temp_celsius >= resin_target_temp_celsius) {
        actualTemp = chamber_temp_celsius;
        targetTemp = resin_target_temp_celsius;
        PID();
        fan1_duty = newSpeed;
        fan2_duty = newSpeed;
      }
      else {
        fan1_duty = FAN1_MENU_SPEED;
        fan2_duty = FAN2_MENU_SPEED;
      }
    }
  }

  fan_pwm_control();

  fan_rpm();

  if (ms_counter >= 4000) {

    //lcd.begin_noclear(20, 4);
    //redraw_menu = true;
    //menu_move(true);
    ms_counter = 0;
  }
  else {
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
    if (heat_to_target_temp || (curing_machine_mode == 3)) {
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
    if (heat_to_target_temp || (curing_machine_mode == 3)) {
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
    if (heat_to_target_temp || (curing_machine_mode == 3)) {
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
  if (heat_to_target_temp || (curing_machine_mode == 3))
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
      switch (finish_beep_mode) {
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
      fan1_duty = FAN1_MENU_SPEED;
      fan2_duty = FAN2_MENU_SPEED;
      stop_heater(); // turn off heat fan
      redraw_menu = true;
      rotary_diff = 128;
      switch (finish_beep_mode) {
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
  fan1_duty = FAN1_MENU_SPEED;
  fan2_duty = FAN2_MENU_SPEED;
  redraw_menu = true;
  rotary_diff = 128;
  switch (finish_beep_mode) {
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
  if (heat_to_target_temp || (curing_machine_mode == 3)) {
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
          if (SI_unit_system) {
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
        lcd.print(" ");
        lcd.setCursor(dots_column + 1, 0);
        lcd.print(" ");
        lcd.setCursor(dots_column + 2, 0);
        lcd.print(" ");
      }
      if (running_count == 1) {
        lcd.setCursor(dots_column, 0);
        lcd.print(".");
        lcd.setCursor(dots_column + 1, 0);
        lcd.print(" ");
        lcd.setCursor(dots_column + 2, 0);
        lcd.print(" ");
      }
      if (running_count == 2) {
        lcd.setCursor(dots_column, 0);
        lcd.print(".");
        lcd.setCursor(dots_column + 1, 0);
        lcd.print(".");
        lcd.setCursor(dots_column + 2, 0);
        lcd.print(" ");
        lcd.setCursor(14, 2);
        lcd.print("  ");
        lcd.setCursor(5, 2);
        lcd.print("  ");
      }
      if (running_count == 3) {
        lcd.setCursor(dots_column, 0);
        lcd.print(".");
        lcd.setCursor(dots_column + 1, 0);
        lcd.print(".");
        lcd.setCursor(dots_column + 2, 0);
        lcd.print(".");
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
  if (fan1_duty > 0) {
    if (fan1_pwm_State == LOW) {
      if (currentMillis - fan1_previous_millis >= ((period) * (1 - ((float)fan1_duty / 100)))) {
        fan1_previous_millis = currentMillis;
        PORTC = PORTC | 0x80; //OUTPUT FAN1 HIGH
        fan1_pwm_State = HIGH;
      }
    }
    if (fan1_pwm_State == HIGH) {
      if (currentMillis - fan1_previous_millis >= ((period) * ((float)fan1_duty / 100))) {
        fan1_previous_millis = currentMillis;
        PORTC = PORTC & 0x7F; //OUTPUT FAN1 LOW
        fan1_pwm_State = LOW;
      }
    }
  }
  else {
    PORTC = PORTC & 0x7F; //OUTPUT FAN1 LOW
  }
  if (fan2_duty > 0) {
    if (fan2_pwm_State == LOW) {
      if (currentMillis - fan2_previous_millis >= ((period) * (1 - ((float)fan2_duty / 100)))) {
        fan2_previous_millis = currentMillis;
        PORTB = PORTB | 0x80; //OUTPUT FAN2 HIGH
        fan2_pwm_State = HIGH;
      }
    }
    if (fan2_pwm_State == HIGH) {
      if (currentMillis - fan2_previous_millis >= ((period) * ((float)fan2_duty / 100))) {
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
  if (fan1_duty > 0) {

    if (!fan1_on) {
      fan1_on = true;

      outputchip.digitalWrite(FAN1_PIN, HIGH);
    }

    if (fan1_pwm_State == LOW) {
      if (currentMillis - fan1_previous_millis >= ((period) * (1 - ((float)fan1_duty / 100)))) {
        fan1_previous_millis = currentMillis;
        PORTC = PORTC & 0x7F; //OUTPUT FAN1 LOW
        fan1_pwm_State = HIGH;
      }
    }
    if (fan1_pwm_State == HIGH) {
      if (currentMillis - fan1_previous_millis >= ((period) * ((float)fan1_duty / 100))) {
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
  if (fan2_duty > 0) {
    if (!fan2_on) {
      fan2_on = true;
      outputchip.digitalWrite(FAN2_PIN, HIGH);
    }
    if (fan2_pwm_State == LOW) {
      if (currentMillis - fan2_previous_millis >= ((period) * (1 - ((float)fan2_duty / 100)))) {
        fan2_previous_millis = currentMillis;
        PORTB = PORTB & 0x7F; //OUTPUT FAN2 LOW
        fan2_pwm_State = HIGH;

      }
    }
    if (fan2_pwm_State == HIGH) {
      if (currentMillis - fan2_previous_millis >= ((period) * ((float)fan2_duty / 100))) {
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

void fan_heater_rpm() {
  ams_counter ++;
  if (ams_counter >= 1000) {

    if (fan3_tacho_count <= fan3_tacho_last_count ) {
      if (heater_running) {
        heater_error = true;
        heater_failure = true;
      }
    }
    else {
      heater_error = false;
      heater_failure = false;
    }
    write_config(EEPROM.length() - EEPROM_OFFSET);
    fan3_tacho_last_count = fan3_tacho_count;

    ams_counter = 0;
    if (fan3_tacho_count >= 10000) {
      fan3_tacho_count = 0;
      fan3_tacho_last_count = 0;
    }
  }
}

void fan_rpm() {
  ams_fan_counter ++;
  if (ams_fan_counter >= 100) {

    if (fan1_tacho_count <= fan1_tacho_last_count ) {
      if (fan1_duty > 0) {
        fan1_error = true;
      }
    }
    else {
      fan1_error = false;
    }
    if (fan2_tacho_count <= fan2_tacho_last_count ) {
      if (fan2_duty > 0) {
        fan2_error = true;
      }
    }
    else {
      fan2_error = false;
    }
    fan1_tacho_last_count = fan1_tacho_count;
    fan2_tacho_last_count = fan2_tacho_count;

    ams_fan_counter = 0;
    if (fan1_tacho_count >= 10000) {
      fan1_tacho_count = 0;
      fan1_tacho_last_count = 0;
    }
    if (fan2_tacho_count >= 10000) {
      fan2_tacho_count = 0;
      fan2_tacho_last_count = 0;
    }
  }
}

void fan_tacho1() {
  fan1_tacho_count++;
}
void fan_tacho2() {
  fan2_tacho_count++;
}
void fan_tacho3() {
  fan3_tacho_count++;
}

void preheat() {
  if (curing_machine_mode == 0 || curing_machine_mode == 2) {
    if (SI_unit_system) {
      if (chamber_temp_celsius < target_temp_celsius)
      {
        run_heater();
      }
      else {
        stop_heater();
        tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
      }
    }
    else {
      if (chamber_temp_fahrenheit < ((target_temp_celsius * 1.8) + 32))
      {
        run_heater();
      }
      else {
        stop_heater();
        tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
      }
    }
  }
  if (curing_machine_mode == 3) {
    if (SI_unit_system) {
      if (chamber_temp_celsius < resin_target_temp_celsius)
      {
        run_heater();
      }
      else {
        stop_heater();
        tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
      }
    }
    else {
      if (chamber_temp_fahrenheit < ((resin_target_temp_celsius * 1.8) + 32))
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

//! @brief Get serial number
//! @param[out] sn null terminated string containing serial number
void get_serial_num(Serial_num_t &sn)
{
  uint16_t snAddress = 0x7fe0;
  sn[0] = 'S';
  sn[1] = 'N';
  sn[2] = ':';
  for (uint_least8_t i = 3; i < 19; ++i)
  {
    sn[i] = pgm_read_byte(snAddress++);
  }

  sn[sizeof(Serial_num_t) - 1] = 0;
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
