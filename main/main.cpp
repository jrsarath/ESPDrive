#include "includes/motor.hpp"
#include "includes/servo.hpp"
#include "includes/wifi.hpp"
#include "includes/web_server.hpp"
#include "includes/led_status.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include <dirent.h>

static constexpr int STBY_PIN = 19;
static constexpr int BIN1_PIN = 20;
static constexpr int BIN2_PIN = 21;
static constexpr int PWMB_PIN = 22;
static constexpr int SERVO_PIN = 5;
static constexpr int NEOPIXEL_PIN = 4;
static constexpr int INBUILT_NEOPIXEL_PIN = 8;

static const char* WIFI_SSID = "ESPDrive";
static const char* WIFI_PASS = "12345678";

static const char* TAG = "main";


Motor gMotor(static_cast<gpio_num_t>(STBY_PIN), static_cast<gpio_num_t>(BIN1_PIN), static_cast<gpio_num_t>(BIN2_PIN), static_cast<gpio_num_t>(PWMB_PIN));
Servo gServo(static_cast<gpio_num_t>(SERVO_PIN));

extern void start_webserver();

extern "C" void app_main(void) {
    esp_log_level_set("*", ESP_LOG_INFO);
    ESP_LOGI(TAG, "Starting ESP Car");

    // Mount SPIFFS
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
    };
    esp_vfs_spiffs_register(&conf);

    DIR* dir = opendir("/spiffs");
    if (dir == NULL) {
        return;
    }

    while (true) {
        struct dirent* de = readdir(dir);
        if (!de) {
            break;
        }
        
        printf("Found file: %s\n", de->d_name);
    }

    closedir(dir);

    // Initialize external 4 LED strip
    external_strip_init(NEOPIXEL_PIN, 4);
    create_amber_blink_task();
    set_vehicle_lights(NORMAL);

    // Initialize hardware
    gMotor.init();
    gServo.init();

    // Start Wi-Fi
    wifi_init_softap(WIFI_SSID, WIFI_PASS);

    // Start web server
    start_webserver();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}