#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "Trinamic_TMC2130.h"
#include "MCP23S17.h"
#include "config.h"
#include "Countimer.h"
#include "thermistor.h"

Countimer tDown;
Countimer tUp;

thermistor therm1(A4, 5);

Trinamic_TMC2130 myStepper(CS_PIN);

MCP outputchip(0, 8);

LiquidCrystal lcd(LCD_PINS_RS, LCD_PINS_ENABLE, LCD_PINS_D4, LCD_PINS_D5, LCD_PINS_D6, LCD_PINS_D7);

enum menu_state {
  MENU,
  SPEED_STATE,
  SPEED_CURING,
  SPEED_WASHING,
  TIME,
  TIME_CURING,
  TIME_DRYING,
  TIME_WASHING,
  SETTINGS,
  DRYING_CURING,
  PREHEAT,
  PREHEAT_ENABLE,
  TARGET_TEMP,
  UNIT_SYSTEM,
  RUN_MODE,
  SOUND_SETTINGS,
  SOUND,
  RUNNING,
  RUN_MENU,
  BEEP,
  INFO,
  CONFIRM,
  ERROR
};

String FW_VERSION = "2.0.7";

menu_state state = MENU;

long lastJob = 0;

byte menu_position = 0;
byte max_menu_position = 0;
bool redraw_menu = true;
bool redraw_ms = true;
bool speed_up = false;
bool pinda_therm = 0; // 0 - 100K thermistor - ATC Semitec 104GT-2/ 1 - PINDA thermistor

bool button_released = false;
volatile byte rotary_diff = 128;

byte washing_speed = 10;
byte curing_speed = 1;
int max_washing_speed = 16; //15 //smaller = faster
int min_washing_speed = 70; //100 //smaller = faster
int max_curing_speed = 25; //50 //smaller = faster
int min_curing_speed = 220; //smaller = faster
int set_washing_speed;
int set_curing_speed;

byte washing_run_time = 4;
byte curing_run_time = 3;
byte drying_run_time = 3;
byte max_preheat_run_time = 30;

byte finish_beep_mode = 1;
byte sound_response = 1;
byte heat_to_target_temp = 0;
byte target_temp_celsius = 35;
byte target_temp_fahrenheit = 95;
byte cover_check_enabled = 1;
byte curing_machine_mode;
byte SI_unit_system = 1;

int fan1_pwm_State = LOW;
int fan2_pwm_State = LOW;

long fan1_tacho_count;
long fan2_tacho_count;
long fan3_tacho_count;

bool fan1_on = false;
bool fan2_on = false;

long fan3_tacho_last_count;

unsigned long fan1_previous_millis = 0;
unsigned long fan2_previous_millis = 0;

bool heater_error = false;

// constants won't change:
int fan_frequency = 70; //Hz
float period = (1 / (float)fan_frequency) * 1000; //
int fan1_duty; //%
int fan2_duty; //%

//fan1_duty = 0-100%
int FAN1_MENU_SPEED = 30;//20;
int FAN1_CURING_SPEED = 80;//70
int FAN1_WASHING_SPEED = 80;//70
int FAN1_DRYING_SPEED = 80;//70
int FAN1_PREHEAT_SPEED = 40;//70

//fan2_duty = 0-100%
int FAN2_MENU_SPEED = 30;//20;
int FAN2_CURING_SPEED = 80;//30
int FAN2_WASHING_SPEED = 80;//70
int FAN2_DRYING_SPEED = 80;//70
int FAN2_PREHEAT_SPEED = 40;//70

long remain = 0;
unsigned long us_last = 0;
unsigned long last_remain = 0;
unsigned long last_millis = 0;
unsigned int last_seconds = 0;
bool motor_running = false;
bool heater_running = false;
bool curing_mode = false;
bool drying_mode = false;
bool last_curing_mode = false;
bool paused = false;
bool cover_open = false;
bool gastro_pan = false;
bool paused_time = false;
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


#define EEPROM_OFFSET 128
#define MAGIC_SIZE 6
const char magic[MAGIC_SIZE] = "CURWA";

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

}

void stop_heater() {

  outputchip.digitalWrite(FAN_HEAT_PIN, LOW); // disable driver
  heater_running = false;

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

  pinMode(FAN1_PIN, OUTPUT);
  pinMode(FAN2_PIN, OUTPUT);

  //pinMode(1, INPUT_PULLUP);
  //pinMode(2, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);

  //attachInterrupt(2, fan_tacho1, RISING);
  //attachInterrupt(1, fan_tacho2, RISING);
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
}


void write_config(unsigned int address) {
  EEPROM.put(address, magic);
  address += MAGIC_SIZE;
  EEPROM.put(address++, washing_speed);
  EEPROM.put(address++, curing_speed);
  EEPROM.put(address++, washing_run_time);
  EEPROM.put(address++, curing_run_time);
  EEPROM.put(address++, finish_beep_mode);
  EEPROM.put(address++, drying_run_time);
  EEPROM.put(address++, sound_response);
  EEPROM.put(address++, curing_machine_mode);
  EEPROM.put(address++, heat_to_target_temp);
  EEPROM.put(address++, target_temp_celsius);
  EEPROM.put(address++, target_temp_fahrenheit);
  EEPROM.put(address++, SI_unit_system);
}


void read_config(unsigned int address) {
  char test_magic[MAGIC_SIZE];
  EEPROM.get(address, test_magic);
  address += MAGIC_SIZE;
  if (!strncmp(magic, test_magic, MAGIC_SIZE)) {
    EEPROM.get(address++, washing_speed);
    EEPROM.get(address++, curing_speed);
    EEPROM.get(address++, washing_run_time);
    EEPROM.get(address++, curing_run_time);
    EEPROM.get(address++, finish_beep_mode);
    EEPROM.get(address++, drying_run_time);
    EEPROM.get(address++, sound_response);
    EEPROM.get(address++, curing_machine_mode);
    EEPROM.get(address++, heat_to_target_temp);
    EEPROM.get(address++, target_temp_celsius);
    EEPROM.get(address++, target_temp_fahrenheit);
    EEPROM.get(address++, SI_unit_system);
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

  lcd.setCursor(0, menu_position);
  lcd.print(">");

  for (int i = 0; i <= 3; i++) {
    if ( i != menu_position) {
      lcd.setCursor(0, i);
      lcd.print(" ");
    }
  }
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
  if (rotary_diff <= 124 || rotary_diff >= 132 || redraw_menu) {
    menu_move();
  }

  if (state == RUNNING || state == RUN_MENU) {
    machine_running();
  }

  if (outputchip.digitalRead(BTN_ENC) == HIGH) { // released button
    button_released = true;
  }

  if (outputchip.digitalRead(BTN_ENC) == LOW && button_released) { // button presset

    button_released = false;

    if (!heater_error) {
      button_press();
    }
  }
}

void menu_move() {
  if (!redraw_menu) {
    if (sound_response) {
      echo();
    }
  }
  else {
    lcd.clear();
  }

  redraw_menu = false;

  switch (state) {
    case MENU:

      switch (curing_machine_mode) {
        case 2:
          generic_menu(4, curing_mode ? "Start drying" : "Start washing", "Run-time", "Rotation speed", "Settings");
          state = MENU;
          break;
        case 1:
          generic_menu(4, curing_mode ? "Start curing" : "Start washing", "Run-time", "Rotation speed", "Settings");
          state = MENU;
          break;
        case 0:
        default:
          generic_menu(4, curing_mode ? "Start drying/curing" : "Start washing", "Run-time", "Rotation speed", "Settings");
          state = MENU;
          break;
      }

      break;

    case SPEED_STATE:

      generic_menu(3, "Curing speed", "Washing speed", "Back");

      break;

    case SPEED_CURING:

      generic_value("Curing speed", &curing_speed, 1, 10, "/10", 0);

      break;

    case SPEED_WASHING:

      generic_value("Washing speed", &washing_speed, 1, 10, "/10", 0);

      break;

    case TIME:

      generic_menu(4, "Curing run-time", "Drying run-time", "Washing run-time", "Back");

      break;

    case TIME_CURING:

      generic_value("Curing run-time", &curing_run_time, 1, 10, " min", 0);

      break;

    case TIME_DRYING:

      generic_value("Drying run-time", &drying_run_time, 1, 10, " min", 0);

      break;

    case TIME_WASHING:

      generic_value("Washing run-time", &washing_run_time, 1, 10, " min", 0);

      break;

    case SETTINGS:
      generic_menu(4, "Drying & Curing", "Sound settings", "Information", "Back");
      break;

    case DRYING_CURING:
      generic_menu(4, "Run mode", "Preheat", "Unit system", "Back");
      break;

    case PREHEAT:
      if (heat_to_target_temp) {
        generic_menu(3, "Preheat enable", "Target temp", "Back");
      }
      else {
        generic_menu(3, "Preheat disable", "Target temp", "Back");
      }
      break;

    case PREHEAT_ENABLE:

      generic_items("Preheat enable", &heat_to_target_temp, 2, "disable", "enable");

      break;

    case TARGET_TEMP:
      if (SI_unit_system) {
        generic_value("Target temp", &target_temp_celsius, 30, 50, "\xDF" "C", 0);
      }
      else {
        generic_value("Target temp", &target_temp_celsius, 30, 50, "\xDF" "F", 1);
      }
      break;

    case UNIT_SYSTEM:
      generic_items("Unit system", &SI_unit_system, 2, "IMPERIAL/ US", "SI");
      break;

    case RUN_MODE:
      generic_items("Run mode", &curing_machine_mode, 3, "Drying & Curing", "Curing", "Drying");
      break;

    case SOUND_SETTINGS:
      generic_menu(3, "Sound response", "Finish beep", "Back");
      break;

    case SOUND:
      generic_items("Sound response", &sound_response, 2, "no", "yes");
      break;

    case BEEP:
      generic_items("Finish beep", &finish_beep_mode, 3, "none", "once", "continuous");
      break;

    case INFO:
      lcd.setCursor(1, 0);
      lcd.print("FW version: ");
      lcd.setCursor(13, 0);
      lcd.print(FW_VERSION);

      break;

    case RUN_MENU:
      generic_menu(3, paused ? "Continue" : "Pause", "Stop", "Back");
      break;

    case RUNNING:
      lcd.setCursor(1, 0);
      if (curing_mode) {
        lcd.print(cover_open ? "The cover is open!" : (paused ? "Paused..." : drying_mode ? "Drying" : "Curing"));
      }
      else {
        lcd.print(cover_open ? "Open cover!" : (paused ? "Paused..." : "Washing"));
      }
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
        lcd.setCursor(1, 0);
        lcd.print("The cover is open! ");
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
      paused_time = false;
      us_last = us_now;
    }
    switch (curing_machine_mode) {
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
          if (tUp.isCounterCompleted() == false) {
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
            }
            if (tDown.isCounterCompleted() == false) {
              start_curing();
            }
            else {
              stop_curing_drying ();
            }
          }
        }
        else {
          if ((drying_mode == true) && (tUp.isCounterCompleted() == false)) {
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
            }
            if (tDown.isCounterCompleted() == false) {
              start_curing();
            }
            else {
              stop_curing_drying ();
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
              case 2: // Drying

                if (!heat_to_target_temp) {
                  remain = drying_run_time;
                  tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                  tDown.start();
                  fan1_duty = FAN1_CURING_SPEED;
                  fan2_duty = FAN2_CURING_SPEED;
                }
                else {
                  remain = max_preheat_run_time;
                  tUp.setCounter(0, remain, 0, tUp.COUNT_UP, tUpComplete);
                  tUp.start();
                  fan1_duty = FAN1_PREHEAT_SPEED;
                  fan2_duty = FAN2_PREHEAT_SPEED;
                }

                outputchip.digitalWrite(LED_RELE_PIN, LOW); //turn off LED
                digitalWrite(LED_PWM_PIN, LOW);
                break;
              case 1: // Curing
                remain = curing_run_time;
                tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                tDown.start();

                fan1_duty = FAN1_CURING_SPEED;
                fan2_duty = FAN2_CURING_SPEED;

                break;
              case 0: // Drying and curing
              default:
                drying_mode = true;
                if (!heat_to_target_temp) {
                  remain = drying_run_time;
                  tDown.setCounter(0, remain, 0, tDown.COUNT_DOWN, tDownComplete);
                  tDown.start();
                  fan1_duty = FAN1_CURING_SPEED;
                  fan2_duty = FAN2_CURING_SPEED;
                }
                else {
                  remain = max_preheat_run_time;
                  tUp.setCounter(0, remain, 0, tUp.COUNT_UP, tUpComplete);
                  tUp.start();
                  fan1_duty = FAN1_PREHEAT_SPEED;
                  fan2_duty = FAN2_PREHEAT_SPEED;
                }

                break;
            }
          } else { // washing_mode
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

          state = RUNNING;
          break;

        case 1:
          state = TIME;
          break;

        case 2:
          state = SPEED_STATE;
          break;

        case 3:
          state = SETTINGS;
          break;

        default:
          break;
      }
      break;

    case SETTINGS:
      switch (menu_position) {
        case 0:
          state = DRYING_CURING;
          break;

        case 1:
          state = SOUND_SETTINGS;
          break;

        case 2:
          state = INFO;
          break;

        default:
          state = MENU;
          break;
      }
      break;

    case SOUND_SETTINGS:
      switch (menu_position) {
        case 0:
          state = SOUND;
          break;

        case 1:
          state = BEEP;
          break;

        default:
          state = SETTINGS;
          break;
      }
      break;

    case DRYING_CURING:
      switch (menu_position) {
        case 0:
          state = RUN_MODE;
          break;

        case 1:
          state = PREHEAT;
          break;

        case 2:
          state = UNIT_SYSTEM;
          break;

        default:
          state = SETTINGS;
          break;
      }
      break;

    case PREHEAT:
      switch (menu_position) {
        case 0:
          state = PREHEAT_ENABLE;
          break;

        case 1:
          state = TARGET_TEMP;
          break;

        default:
          state = DRYING_CURING;
          break;
      }
      break;

    case SPEED_STATE:
      switch (menu_position) {
        case 0:
          state = SPEED_CURING;
          break;

        case 1:
          state = SPEED_WASHING;
          break;

        default:
          state = MENU;
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
          state = TIME_CURING;
          break;

        case 1:
          state = TIME_DRYING;
          break;

        case 2:
          state = TIME_WASHING;
          break;

        default:
          state = MENU;
          break;
      }
      break;
    case BEEP:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = SOUND_SETTINGS;
      break;
    case SOUND:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = SOUND_SETTINGS;
      break;
    case TIME_CURING:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = TIME;
      break;
    case TIME_DRYING:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = TIME;
      break;
    case TIME_WASHING:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = TIME;
      break;
    case INFO:
      state = SETTINGS;
      break;
    case RUN_MODE:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = DRYING_CURING;
      break;
    case PREHEAT_ENABLE:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = PREHEAT;
      break;
    case TARGET_TEMP:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = PREHEAT;
      break;
    case UNIT_SYSTEM:
      write_config(EEPROM.length() - EEPROM_OFFSET);
      state = DRYING_CURING;
      break;

    case CONFIRM:
      state = MENU;
      break;

    case RUN_MENU:
      switch (menu_position) {
        case 0:
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

            if (curing_mode) {
              if (!heat_to_target_temp) {
              fan1_duty = FAN1_CURING_SPEED;
              fan2_duty = FAN2_CURING_SPEED;
            }
            else{
              fan1_duty = FAN1_PREHEAT_SPEED;
                  fan2_duty = FAN2_PREHEAT_SPEED;
            }
            }
            else {
              fan1_duty = FAN1_WASHING_SPEED;
              fan2_duty = FAN2_WASHING_SPEED;
            }
            state = RUNNING;
          }
          break;

        case 1:
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
          break;

        case 2:
          running_count = 0;
          state = RUNNING;
          break;

        default:
          break;
      }
      break;

    case RUNNING:
      state = RUN_MENU;
      break;

    default:
      break;
  }
  menu_position = 0;
  redraw_menu = true;
  rotary_diff = 128;
  delay(475);
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

  if (!heater_error) {
    read_encoder();
  }

  fan_pwm_control();
  if (heater_running) {
    fan_heater_rpm();
  }
  else
  {
    ams_counter = 0;
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
    if (heat_to_target_temp) {
      preheat(); // turn on heat fan
    }
    else {
      run_heater(); // turn on heat fan
    }
  }
  if (cover_open == true || paused == true) {
    if (!heat_to_target_temp) {
      tDown.pause();
    }
    else {
      tUp.pause();
    }
  }
  else {
    if (!heat_to_target_temp) {
      tDown.start();
    }
    else {
      tUp.start();
    }
  }
  lcd_time_print();
}

void start_curing() {

  stop_heater(); // turn off heat fan
  if (cover_open == false && paused == false) {
    outputchip.digitalWrite(LED_RELE_PIN, HIGH); // turn LED on
    digitalWrite(LED_PWM_PIN, HIGH);
  }
  else {
    outputchip.digitalWrite(LED_RELE_PIN, LOW); // turn LED off
    digitalWrite(LED_PWM_PIN, LOW);
  }
  if (cover_open == true || paused == true) {
    tDown.pause();
  }
  else {
    tDown.start();
  }
  lcd_time_print();
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
      //cover_open = true;
      if (!cover_open) {
        if (sound_response) {
          warning_beep();
        }
        redraw_menu = true;
        cover_open = true;
      }
    }
    else {
      if (cover_open) {
        redraw_menu = true;
        cover_open = false;
      }
      if (outputchip.digitalRead(WASH_DETECT_PIN) == LOW) { //Gastro Pen check
        lcd.setCursor(1, 0);
        lcd.print("Gastro Pen is out");
        running_count = 0;
        paused = true;
        tDown.pause();
        stop_motor();
        gastro_pan = true;
      }
      else {
        if (gastro_pan) {
          redraw_menu = true;
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

        lcd_time_print();
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
      lcd.print("Gastro Pen is out");
      running_count = 0;
      paused = true;
      tDown.pause();
      stop_motor();
      gastro_pan = true;
    }
    else {
      if (gastro_pan) {
        redraw_menu = true;
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

        lcd_time_print();
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
}

void stop_curing_drying() {
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
}

void lcd_time_print() {
  byte mins;
  byte secs;
  if (!heat_to_target_temp) {
    mins = tDown.getCurrentMinutes();
    secs = tDown.getCurrentSeconds();
  }
  else {
    if (drying_mode) {
      mins = tUp.getCurrentMinutes();
      secs = tUp.getCurrentSeconds();
    }
    else {
      mins = tDown.getCurrentMinutes();
      secs = tDown.getCurrentSeconds();
    }
  }

  if (state == RUNNING && (secs != last_seconds || redraw_ms)) {
    therm_read != therm_read;

    therm1_read();
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
    if (!paused) {
      if (curing_mode) { //curing or drying mode
        if (running_count > 1) {
          lcd.setCursor(5 + running_count, 0);
          lcd.print(".");
          lcd.setCursor(14, 2);
          lcd.print("  ");
          lcd.setCursor(5, 2);
          lcd.print("  ");
        }
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
      else { //washing mode
        if (running_count > 1) {
          lcd.setCursor(6 + running_count, 0);
          lcd.print(".");
          lcd.setCursor(14, 2);
          lcd.print("  ");
          lcd.setCursor(5, 2);
          lcd.print("  ");
        }
      }
      if (running_count > 4) {
        running_count = 0;
        redraw_menu = true;
      }
      running_count++;
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

    if (fan3_tacho_count <= fan3_tacho_last_count )
      if (heater_running) {
        heater_error = true;
      }
    fan3_tacho_last_count = fan3_tacho_count;

    ams_counter = 0;
    if (fan3_tacho_count >= 10000) {
      fan3_tacho_count = 0;
      fan3_tacho_last_count = 0;
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
  if (SI_unit_system) {
    if (chamber_temp_celsius <= target_temp_celsius)
    {
      run_heater();
    }
    else {
      stop_heater();
      tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
    }
  }
  else {
    if (chamber_temp_fahrenheit <= ((target_temp_celsius * 1.8) + 32))
    {
      run_heater();
    }
    else {
      stop_heater();
      tUp.setCounter(0, 0, 0, tUp.COUNT_UP, tUpComplete);
    }
  }
}
