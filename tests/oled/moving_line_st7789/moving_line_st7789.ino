// Graphic primitives playground, controlled by rotary encoder

/*
  Status: works
  Version:
  Last mod.: 2021-07-10
*/

/*
  Wiring
  
  GND
  VCC 5V
  SCL 13
  SDA 11
  RES see consts
  DC see consts
  CS see consts
  BLK 5V
*/

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include <me_RotaryEncoder.h>

const uint8_t
  PhaseA_pin = 2,
  PhaseB_pin = 3,
  Switch_pin = 4,

  Display_cs_pin = 8,
  Display_dc_pin = 9,
  Display_reset_pin = 10;

const int32_t
  Debounce_ms = 40;

RotaryEncoder RotaryEncoder(PhaseA_pin, PhaseB_pin, Switch_pin);
Adafruit_ST7789 Display = Adafruit_ST7789(Display_cs_pin, Display_dc_pin, Display_reset_pin);

void setup() {
  Serial.begin(9600);

  RotaryEncoder.Debounce_ms = Debounce_ms;

  attachInterrupt(digitalPinToInterrupt(PhaseA_pin), Phase_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PhaseB_pin), Phase_change, CHANGE);

  Display.init(240, 240);
  Display.fillScreen(ST77XX_BLACK);
}

void Phase_change() {
  RotaryEncoder.UpdateState();
}

int32_t PrevPosition_printed = -1;
bool PrevSwitchState_printed = false;

void loop() {
  cli();
  int32_t Position_copy = RotaryEncoder.Position;
  sei();
  bool NeedPositionRedisplay = (Position_copy != PrevPosition_printed);
  if (RotaryEncoder.PositionHasChanged || NeedPositionRedisplay) {
    if (NeedPositionRedisplay) {
      int32_t x_prev = PrevPosition_printed / 1;
      int32_t x = Position_copy / 1;

      if (x != x_prev) {
        Display.drawFastVLine(x_prev, 0, Display.height() - 1, ST77XX_BLACK);
        Display.drawFastHLine(0, x_prev, Display.width(), ST77XX_BLACK);
        Display.drawFastVLine(x, 0, Display.height() - 1, ST77XX_YELLOW);
        Display.drawFastHLine(0, x, Display.width(), ST77XX_YELLOW);
      }

      PrevPosition_printed = Position_copy;
      Serial.println(Position_copy);
    }
    RotaryEncoder.PositionHasChanged = false;
  }

  RotaryEncoder.UpdateSwitch();
  if (RotaryEncoder.SwitchHasChanged) {
    bool NeedSwitchRedisplay = (RotaryEncoder.SwitchState != PrevSwitchState_printed);
    if (NeedSwitchRedisplay) {
      PrevSwitchState_printed = RotaryEncoder.SwitchState;
      Display.invertDisplay(PrevSwitchState_printed);      
    }
    RotaryEncoder.SwitchHasChanged = false;
  }
}