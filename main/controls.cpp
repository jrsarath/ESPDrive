#include "includes/controls.hpp"
#include "includes/motor.hpp"
#include "includes/servo.hpp"
#include "includes/led_status.hpp"
#include "esp_err.h"

extern Motor gMotor;
extern Servo gServo;

esp_err_t forward(int speed) {
    set_vehicle_lights(NORMAL);
    gMotor.forward(speed);
    return ESP_OK;
}
esp_err_t reverse(int speed) {
    set_vehicle_lights(REVERSING);
    gMotor.reverse(speed);
    return ESP_OK;
}
esp_err_t stop() {
    set_vehicle_lights(BRAKING);
    gMotor.stop();
    return ESP_OK;
}
esp_err_t left() {
    // led_status_blink(255, 255, 0, 3, 500);
    set_vehicle_lights(STEERING_LEFT);
    gServo.writeAngle(30);
    return ESP_OK;
}
esp_err_t right() {
    // led_status_blink(255, 255, 0, 3, 500);
    set_vehicle_lights(STEERING_RIGHT);
    gServo.writeAngle(150);
    return ESP_OK;
}
esp_err_t center() {
    set_vehicle_lights(NORMAL);
    gServo.writeAngle(90);
    return ESP_OK;
}