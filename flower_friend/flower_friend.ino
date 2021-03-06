// Measures soil dryness and pours if needed.

/*
  Status: stable
  Generation: 5.2.3
  Last mod.: 2020-10-30
*/

#include <Wire.h>
#include <TM1637Display.h>

#include <me_humidity_measurer.h>
#include <me_switch.h>
#include <me_DateTime.h>
#include <me_ds3231.h>

const char
  code_descr[] = "\"Flower friend\" gardening system",
  version[] = "5.2.3";

const uint8_t
  MEASURER_1_SIGNAL_PIN = A0,
  MEASURER_1_POWER_PIN = 8,
  MOTOR_1_CONTROL_PIN = 4,

  LED_DISPLAY_INPUT_PIN = 2,
  LED_DISPLAY_CLOCK_PIN = 3,

  led_display_brightness = 13, // 00..15

  pour_hours[24] =
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    // {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},

  desired_rh_min = 60,
  desired_rh_max = 80,

  num_blocks = 1;

const uint32_t
  idle_measurement_delay = uint32_t(1000) * 60 * 12,
  pour_measurement_delay = uint32_t(1000) * 5;

TM1637Display led_display(LED_DISPLAY_CLOCK_PIN, LED_DISPLAY_INPUT_PIN);

c_humidity_measurer measurer[num_blocks];
c_switch motor[num_blocks] = {c_switch(MOTOR_1_CONTROL_PIN)};
me_ds3231 rtc;

struct t_measurer_params
  {
    uint8_t power_pin;
    uint8_t sensor_pin;
    int16_t min_value;
    int16_t max_value;
    uint16_t hysteresis;
    bool power_off_between_measures;
    bool high_means_dry;
  };

const t_measurer_params sensor_params[num_blocks] =
  {
    {MEASURER_1_POWER_PIN, MEASURER_1_SIGNAL_PIN, 30, 350, 4, true, false} //funduino
  };

void setup()
{
  led_display.setBrightness(led_display_brightness);

  Serial.begin(9600);

  Serial.println("Setup. Motors.");
  Serial.println("Setup. Sensors.");
  init_moisture_sensors();
  Serial.println("Setup. Clock. Init.");
  init_clock();
  // setup_clock();

  // assure normal business logic was done before printing status:
  Serial.println("Setup. Loop.");
  loop();

  print_signature();
  print_usage();
  print_status();
}

void init_moisture_sensors()
{
  for (int i = 0; i < num_blocks; i++)
  {
    if (sensor_params[i].sensor_pin != -1)
    {
      pinMode(sensor_params[i].sensor_pin, INPUT);
      measurer[i].sensor_pin = sensor_params[i].sensor_pin;
    }
    if (sensor_params[i].min_value != -1)
      measurer[i].min_value = sensor_params[i].min_value;
    if (sensor_params[i].max_value != -1)
      measurer[i].max_value = sensor_params[i].max_value;
    measurer[i].hysteresis = sensor_params[i].hysteresis;
    measurer[i].power_off_between_measures = sensor_params[i].power_off_between_measures;
    measurer[i].high_means_dry = sensor_params[i].high_means_dry;
    if (sensor_params[i].power_pin != -1)
    {
      pinMode(sensor_params[i].power_pin, OUTPUT);
      measurer[i].power_pin = sensor_params[i].power_pin;
    }
  }
}

void init_clock()
{
  if (!rtc.isOscillatorAtBattery())
  {
    Serial.println("Oscillator was disabled at battery mode. Enabling.");
    rtc.enableOscillatorAtBattery();
  }
  rtc.disableSqwAtBattery();
}

void setup_clock()
{
  /*
    Call this function from setup() to set RTC to time to the moment
    of uploading sketch to board. Upload sketch. Disable this function
    call and upload sketch again.

    Constant added to time to compensate time between compilation
    on main computer and execution of setup() on board.
  */

  uint8_t time_from_compile_to_setup_secs = 8;

  rtc.setDateTime(
    DateTime(F(__DATE__), F(__TIME__)) +
    TimeSpan(time_from_compile_to_setup_secs)
  );
  rtc.clearOscillatorWasStopped();
}

uint32_t next_request_time[num_blocks];

int parse_block_num(char c)
{
  if (c == '0')
    return 0;
  else if (c == '1')
    return 1;
  else
    return -1;
}

int parse_on_off(char c)
{
  if (c == '0')
    return 0;
  else if (c == '1')
    return 1;
  else
    return -1;
}

void send_measurement(uint8_t value)
{
  String msg = "";
  msg = msg + "value: " + value;
  Serial.println(msg);
}

void print_signature()
{
  Serial.println("-----------------------------------");

  Serial.print(code_descr);
  Serial.println();

  Serial.print("  Version: ");
  Serial.print(version);
  Serial.println();

  Serial.print("  Uploaded: ");
  Serial.print(get_upload_time());
  Serial.println();

  Serial.print("  File: ");
  Serial.print(F(__FILE__));
  Serial.println();

  Serial.println("-----------------------------------");
}

void display_number(uint16_t value)
{
  led_display.showNumberDec(value, true);
}

void display_time()
{
  DateTime rtc_time = rtc.getDateTime();
  uint16_t value = rtc_time.hour() * 100 + rtc_time.minute();

  display_number(value);
}

const char
  CMD_CLEAR_CLOCK_ERROR = 'C',
  CMD_MEASURE = 'M',
  CMD_MOTOR = 'T',
  CMD_GET_STATE = 'G';

void print_usage()
{
  String msg = "";
  msg =
    msg +
    "Usage:" + "\n" +
    "  " + CMD_MEASURE + " <block_num>" + "\n" +
    "  " + "  " + "Measure probe <block_num>." + "\n" +
    "  " + CMD_MOTOR + " <block_num> (0 | 1)" + "\n" +
    "  " + "  " + "Enable/disable motor for given <block_num>." + "\n" +
    "  " + CMD_GET_STATE + "\n" +
    "  " + "  " + "Print current status." + "\n" +
    "  " + CMD_CLEAR_CLOCK_ERROR + "\n" +
    "  " + "  " + "Clear clock error flag." + "\n" +
    "\n";
  Serial.print(msg);
}

void handle_command()
{
  char cmd = Serial.peek();
  uint8_t data_length = Serial.available();
  int8_t block_num;
  int8_t on_off;
  uint8_t value;
  switch (cmd)
  {
    case CMD_MEASURE:
      if (data_length < 2)
        break;
      Serial.read();
      block_num = parse_block_num(Serial.read());
      if (block_num < 0)
        break;
      value = measurer[block_num].get_value();
      send_measurement(value);
      break;
    case CMD_MOTOR:
      if (data_length < 3)
        break;
      Serial.read();
      block_num = parse_block_num(Serial.read());
      on_off = parse_on_off(Serial.read());
      if ((block_num < 0) || (on_off < 0))
        break;
      if (on_off == 1)
        motor[block_num].switch_on();
      else
        motor[block_num].switch_off();
      break;
    case CMD_GET_STATE:
      Serial.read();
      print_status();
      break;
    case CMD_CLEAR_CLOCK_ERROR:
      Serial.read();
      rtc.clearOscillatorWasStopped();
      Serial.println("Clock-was-stopped flag is cleared.");
      break;
    default:
      Serial.read();
      print_usage();
      break;
  }
}

String pad_zeroes(uint8_t value)
{
  String result = "";
  if (value < 10)
    result = result + "0";
  result = result + value;
  return result;
}

DateTime rtc_time;

String get_rtc_time()
{
  String result = "";
  rtc_time = rtc.getDateTime();
  result =
    result + rtc_time.year() +
    "-" + pad_zeroes(rtc_time.month()) +
    "-" + pad_zeroes(rtc_time.day()) +
    " " + pad_zeroes(rtc_time.hour()) +
    ":" + pad_zeroes(rtc_time.minute()) +
    ":" + pad_zeroes(rtc_time.second()) +
    // " " + "(day " + rtc_time.dayOfTheWeek() + ")" +
    "";
  return result;
}

String get_upload_time()
{
  String result = "";
  DateTime upload_time = DateTime(F(__DATE__), F(__TIME__));
  result =
    result + upload_time.year() +
    "-" + pad_zeroes(upload_time.month()) +
    "-" + pad_zeroes(upload_time.day()) +
    " " + pad_zeroes(upload_time.hour()) +
    ":" + pad_zeroes(upload_time.minute()) +
    ":" + pad_zeroes(upload_time.second()) +
    "";
  return result;
}

String get_pour_hours()
{
  String result = "";
  for (uint8_t i = 0; i < 24; i++)
    if (pour_hours[i])
      result = result + i + " ";
  return result;
}

/*
  This function is from "ShowInfo" sketch.

  I've changed constants to better fit my case.
*/
float get_board_temp()
{
  unsigned int wADC;
  float result;

  // The internal temperature has to be used
  // with the internal reference of 1.1V.

  // This code is not valid for the Arduino Mega,
  // and the Arduino Mega 2560.

  // Set the internal reference and mux.
  ADMUX = (_BV(REFS1) | _BV(REFS0) | _BV(MUX3));
  ADCSRA |= _BV(ADEN); // enable the ADC

  delay(20); // wait for voltages to become stable.

  ADCSRA |= _BV(ADSC); // Start the ADC

  while (bit_is_set(ADCSRA,ADSC));

  wADC = ADCW;

  // The constants could be wrong. It is just an adjustment.
  result = (wADC - 325.0 ) / 1.04;

  return result;
}

String represent_time_passed(uint32_t seconds)
{
  String result = "";
  TimeSpan time_span = seconds;
  bool construction_started = false;

  if (time_span.days() > 0)
  {
    result = result + time_span.days() + "d ";
    construction_started = true;
  }
  if (construction_started || (time_span.hours() > 0))
  {
    result = result + time_span.hours() + "h ";
    construction_started = true;
  }
  if (construction_started || (time_span.minutes() > 0))
  {
    result = result + time_span.minutes() + "m ";
    construction_started = true;
  }
  result = result + time_span.seconds() + "s";

  return result;
}

uint32_t cur_time;

void print_status()
{
  String msg;

  msg = "";
  msg =
    msg +
    "Status:" + "\n" +
    "  " + "Pour settings:" + "\n" +
    "  " + "  " + "pour_hours: " + get_pour_hours() + "\n" +
    "  " + "  " + "desired_rh_min: " + desired_rh_min + "\n" +
    "  " + "  " + "desired_rh_max: " + desired_rh_max + "\n";
  Serial.print(msg);


  msg = "";
  msg =
    msg +
    "  " + "Time:" + "\n" +
    "  " + "  " + "rtc_time: " + get_rtc_time() + "\n" +
    "  " + "  " + "rtc_status: ";
  if (rtc.oscillatorWasStopped())
    msg = msg + "Clock was stopped. Battery is over?";
  else
    msg = msg + "ok";
  msg = msg + "\n";
  Serial.print(msg);

  Serial.print("    uptime: ");
  Serial.print(represent_time_passed(cur_time / 1000));
  Serial.print("\n");

  Serial.print("    board_temperature: ");
  Serial.print(get_board_temp(), 2);
  Serial.print("\n");

  Serial.print("    rtc_temperature: ");
  Serial.print(rtc.getTemp(), 2);
  Serial.print("\n");

  Serial.println("  Delays:");

  Serial.print("    idle_measurement_delay: ");
  Serial.print((float)idle_measurement_delay / 1000);
  Serial.print("\n");
  Serial.print("    pour_measurement_delay: ");
  Serial.print((float)pour_measurement_delay / 1000);
  Serial.print("\n");

  Serial.println("  Blocks:");
  for (int i = 0; i < num_blocks; i++)
  {
    /*
      <is_line_problem> is set inside <get_value()>.
      In string constructor "a() + b()" the actual call order
        is b(), a().
      So we force correct order via direct assignments.
    */
    uint8_t value = measurer[i].get_value();
    uint8_t is_line_problem = measurer[i].is_line_problem; //
    msg = "";
    msg =
      msg +
      "  " + "  " + "block[" + i + "]:" + "\n" +
      "  " + "  " + "  " +
      "sensor: " + value + ", " +
      "is_line_problem: " + is_line_problem + ", " +
      "motor: " + motor[i].is_on() + "\n";
    Serial.print(msg);
  }

  Serial.print("\n");
}

bool is_time_to_work(uint8_t block_num)
{
  return
    (cur_time >= next_request_time[block_num]) ||
    (
      (cur_time < 0x10000000) &&
      (next_request_time[block_num] >= 0x80000000)
    );
}

void do_common_business()
{
}

void do_block_business(uint8_t block_num)
{
  if (pour_hours[rtc_time.hour()] || motor[block_num].is_on())
  {
    int val = measurer[block_num].get_value();
    display_number(val);
    if (measurer[block_num].is_line_problem)
    {
      motor[block_num].switch_off();
      print_status();
    }
    else
    {
      if ((val <= desired_rh_min) && motor[block_num].is_off())
      {
        motor[block_num].switch_on();
        print_status();
      }
      else if ((val >= desired_rh_max) && (motor[block_num].is_on()))
      {
        motor[block_num].switch_off();
        print_status();
      }
    }
  }

  if (motor[block_num].is_on())
    next_request_time[block_num] = cur_time + pour_measurement_delay;
  else
    next_request_time[block_num] = cur_time + idle_measurement_delay;
}

void do_business()
{
  bool time_to_work = false;

  for (uint8_t block_num = 0; block_num < num_blocks; block_num++)
    time_to_work = time_to_work || is_time_to_work(block_num);

  if (!time_to_work)
    return;

  rtc_time = rtc.getDateTime();

  do_common_business();

  for (uint8_t block_num = 0; block_num < num_blocks; block_num++)
    if (is_time_to_work(block_num))
      do_block_business(block_num);
}

void serialEvent()
{
  handle_command();
}

void loop()
{
  cur_time = millis();
  do_business();
}

/*
  2016-03-16
  2016-03-30
  2016-03-31
  2016-04-21
  2016-04-28
  2016-05-27
  2016-10-11
  2017-01-05
  2017-03-24
  2017-06-30
    RTC module added.
  2017-07-16
  2017-10-05
  2018-12-04
  2019-01-12
  2019-01-18
    Correct RTC usage.
  2019-01-29
  2019-02-27
  2019-03-21
  2019-03-30
    Lamp support.
  2019-04-19
    Lighting depends on month.
  2019-05-01
  2019-08-04
    Dropped lamp support.
  2019-12-05
    Dropped second humidity and motor modules.
  2019-12-19
    Fixed bug with losing time track.
  2020-02-17
*/
