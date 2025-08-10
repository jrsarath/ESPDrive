#pragma once
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"

class Motor {
    public:
        Motor(gpio_num_t stby_pin, gpio_num_t bin1_pin, gpio_num_t bin2_pin, gpio_num_t pwm_pin);

        void init();
        void forward(uint8_t speed);
        void reverse(uint8_t speed);
        void stop();

    private:
        gpio_num_t stby_pin_;
        gpio_num_t bin1_pin_;
        gpio_num_t bin2_pin_;
        gpio_num_t pwm_pin_;

        static constexpr ledc_timer_bit_t PWM_RES   = LEDC_TIMER_8_BIT;
        static constexpr ledc_timer_t     PWM_TIMER = LEDC_TIMER_0;
        static constexpr ledc_channel_t   PWM_CHANNEL = LEDC_CHANNEL_0;
        static constexpr uint32_t         PWM_FREQ  = 1000;
};
