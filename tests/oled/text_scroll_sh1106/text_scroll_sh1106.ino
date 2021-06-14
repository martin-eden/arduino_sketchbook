#include <Arduino.h>
#include <U8x8lib.h>

#define Display_cs_pin 8
#define Display_dc_pin 9
#define Display_reset_pin 10

U8X8_SH1106_128X64_NONAME_4W_HW_SPI Display(Display_cs_pin, Display_dc_pin, Display_reset_pin);

#define U8LOG_WIDTH 16
#define U8LOG_HEIGHT 8
uint8_t u8log_buffer[U8LOG_WIDTH*U8LOG_HEIGHT];
U8X8LOG u8x8log;

void setup(void) {
  Display.begin();
  Display.setFont(u8x8_font_chroma48medium8_r);

  u8x8log.begin(Display, U8LOG_WIDTH, U8LOG_HEIGHT, u8log_buffer);
  u8x8log.setRedrawMode(1);  // 0: Update screen with newline, 1: Update screen for every char
}

unsigned long t = 0;

// print the output of millis() to the terminal every second
void loop(void) {
  if (0) {
    t = millis() + 15000;
    u8x8log.print("\f");  // \f = form feed: clear the screen
  }
  u8x8log.print("millis=");
  u8x8log.print(millis());
  u8x8log.print("\n");
  delay(500);
}
