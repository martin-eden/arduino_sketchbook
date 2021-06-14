// Rotary encoder tracking

/*
  Status: done
  Version: 1.0
  Last mod.: 2021-04-28
*/

#include <me_RotaryEncoder.h>

const uint8_t
  PhaseA_pin = 2,
  PhaseB_pin = 3,
  Switch_pin = 4;

const int32_t
  Debounce_ms = 40;

RotaryEncoder RotaryEncoder(PhaseA_pin, PhaseB_pin, Switch_pin);

void setup() {
  Serial.begin(9600);

  RotaryEncoder.Debounce_ms = Debounce_ms;

  attachInterrupt(digitalPinToInterrupt(PhaseA_pin), Phase_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PhaseB_pin), Phase_change, CHANGE);
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
      Serial.print("Position ");
      Serial.println(Position_copy);
      PrevPosition_printed = Position_copy;
    }
    RotaryEncoder.PositionHasChanged = false;
  }

  RotaryEncoder.UpdateSwitch();
  if (RotaryEncoder.SwitchHasChanged) {
    bool NeedSwitchRedisplay = (RotaryEncoder.SwitchState != PrevSwitchState_printed);
    if (NeedSwitchRedisplay) {
      Serial.print("Switch ");
      Serial.println(RotaryEncoder.SwitchState);
      PrevSwitchState_printed = RotaryEncoder.SwitchState;
    }
    RotaryEncoder.SwitchHasChanged = false;
  }
}