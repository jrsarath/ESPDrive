#include "includes/motor.hpp"

static const char* TAG = "Motor";

Motor::Motor(gpio_num_t stby_pin, gpio_num_t bin1_pin, gpio_num_t bin2_pin, gpio_num_t pwm_pin)
    : stby_pin_(stby_pin), bin1_pin_(bin1_pin), bin2_pin_(bin2_pin), pwm_pin_(pwm_pin) {}

void Motor::init() {
    // Direction and standby pins
    gpio_reset_pin(stby_pin_);
    gpio_set_direction(stby_pin_, GPIO_MODE_OUTPUT);

    gpio_reset_pin(bin1_pin_);
    gpio_set_direction(bin1_pin_, GPIO_MODE_OUTPUT);

    gpio_reset_pin(bin2_pin_);
    gpio_set_direction(bin2_pin_, GPIO_MODE_OUTPUT);

    // Configure PWM timer
    ledc_timer_config_t timer_cfg = {};
    timer_cfg.speed_mode       = LEDC_LOW_SPEED_MODE;
    timer_cfg.duty_resolution  = PWM_RES;
    timer_cfg.timer_num        = PWM_TIMER;
    timer_cfg.freq_hz          = PWM_FREQ;
    timer_cfg.clk_cfg          = LEDC_AUTO_CLK;
    ledc_timer_config(&timer_cfg);

    // Configure PWM channel
    ledc_channel_config_t ch_cfg = {};
    ch_cfg.gpio_num   = pwm_pin_;
    ch_cfg.speed_mode = LEDC_LOW_SPEED_MODE;
    ch_cfg.channel    = PWM_CHANNEL;
    ch_cfg.timer_sel  = PWM_TIMER;
    ch_cfg.duty       = 0;
    ch_cfg.hpoint     = 0;
    ledc_channel_config(&ch_cfg);

    // Disable motor initially
    gpio_set_level(stby_pin_, 0);

    ESP_LOGI(TAG, "Motor initialized (STBY=%d BIN1=%d BIN2=%d PWM=%d)",
             stby_pin_, bin1_pin_, bin2_pin_, pwm_pin_);
}

void Motor::forward(uint8_t speed) {
    gpio_set_level(stby_pin_, 1);
    gpio_set_level(bin1_pin_, 1);
    gpio_set_level(bin2_pin_, 0);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL);
}

void Motor::reverse(uint8_t speed) {
    gpio_set_level(stby_pin_, 1);
    gpio_set_level(bin1_pin_, 0);
    gpio_set_level(bin2_pin_, 1);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, speed);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL);
}

void Motor::stop() {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, PWM_CHANNEL);
    gpio_set_level(stby_pin_, 0);
}
