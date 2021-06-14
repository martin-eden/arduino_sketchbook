#include <FastLED.h>

#define LED_TYPE WS2812
#define LED_PIN A0

const int16_t
  NUM_LEDS = 60,
  LEDS_OFFSET = 0,
  LEDS_USED = NUM_LEDS - LEDS_OFFSET - 1,
  BRIGHTNESS = 64,
  UPDATES_PER_MINUTE = 72;

const uint8_t
  BACKGROUND_NOISE = 0,
  MAX_HUE_DISTANCE = LEDS_USED;

const float
  SCALE = 1.0;

float
  START_HUE_DRIFT = 1.0,
  FINSH_HUE_DRIFT = 0.777;

CRGB leds[NUM_LEDS];

void setup() {
  randomSeed(analogRead(A0));
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, LED_PIN>(leds, NUM_LEDS);
  FastLED.setCorrection(TypicalSMD5050);
  // FastLED.setCorrection(UncorrectedColor);
  FastLED.setBrightness(BRIGHTNESS);
  FastLED.setDither(0);
  delay(300);
}

void Hilight(
  uint8_t offset,
  CHSV color
) {
  leds[offset] = color;
}

int8_t GetMixNoise(
  uint8_t noise_magnitude
) {
  int8_t result = random(-noise_magnitude / 2, noise_magnitude / 2);

  return result;
}

int8_t GetHueNoise(
  uint8_t noise_magnitude
) {
  int8_t result = random(-noise_magnitude / 2, noise_magnitude / 2);
  result += random(-BACKGROUND_NOISE / 2, BACKGROUND_NOISE / 2);

  return result;
}

CHSV MixColors(
  CHSV start_color,
  CHSV finish_color,
  uint8_t noise_magnitude
) {
  fract8 mix_fract = 0x80 + GetMixNoise(noise_magnitude);
  CHSV result = blend(start_color, finish_color, mix_fract);
  result.hue += GetHueNoise(noise_magnitude);

  return result;
}

void FractalFill(
  uint8_t start,
  uint8_t finish,
  CHSV start_color,
  CHSV finish_color
) {
  if (start > finish)
    return;

  Hilight(start, start_color);

  if (start == finish)
    return;

  Hilight(finish, finish_color);

  if (start + 1 == finish)
    return;

  float noise_magnitude = (float) SCALE * (finish - start) / LEDS_USED;
  noise_magnitude = noise_magnitude * 0x100;
  noise_magnitude = constrain(noise_magnitude, 0, 0x100);

  CHSV middle_color = MixColors(start_color, finish_color, noise_magnitude);

  uint8_t middle = (start + finish) / 2;

  FractalFill(start, middle, start_color, middle_color);
  FractalFill(middle, finish, middle_color, finish_color);
}

void ChangeHueStep(
  float* cur_hue,
  float* cur_step
) {
  (*cur_hue) += (*cur_step);
  if ((*cur_hue) > 0xFF)
    *cur_hue -= 0xFF;
  if (*cur_hue < 0)
    *cur_hue += 0xFF;
}

float GetHueDistance(
  float hue_a,
  float hue_b
) {
  float result = min(abs(hue_a - hue_b), 0xFF - abs(hue_a - hue_b));

  /*
  Serial.print(hue_a); Serial.print(" ");
  Serial.print(hue_b); Serial.print(" ");
  Serial.print(result); Serial.print(" ");
  Serial.println();
  //*/

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
    finish_color = CHSV(50, 255, 255);
    // start_color = rgb2hsv_approximate(CRGB::CornflowerBlue),
    // finish_color = rgb2hsv_approximate(CRGB::Chartreuse);

  // .hue field values may be clamped. So we roll hue independently.
  static float
    start_color_hue = start_color.hue,
    finish_color_hue = finish_color.hue;

  FractalFill(LEDS_OFFSET, NUM_LEDS - 1, start_color, finish_color);

  FastLED.show();
  FastLED.delay(60000 / UPDATES_PER_MINUTE);

  ChangeHueStep(&start_color_hue, &START_HUE_DRIFT);
  ChangeHueStep(&finish_color_hue, &FINSH_HUE_DRIFT);

  if (GetHueDistance(start_color_hue, finish_color_hue) > MAX_HUE_DISTANCE) {
    // Never invert slowest point.
    START_HUE_DRIFT = -START_HUE_DRIFT;
    do {
      ChangeHueStep(&start_color_hue, &START_HUE_DRIFT);
    } while (GetHueDistance(start_color_hue, finish_color_hue) > MAX_HUE_DISTANCE);
  }

  start_color.hue = start_color_hue;
  finish_color.hue = finish_color_hue;
}
