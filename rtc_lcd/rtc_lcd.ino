// Table clock

/*
  Status: testing
  Last mod.: 2021-01-01
  Version: 2.0.0
*/

/*
  Built on:
    16x2 LCD display
    DS3231 RTC
    Arduino Uno

  It sets DS3231 to emit square wave at 1Hz, connects output to
  interrupt pin and use it to update display.

  Wiring.
    Connect LCD to <LCD_..> pins.
    Connect DS3231.BBSQW to <TICK_PIN> pin which accepts interrupts.
*/

#include <LiquidCrystal.h>

#include <me_ds3231.h>
#include <me_DateTime.h>

const uint8_t
  LCD_RS = 12,
  LCD_EN = 11,
  LCD_D4 = 7,
  LCD_D5 = 6,
  LCD_D6 = 5,
  LCD_D7 = 4,

  TICK_PIN = 2;

const uint8_t
  MAX_MSG_LEN = 2 * 16;
char msg_buf[MAX_MSG_LEN];

LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);
me_ds3231 ds3231 = me_ds3231();

void announce(char* msg) {
  Serial.print("  ");
  Serial.println(msg);
}

void init_lcd() {
  announce("LCD");
  lcd.begin(16, 2);
}

void init_clock() {
  announce("Clock");

  // SQW mode 0 - 1Hz square wave
  ds3231.setSqwMode(0);
  ds3231.disableSqwAtBattery();

  if (ds3231.wave32kEnabled()) {
    ds3231.disable32kWave();
    Serial.println("32kHz wave disabled.");
  }

  if (ds3231.oscillatorWasStopped()) {
    Serial.println("Clock was stopped. Battery is over?");
  }

  if (!ds3231.isOscillatorAtBattery()) {
    ds3231.enableOscillatorAtBattery();
    Serial.println("Enabled oscillator at battery.");
  }

  if (ds3231.getSqwMode() != 0) {
    ds3231.setSqwMode(0);
    Serial.println("Set 1Hz square wave.");
  }

  if (!ds3231.isSqw()) {
    ds3231.emitSqwNoAlarm();
    Serial.println("Alarm pin now emits square wave.");
  }

  pinMode(TICK_PIN, INPUT_PULLUP);
  uint8_t internal_pin_no = digitalPinToInterrupt(TICK_PIN);
  attachInterrupt(internal_pin_no, tick_handler, RISING);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");

  init_lcd();
  init_clock();

  Serial.println("Initialization done.");
}

void do_business() {
  // Serial.println("*click*");

  DateTime dt = ds3231.getDateTime();
  float temperature = ds3231.getTemp();

  dt.represent_date(msg_buf, MAX_MSG_LEN);
  lcd.setCursor(0, 0);
  lcd.print(msg_buf);

  dt.represent_dow(msg_buf, MAX_MSG_LEN);
  lcd.setCursor(11, 0);
  lcd.print(msg_buf);

  dt.represent_time(msg_buf, MAX_MSG_LEN);
  lcd.setCursor(0, 1);
  lcd.print(msg_buf);

  lcd.setCursor(9, 1);
  lcd.print(temperature, 2);
  lcd.print("\337C");
}

volatile bool tick_registered;

void loop() {
  if (tick_registered) {
    do_business();
    tick_registered = false;
  }
  // delay(1000);
  // tick_registered = true;
}

void tick_handler() {
  tick_registered = true;
}
