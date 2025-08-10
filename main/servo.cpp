#include "includes/servo.hpp"
#include "driver/ledc.h"
#include "esp_log.h"

static const char* TAG = "Servo";

Servo::Servo(int gpio_pin) : pin_(gpio_pin) {}

void Servo::init() {
    ledc_timer_config_t tt = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = RES,
        .timer_num = TIMER,
        .freq_hz = FREQ,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&tt);

    ledc_channel_config_t ch = {
        .gpio_num = pin_,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = CHANNEL,
        .timer_sel = TIMER,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ch);
    // center servo
    writeAngle(90);
    ESP_LOGI(TAG, "Servo initialized on pin %d", pin_);
}

uint32_t Servo::angleToDutyUs(int angle){
    // typical servo pulse 500us - 2500us
    int duty_us = 500 + (angle * 2000 / 180);
    return (uint32_t)duty_us;
}

void Servo::writeAngle(int angle){
    if(angle < 0) angle = 0;
    if(angle > 180) angle = 180;
    uint32_t duty_us = angleToDutyUs(angle);
    // convert microseconds to duty for RES resolution and FREQ
    uint32_t max_duty = (1 << RES) - 1;
    uint32_t duty = (uint32_t)(((uint64_t)duty_us * FREQ * max_duty) / 1000000ULL);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, CHANNEL, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, CHANNEL);
}
