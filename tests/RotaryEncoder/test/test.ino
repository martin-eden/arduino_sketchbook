// Rotary encoder tracking

/*
  Status: works great
  Version: 0.9
  Last mod.: 2021-04-26
*/

/*
  Rotary encoder model: KY-040

  Tracking done via hardware interrupts on two phase pins. For button press
  software polling is used, as there are just two hardware interrupts
  available on Arduino Uno.

  Care spent on assuring that we keep tracking position changes while
  doing other job, like Serial.print'ing previous position value.

  Button press has noise when releasing. So we need to filter it. Average
  fastest time I was able to toggle switch is 80ms. Using 1/2 of it as
  debounce value.
*/

const uint8_t
  PhaseA_pin = 2,
  PhaseB_pin = 3,
  Switch_pin = 4;

const int32_t
  Debounce_ms = 40;

volatile int32_t Position = 0;
bool PositionHasChanged = false;

bool SwitchState;
bool SwitchHasChanged = false;

uint8_t GetState(bool PhaseA, bool PhaseB) {
  if (!PhaseA & !PhaseB)
    return 0;
  else if (!PhaseA & PhaseB)
    return 1;
  else if (PhaseA & PhaseB)
    return 2;
  else if (PhaseA & !PhaseB)
    return 3;
  else
    return 100;
}

uint8_t GetPinsState() {
  return GetState(digitalRead(PhaseA_pin), digitalRead(PhaseB_pin));
}

uint8_t CurState, PrevState;

const int8_t UnreachableState = 50;

int8_t StateDelta[4][4] = {
  {0, 1, UnreachableState, -1},
  {-1, 0, 1, UnreachableState},
  {UnreachableState, -1, 0, 1},
  {1, UnreachableState, -1, 0}
};

void UpdateState() {
  PrevState = CurState;
  CurState = GetPinsState();

  int8_t StateDelta_value = StateDelta[CurState][PrevState];
  if (StateDelta_value == UnreachableState)
    CurState = PrevState;
  else
    Position += StateDelta_value;

  if (CurState != PrevState)
    PositionHasChanged = true;
}

bool PrevSwitchState;

bool GetSwitchState() {
  return digitalRead(Switch_pin);
}

int32_t LastSwitchTime_ms;

void UpdateSwitch() {
  PrevSwitchState = SwitchState;
  SwitchState = GetSwitchState();
  int32_t CurTime_ms = millis();
  if (SwitchState != PrevSwitchState) {
    if (CurTime_ms - LastSwitchTime_ms > Debounce_ms) {
      SwitchHasChanged = true;
      LastSwitchTime_ms = CurTime_ms;
    }
  }
}

void PhaseA_change() {
  UpdateState();
}

void PhaseB_change() {
  UpdateState();
}

void setup() {
  Serial.begin(9600);

  pinMode(PhaseA_pin, INPUT_PULLUP);
  pinMode(PhaseB_pin, INPUT_PULLUP);
  pinMode(Switch_pin, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(PhaseA_pin), PhaseA_change, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PhaseB_pin), PhaseB_change, CHANGE);

  CurState = GetPinsState();
  UpdateState();
  PositionHasChanged = true;

  SwitchState = GetSwitchState();
  LastSwitchTime_ms = 0;
  UpdateSwitch();
  SwitchHasChanged = true;
}

int32_t PrevPosition_printed = -1;
bool PrevSwitchState_printed = false;

void loop() {
  int32_t Position_copy = Position;
  bool NeedPositionRedisplay = (Position_copy != PrevPosition_printed);
  if (PositionHasChanged || NeedPositionRedisplay) {
    if (NeedPositionRedisplay) {
      Serial.print("Position ");
      Serial.println(Position_copy);
      PrevPosition_printed = Position_copy;
    }
    PositionHasChanged = false;
  }

  UpdateSwitch();
  bool SwitchState_copy = SwitchState;
  bool NeedSwitchRedisplay = (SwitchState_copy != PrevSwitchState_printed);
  if (SwitchHasChanged && NeedSwitchRedisplay) {
    if (NeedSwitchRedisplay) {
      Serial.print("Switch ");
      Serial.println(SwitchState_copy);
      PrevSwitchState_printed = SwitchState_copy;
    }
    SwitchHasChanged = false;
  }
}