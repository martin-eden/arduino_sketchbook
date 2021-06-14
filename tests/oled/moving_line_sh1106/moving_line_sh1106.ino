// Graphic primitives playground, controlled by rotary encoder

/*
  Status: works
  Version:
  Last mod.: 2021-05-07
*/

#include <U8g2lib.h>
#include <Wire.h>

#include <me_RotaryEncoder.h>

const uint8_t
  PhaseA_pin = 2,
  PhaseB_pin = 3,
  Switch_pin = 4;

#define Display_cs_pin 8
#define Display_dc_pin 9
#define Display_reset_pin 10
typedef u8g2_uint_t u8g_uint_t;

const int32_t
  Debounce_ms = 40;

RotaryEncoder RotaryEncoder(PhaseA_pin, PhaseB_pin, Switch_pin);
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI Display(U8G2_R0, Display_cs_pin, Display_dc_pin, Display_reset_pin);

void setup() {
  Serial.begin(9600);
  Serial.println("Test: rotary encoder, monochrome OLED display.");

  RotaryEncoder.Debounce_ms = Debounce_ms;

  attachInterrupt(digitalPinToInterrupt(PhaseA_pin), Phase_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PhaseB_pin), Phase_change, CHANGE);

  Display.begin();

  resetPosition();
  drawState_force(getPosition(), 0);
}

void Phase_change() {
  RotaryEncoder.UpdateState();
}

void resetPosition() {
  cli();
  RotaryEncoder.Position = 20;
  sei();
}

void drawState_force(int32_t Position_copy, int32_t PrevPosition_printed) {
  u8g2_uint_t x_prev = PrevPosition_printed / 4;
  u8g2_uint_t x = Position_copy / 4;

  if (x != x_prev) {
    Display.firstPage();
    do {
      //*
      Display.setColorIndex(0);
      Display.drawVLine(x_prev, 0, Display.getHeight());
      Display.setColorIndex(1);
      Display.drawVLine(x, 0, Display.getHeight());
      //*/
      //*
      Display.setColorIndex(0);
      Display.drawHLine(0, x_prev, Display.getWidth());
      Display.setColorIndex(1);
      Display.drawHLine(0, x, Display.getWidth());
      //*/
    } while (Display.nextPage());
  }
}

int32_t getPosition() {
  int32_t result;
  cli();
  result = RotaryEncoder.Position;
  sei();
  return result;
}

void drawState() {
  if (RotaryEncoder.PositionHasChanged)
    RotaryEncoder.PositionHasChanged = false;

  static int32_t PrevPosition_printed = -1;
  int32_t Position_copy = getPosition();
  if (Position_copy != PrevPosition_printed) {
    drawState_force(Position_copy, PrevPosition_printed);
    PrevPosition_printed = Position_copy;
  }
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
