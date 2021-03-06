#pragma once

class c_humidity_measurer {
  public:
    int sensor_pin;
    int power_pin;

    int min_value;
    int max_value;
    uint16_t hysteresis;
    bool power_off_between_measures;
    bool high_means_dry;

    c_humidity_measurer();
    int get_value();
    int get_raw_value();
    bool is_line_problem;

  private:
    int16_t last_raw_value;
};
