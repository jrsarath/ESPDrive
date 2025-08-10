#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t forward();
esp_err_t reverse();
esp_err_t stop();
esp_err_t left();
esp_err_t right();
esp_err_t center();

#ifdef __cplusplus
}
#endif
