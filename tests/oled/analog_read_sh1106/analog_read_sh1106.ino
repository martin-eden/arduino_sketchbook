// Graphic primitives playground, controlled by rotary encoder

/*
  Status: works
  Version:
  Last mod.: 2021-05-16
*/

#include <U8g2lib.h>
#include <Wire.h>

#include <me_RotaryEncoder.h>

const uint8_t
  PhaseA_pin = 2,
  PhaseB_pin = 3,
  Switch_pin = 4,
  Sound_pin = 7;

#define Display_cs_pin 8
#define Display_dc_pin 9
#define Display_reset_pin 10
typedef u8g2_uint_t u8g_uint_t;

const int32_t
  Debounce_ms = 40;

RotaryEncoder RotaryEncoder(PhaseA_pin, PhaseB_pin, Switch_pin);
U8G2_SH1106_128X64_NONAME_F_4W_HW_SPI Display(U8G2_R0, Display_cs_pin, Display_dc_pin, Display_reset_pin);

void setup() {
  Serial.begin(9600);
  Serial.println("Test: rotary encoder, monochrome OLED display.");

  RotaryEncoder.Debounce_ms = Debounce_ms;

  pinMode(Sound_pin, OUTPUT);
  digitalWrite(Sound_pin, LOW);

  attachInterrupt(digitalPinToInterrupt(PhaseA_pin), Phase_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PhaseB_pin), Phase_change, CHANGE);

  Display.begin();
  Display.setColorIndex(1);

  resetPosition();
}

void Phase_change() {
  RotaryEncoder.UpdateState();
}

void resetPosition() {
  cli();
  RotaryEncoder.Position = 20;
  sei();
}

int32_t getPosition() {
  int32_t result;
  cli();
  result = RotaryEncoder.Position;
  sei();
  return result;
}

void drawState() {
  static u8g2_uint_t width = Display.getWidth();
  static u8g2_uint_t height = Display.getHeight();
  static u8g2_uint_t x = 0;
  static u8g2_uint_t y = 0;
  static uint8_t SoundState = 0;

  y = map(analogRead(A0), 0, 1023, 0, height - 1);

  // Display.firstPage();
  // do {
    // Display.setColorIndex(0);
    // Display.drawHLine(x, y, (width - x));

    // Display.setColorIndex(1);
    Display.drawPixel(x, y);
    Display.updateDisplay();
  // } while (Display.nextPage());

  x = (x + 1) % width;
  // y = (y + 1) % height;

  digitalWrite(Sound_pin, SoundState);
  SoundState = (SoundState + 1) % 2;
}

bool PrevSwitchState_handled = false;

void loop() {
  drawState();

  RotaryEncoder.UpdateSwitch();
  if (RotaryEncoder.SwitchHasChanged) {
    bool NeedSwitchRedisplay =
      (RotaryEncoder.SwitchState != PrevSwitchState_handled) &&
      RotaryEncoder.SwitchState;
    if (NeedSwitchRedisplay) {
      Display.clear();
      resetPosition();
      drawState();
    }
    PrevSwitchState_handled = RotaryEncoder.SwitchState;
    RotaryEncoder.SwitchHasChanged = false;
  }
}
