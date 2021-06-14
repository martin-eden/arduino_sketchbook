// Measure temperature and humidity and show it on display.

/*
  Status: works
  Version: 1.0.0
  Last mod.: 2021-05-28
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

#include <SimpleDHT.h>
#include <U8g2lib.h>
#include <me_CapacitiveFilter.h>

#define Display_cs_pin 8
#define Display_dc_pin 9
#define Display_reset_pin 10

const uint8_t
  DHT11_pin = 2;

const uint32_t
  MeasureTick_ms = 1000;

const float
  DataFilterCapacitance_temp = 5.0,
  DataFilterCapacitance_hum = 10.0;

SimpleDHT11 dht11(DHT11_pin);
U8G2_SH1106_128X64_NONAME_1_4W_HW_SPI
  Display(U8G2_R0, Display_cs_pin, Display_dc_pin, Display_reset_pin);
CapacitiveFilter capacitiveFilter_temp = CapacitiveFilter(DataFilterCapacitance_temp);
CapacitiveFilter capacitiveFilter_hum = CapacitiveFilter(DataFilterCapacitance_hum);

void setup() {
  Serial.begin(9600);
  Serial.println("Measure temperature and humidity and show it on display.");

  Display.begin();
  Display.setFont(u8g2_font_osb35_tn);
}

float
  temperature = 0,
  humidity = 0;

void get_data() {
  int16_t read_error = dht11.read2(&temperature, &humidity, NULL);
  if (read_error) {
    Serial.print("Read DHT11 failed, err=");
    Serial.println(read_error);
  }

  String data_str = "RAW " + String(temperature, 0) + " " + String(humidity, 0);
  Serial.println(data_str);
}

void process_data() {
  capacitiveFilter_temp.addValue(temperature);
  temperature = capacitiveFilter_temp.getValue();

  capacitiveFilter_hum.addValue(humidity);
  humidity = capacitiveFilter_hum.getValue();
}

void display_data() {
  String data_str = String(temperature, 0) + " " + String(humidity, 0);
  static String prev_data_str = "";

  if (data_str != prev_data_str) {
    char buf_str[16];
    data_str.toCharArray(buf_str, 16);

    Display.firstPage();
    do {
      Display.drawStr(0, 50, buf_str);
      delay(50);
    } while (Display.nextPage());
    prev_data_str = data_str;
  }

  Serial.println(data_str);
}

void loop() {
  get_data();
  process_data();
  display_data();
  delay(MeasureTick_ms);
}