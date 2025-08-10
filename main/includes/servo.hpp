#pragma once
#include <cstdint>
#include "driver/ledc.h"

class Servo {
public:
    Servo(int gpio_pin);
    void init();
    void writeAngle(int angle); // 0-180
private:
    int pin_;
    static constexpr ledc_channel_t CHANNEL = LEDC_CHANNEL_1;
    static constexpr ledc_timer_t TIMER = LEDC_TIMER_1;
    static constexpr uint32_t FREQ = 50;
    static constexpr ledc_timer_bit_t RES = LEDC_TIMER_16_BIT;
    uint32_t angleToDutyUs(int angle);
};
