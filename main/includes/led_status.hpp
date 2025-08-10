#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void led_status_init(int gpio_num, uint32_t max_leds = 1);
void led_status_set(uint8_t r, uint8_t g, uint8_t b);
void led_status_blink(uint8_t r, uint8_t g, uint8_t b, int times, int delay_ms);

void external_strip_init(int gpio_num, uint32_t max_leds);
void create_amber_blink_task();
void external_strip_set(uint8_t r[], uint8_t g[], uint8_t b[]);
void external_strip_show();

enum VehicleLightState {
    NORMAL,
    REVERSING,
    BRAKING,
    STEERING_LEFT,
    STEERING_RIGHT,
};
void set_vehicle_lights(VehicleLightState state);

#ifdef __cplusplus
}
#endif
