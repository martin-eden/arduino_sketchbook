#include <FastLED.h>

#define LED_TYPE WS2812
#define COLOR_ORDER GRB
#define LED_PIN A0

const int16_t
  NUM_LEDS = 60,
  LEDS_OFFSET = 0,
  LEDS_USED = NUM_LEDS - LEDS_OFFSET - 1,
  BRIGHTNESS = 128,
  MAX_CURRENT_MA = 150,
  UPDATES_PER_MINUTE = 48;

const uint8_t
  MAX_HUE_DISTANCE = 250;

float
  START_HUE_DRIFT = 1.0,
  FINSH_HUE_DRIFT = 0.85;

CRGB leds[NUM_LEDS];

void setup() {
  randomSeed(analogRead(A0));
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_CURRENT_MA);
  FastLED.setCorrection(TypicalSMD5050);
  // FastLED.setCorrection(UncorrectedColor);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(0);
  delay(100);
}

void LinearBlend(
  uint8_t start,
  uint8_t finish,
  CHSV start_color,
  CHSV finish_color
) {
  fill_gradient(
    &(leds[start]),
    finish - start + 1,
    start_color,
    finish_color,
    FORWARD_HUES
  );
}

void StepHue(
  float *cur_hue,
  float cur_step
) {
  (*cur_hue) += cur_step;
  if ((*cur_hue) > 0xFF)
    *cur_hue -= 0xFF;
  if (*cur_hue < 0)
    *cur_hue += 0xFF;
}

float GetHueDistance(
  float hue_a,
  float hue_b
) {
  // float result = min(abs(hue_a - hue_b), 0xFF - abs(hue_a - hue_b));
  float result;
  if (hue_a < hue_b)
    result = hue_b - hue_a;
  else if (hue_a == hue_b)
    result = 0;
  else
    result = (0xFF - hue_a) + hue_b;
  return result;
}

void loop() {
  static uint32_t last_millis = millis();
  uint32_t cur_time = millis();
  uint32_t time_passed = cur_time - last_millis;
  last_millis = cur_time;
  // Serial.println(time_passed);

  static CHSV
    start_color = CHSV(0, 255, 255),
    finish_color = CHSV(250, 255, 255);
    // start_color = rgb2hsv_approximate(CRGB::CornflowerBlue),
    // finish_color = rgb2hsv_approximate(CRGB::Chartreuse);

  // .hue field values may be clamped. So we roll hue independently.
  static float
    start_color_hue = start_color.hue,
    finish_color_hue = finish_color.hue;

  LinearBlend(LEDS_OFFSET, NUM_LEDS - 1, start_color, finish_color);

  FastLED.show();
  FastLED.delay(60000 / UPDATES_PER_MINUTE);

  StepHue(&start_color_hue, START_HUE_DRIFT);
  StepHue(&finish_color_hue, FINSH_HUE_DRIFT);

  if (GetHueDistance(start_color_hue, finish_color_hue) > MAX_HUE_DISTANCE) {
    /*
      Never invert slowest point. Then slowest point will cover
      whole spectra.
    */
    float *fastest_point_hue, *fastest_point_drift;
    if (abs(START_HUE_DRIFT) >= abs(FINSH_HUE_DRIFT)) {
      fastest_point_hue = &start_color_hue;
      fastest_point_drift = &START_HUE_DRIFT;
    } else {
      fastest_point_hue = &finish_color_hue;
      fastest_point_drift = &FINSH_HUE_DRIFT;
    }
    (*fastest_point_drift) = - (*fastest_point_drift);

    do {
      StepHue(fastest_point_hue, (*fastest_point_drift));
    } while (GetHueDistance(start_color_hue, finish_color_hue) > MAX_HUE_DISTANCE);
  }

  start_color.hue = start_color_hue;
  finish_color.hue = finish_color_hue;
}
