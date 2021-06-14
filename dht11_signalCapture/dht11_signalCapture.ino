// Display signal response of DHT11 sensor.

/*
  Status: works
  Version: 1.0
  Last mod.: 2021-06-01
*/

/*
  Hardware and wiring.

  SH1106 OLED display
    CLK 13
    MOS 11
    RES <Display_reset_pin>
    DC <Display_dc_pin>
    CS <Display_cs_pin>

  DHT11 temp/humidity sensor
    S <DHT11_pin>
*/

/*
  Todo:
    * Move signal capturer into class.
    * Move cyclic array into class.
*/

#include <SimpleDHT.h>
#include <U8g2lib.h>

#define Display_cs_pin 8
#define Display_dc_pin 9
#define Display_reset_pin 10

const uint8_t
  DHT11_pin = 2;

const uint32_t
  MeasureTick_ms = 120000;

SimpleDHT11 dht11(DHT11_pin);
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI
  Display(U8G2_R0, Display_cs_pin, Display_dc_pin, Display_reset_pin);

void setup() {
  Serial.begin(9600);
  Serial.println("Display signal response of DHT11 sensor.");

  Display.begin();
  Display.setFont(u8g2_font_osb35_tn);

  delay(1000);

  ResetHistory();
  attachInterrupt(digitalPinToInterrupt(DHT11_pin), SignalChange, CHANGE);
}

struct SampleRec {
  uint32_t time;
  uint8_t value;
};

const uint8_t MaxHistoryLen = 100;

volatile static struct SampleRec SignalHistory[MaxHistoryLen];

volatile static uint8_t
  HistoryStartIdx,
  HistoryEndIdx;

static bool
  _HistoryIsEmpty = true;

uint8_t NextIdx(uint8_t CurIdx) {
  return (CurIdx + 1) % MaxHistoryLen;
}

uint8_t PrevIdx(uint8_t CurIdx) {
  return (CurIdx + MaxHistoryLen - 1) % MaxHistoryLen;
}

void AddHistoryRecord(uint8_t value) {
  SignalHistory[HistoryEndIdx].time = micros();
  SignalHistory[HistoryEndIdx].value = value;
  HistoryEndIdx = NextIdx(HistoryEndIdx);
  if (HistoryStartIdx == HistoryEndIdx)
    HistoryStartIdx = NextIdx(HistoryStartIdx);
  _HistoryIsEmpty = false;
}

void ResetHistory() {
  HistoryStartIdx = 0;
  HistoryEndIdx = 0;
  _HistoryIsEmpty = true;
}

bool HistoryIsEmpty() {
  return _HistoryIsEmpty;
}

void SignalChange() {
  AddHistoryRecord(digitalRead(DHT11_pin));
}

void DisplayHistory() {
  Serial.println("___");
  if (!HistoryIsEmpty()) {
    uint8_t CurIdx = HistoryStartIdx;
    uint32_t TimeBase;
    do {
      if (CurIdx == HistoryStartIdx)
        TimeBase = SignalHistory[CurIdx].time;
      else
        TimeBase = SignalHistory[PrevIdx(CurIdx)].time;
      Serial.print(
        String("") +
        "[" + CurIdx + "] " +
        "duration: " + (SignalHistory[CurIdx].time - TimeBase) + "\n" +
        "value: " + SignalHistory[CurIdx].value + ", "
      );
      CurIdx = NextIdx(CurIdx);
    } while (CurIdx != HistoryEndIdx);
    Serial.println();
  }
  Serial.println("~~~");
}

void initiateDHT11Response() {
  pinMode(DHT11_pin, OUTPUT);
  digitalWrite(DHT11_pin, LOW);
  delay(20);

  digitalWrite(DHT11_pin, HIGH);
  pinMode(DHT11_pin, INPUT);
  delayMicroseconds(25);
}

void loop() {
  ResetHistory();
  initiateDHT11Response();
  delay(200);
  DisplayHistory();
  delay(MeasureTick_ms);
}