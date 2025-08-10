#include "includes/led_status.hpp"
#include "led_strip.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static const char* TAG = "LED_STATUS";

static led_strip_handle_t led_strip = nullptr;
static led_strip_handle_t ext_led_strip = nullptr;

static TaskHandle_t amber_blink_task_handle = nullptr;
static SemaphoreHandle_t amber_blink_mutex = nullptr;

static bool amber_blink_active = false;
static uint8_t base_r[4] = {0};
static uint8_t base_g[4] = {0};
static uint8_t base_b[4] = {0};

static int amber_indices[2] = {0};
static int amber_count = 0;

static const uint32_t NUM_LEDS = 4;
static const uint8_t amber_r_val = 255, amber_g_val = 133, amber_b_val = 3;

// Forward declarations
void external_strip_set(uint8_t r[], uint8_t g[], uint8_t b[], uint32_t num_leds);
void external_strip_show();
void set_base_led_colors(VehicleLightState state);

void led_status_init(int gpio_num, uint32_t max_leds) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_num,
        .max_leds = max_leds,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_RGB,
        .flags = {
            .invert_out = false
        }
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false
        }
    };

    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LED strip (err=%d)", err);
        return;
    }

    led_strip_clear(led_strip);
    ESP_LOGI(TAG, "LED strip initialized on GPIO %d with %u LED(s)", gpio_num, (unsigned int)max_leds);
}

void led_status_set(uint8_t r, uint8_t g, uint8_t b) {
    if (!led_strip) return;

    led_strip_set_pixel(led_strip, 0, r, g, b);
    led_strip_refresh(led_strip);
}

void external_strip_init(int gpio_num, uint32_t max_leds) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = gpio_num,
        .max_leds = max_leds,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = false
        }
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false
        }
    };

    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &ext_led_strip);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize external LED strip (err=%d)", err);
        return;
    }

    led_strip_clear(ext_led_strip);
    ESP_LOGI(TAG, "External LED strip initialized on GPIO %d with %u LED(s)", gpio_num, (unsigned int)max_leds);
}

void external_strip_set(uint8_t r[], uint8_t g[], uint8_t b[], uint32_t num_leds) {
    if (!ext_led_strip) return;

    for (uint32_t i = 0; i < num_leds; i++) {
        led_strip_set_pixel(ext_led_strip, i, r[i], g[i], b[i]);
    }
}

void external_strip_show() {
    if (!ext_led_strip) return;
    led_strip_refresh(ext_led_strip);
}

// Amber blinking task — runs forever
static void amber_blink_task(void *param) {
    while (true) {
        bool blink_now = false;

        // Take mutex and check if blinking is active
        if (xSemaphoreTake(amber_blink_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            blink_now = amber_blink_active;
            xSemaphoreGive(amber_blink_mutex);
        }

        if (!blink_now) {
            // If blinking not active, just show base colors and sleep a bit
            external_strip_set(base_r, base_g, base_b, NUM_LEDS);
            external_strip_show();
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        // Blink ON: set amber LEDs ON, others base color
        for (int i = 0; i < amber_count; i++) {
            int idx = amber_indices[i];
            base_r[idx] = amber_r_val;
            base_g[idx] = amber_g_val;
            base_b[idx] = amber_b_val;
        }
        external_strip_set(base_r, base_g, base_b, NUM_LEDS);
        external_strip_show();
        vTaskDelay(pdMS_TO_TICKS(500));

        // Blink OFF: set amber LEDs OFF, others base color
        for (int i = 0; i < amber_count; i++) {
            int idx = amber_indices[i];
            base_r[idx] = 0;
            base_g[idx] = 0;
            base_b[idx] = 0;
        }
        external_strip_set(base_r, base_g, base_b, NUM_LEDS);
        external_strip_show();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void create_amber_blink_task() {
    amber_blink_mutex = xSemaphoreCreateMutex();
    xTaskCreate(amber_blink_task, "amber_blink_task", 2048, NULL, 5, &amber_blink_task_handle);
}

void set_base_led_colors(VehicleLightState state) {
    const uint8_t white_r = 255, white_g = 255, white_b = 255;
    const uint8_t red_full = 255;
    const uint8_t red_half = 128;

    // Reset amber LEDs indices
    amber_count = 0;

    switch(state) {
        case REVERSING:
            for (int i = 0; i < NUM_LEDS; i++) {
                base_r[i] = white_r;
                base_g[i] = white_g;
                base_b[i] = white_b;
            }
            break;

        case BRAKING:
            base_r[0] = red_full; base_g[0] = 0; base_b[0] = 0;
            base_r[1] = red_full; base_g[1] = 0; base_b[1] = 0;
            base_r[2] = white_r; base_g[2] = white_g; base_b[2] = white_b;
            base_r[3] = white_r; base_g[3] = white_g; base_b[3] = white_b;
            break;

        case NORMAL:
            base_r[0] = red_half; base_g[0] = 0; base_b[0] = 0;
            base_r[1] = red_half; base_g[1] = 0; base_b[1] = 0;
            base_r[2] = white_r; base_g[2] = white_g; base_b[2] = white_b;
            base_r[3] = white_r; base_g[3] = white_g; base_b[3] = white_b;
            break;

        case STEERING_LEFT:
            base_r[0] = red_half; base_g[0] = red_half; base_b[0] = red_half;
            base_r[1] = 0; base_g[1] = 0; base_b[1] = 0; // amber LEDs off — blinking task controls these
            base_r[2] = 0; base_g[2] = 0; base_b[2] = 0;
            base_r[3] = white_r; base_g[3] = white_g; base_b[3] = white_b;

            amber_indices[0] = 1;
            amber_indices[1] = 2;
            amber_count = 2;
            break;

        case STEERING_RIGHT:
            base_r[0] = 0; base_g[0] = 0; base_b[0] = 0; // amber LEDs off — blinking task controls these
            base_r[1] = red_half; base_g[1] = 0; base_b[1] = 0;
            base_r[2] = white_r; base_g[2] = white_g; base_b[2] = white_b;
            base_r[3] = 0; base_g[3] = 0; base_b[3] = 0;

            amber_indices[0] = 0;
            amber_indices[1] = 3;
            amber_count = 2;
            break;

        default:
            for (int i = 0; i < NUM_LEDS; i++) {
                base_r[i] = 0;
                base_g[i] = 0;
                base_b[i] = 0;
            }
            break;
    }

    external_strip_set(base_r, base_g, base_b, NUM_LEDS);
    external_strip_show();
}

void set_vehicle_lights(VehicleLightState state) {
    if (!amber_blink_mutex) {
        ESP_LOGW(TAG, "amber_blink_mutex not initialized!");
        return;
    }

    // Take mutex to update blinking flag and LED indices safely
    if (xSemaphoreTake(amber_blink_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        amber_blink_active = (state == STEERING_LEFT || state == STEERING_RIGHT);
        xSemaphoreGive(amber_blink_mutex);
    }

    set_base_led_colors(state);

    ESP_LOGI(TAG, "Set vehicle lights state %d", state);
}
