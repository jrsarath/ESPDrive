#include "includes/controls.hpp"
#include "includes/web_server.hpp"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string>

static const char* TAG = "web_server";

static httpd_handle_t server = nullptr;

static esp_err_t root_get_handler(httpd_req_t *req) {
    const char* resp =
        "<html><body style='text-align:center;font-family:sans-serif'>"
        "<h1>ESP Car Control</h1>"
        "<button onclick=\"fetch('/fwd')\">Forward</button>"
        "<button onclick=\"fetch('/rev')\">Reverse</button>"
        "<button onclick=\"fetch('/stop')\">Stop</button><br><br>"
        "<button onclick=\"fetch('/left')\">Left</button>"
        "<button onclick=\"fetch('/right')\">Right</button>"
        "<button onclick=\"fetch('/center')\">Center</button>"
        "</body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t fwd_handler(httpd_req_t *req) {
    forward();
    httpd_resp_send(req, "Forward", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
static esp_err_t rev_handler(httpd_req_t *req) {
    reverse();
    httpd_resp_send(req, "Reverse", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
static esp_err_t stop_handler(httpd_req_t *req) {
    stop();
    httpd_resp_send(req, "Stop", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
static esp_err_t left_handler(httpd_req_t *req) {
    left();
    httpd_resp_send(req, "Left", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
static esp_err_t right_handler(httpd_req_t *req) {
    right();
    httpd_resp_send(req, "Right", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
static esp_err_t center_handler(httpd_req_t *req) {
    center();
    httpd_resp_send(req, "Center", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

void start_webserver() {
    if (server) return; // already started

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uris[] = {
            { "/", HTTP_GET, root_get_handler, nullptr },
            { "/fwd", HTTP_GET, fwd_handler, nullptr },
            { "/rev", HTTP_GET, rev_handler, nullptr },
            { "/stop", HTTP_GET, stop_handler, nullptr },
            { "/left", HTTP_GET, left_handler, nullptr },
            { "/right", HTTP_GET, right_handler, nullptr },
            { "/center", HTTP_GET, center_handler, nullptr }
        };
        for (auto &u : uris) {
            httpd_register_uri_handler(server, &u);
        }
        ESP_LOGI(TAG, "HTTP Server started");
    } else {
        ESP_LOGE(TAG, "Failed to start server");
    }
}
