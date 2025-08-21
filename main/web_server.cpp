#include "includes/controls.hpp"
#include "includes/web_server.hpp"
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "esp_vfs.h"
#include "esp_log.h"
#include "cJSON.h"
#include <string>

static const char* TAG = "web_server";

static httpd_handle_t server = nullptr;
static int client_session_id;

static esp_err_t static_get_handler(httpd_req_t *req) {
    // Mount SPIFFS
    esp_vfs_spiffs_conf_t esp_vfs_spiffs_conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true};
    esp_vfs_spiffs_register(&esp_vfs_spiffs_conf);

    char path[600];
    if (strcmp(req->uri, "/") == 0) {
        strcpy(path, "/spiffs/index.html");
    } else {
        sprintf(path, "/spiffs%s", req->uri);
    }

    char *ext = strrchr(path, '.');
    if (ext == NULL || strncmp(ext, ".local", strlen(".local")) == 0) {
        httpd_resp_set_status(req, "301 Moved Permanently");
        httpd_resp_set_hdr(req, "Location", "/");
        httpd_resp_send(req, NULL, 0);
        return ESP_OK;
    }
    if (strcmp(ext, ".css") == 0) {
        httpd_resp_set_type(req, "text/css");
    } 
    if (strcmp(ext, ".js") == 0) {
        httpd_resp_set_type(req, "text/javascript");
    }
    if (strcmp(ext, ".png") == 0) {
        httpd_resp_set_type(req, "image/png");
    }

    FILE *file = fopen(path, "r");
    if (file == NULL) {
        httpd_resp_send_404(req);
        esp_vfs_spiffs_unregister(NULL);
        return ESP_OK;
    }

    char lineRead[256];
    while (fgets(lineRead, sizeof(lineRead), file)) {
        httpd_resp_sendstr_chunk(req, lineRead);
    }
    httpd_resp_sendstr_chunk(req, NULL);

    esp_vfs_spiffs_unregister(NULL);
    return ESP_OK;
}
static esp_err_t ws_handler(httpd_req_t *req) {
    if (req->method == HTTP_GET) {
        // Client is connecting
        ESP_LOGI(TAG, "WebSocket client connected");
        return ESP_OK;
    }

    client_session_id = httpd_req_to_sockfd(req);

    httpd_ws_frame_t ws_pkt;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t*)malloc(WS_MAX_SIZE);
    if (!ws_pkt.payload) {
        ESP_LOGE(TAG, "WS: malloc failed");
        return ESP_ERR_NO_MEM;
    }
    if (httpd_ws_recv_frame(req, &ws_pkt, WS_MAX_SIZE) != ESP_OK) {
        ESP_LOGE(TAG, "WS: recv_frame failed");
        free(ws_pkt.payload);
        return ESP_FAIL;
    }

    cJSON *payload = cJSON_ParseWithLength((char *)ws_pkt.payload, ws_pkt.len);
    if (!payload) {
        ESP_LOGE(TAG, "WS: cJSON_ParseWithLength failed");
        free(ws_pkt.payload);
        return ESP_FAIL;
    }
    cJSON *typeItem = cJSON_GetObjectItem(payload, "type");
    cJSON *commandItem = cJSON_GetObjectItem(payload, "command");
    cJSON *valueItem = cJSON_GetObjectItem(payload, "value");

    if (!typeItem || !commandItem || !cJSON_IsString(typeItem) || !cJSON_IsString(commandItem)) {
        ESP_LOGE(TAG, "WS: JSON missing 'type' or 'command' string");
        cJSON_Delete(payload);
        free(ws_pkt.payload);
        return ESP_FAIL;
    }
    const char *type = typeItem->valuestring;
    const char *command = commandItem->valuestring;
    int value = 200;
    if (valueItem && cJSON_IsNumber(valueItem)) {
        value = valueItem->valueint;
    }

    ESP_LOGI(TAG, "WS: type: %s, command: %s, value: %d", type, command, value);

    // Bind actions to commands with value
    if (strcmp(command, "fwd") == 0) {
        forward(value);
    } else if (strcmp(command, "rev") == 0) {
        reverse(value);
    } else if (strcmp(command, "left") == 0) {
        left();
    } else if (strcmp(command, "right") == 0) {
        right();
    } else if (strcmp(command, "center") == 0) {
        center();
    } else if (strcmp(command, "stop") == 0) {
        stop();
    }

    cJSON_Delete(payload);
    free(ws_pkt.payload);

    const char *responsestr = "OK";
    httpd_ws_frame_t ws_responce = {
        .final = true,
        .fragmented = false,
        .type = HTTPD_WS_TYPE_TEXT,
        .payload = (uint8_t*)responsestr,
        .len = strlen(responsestr)};
    esp_err_t send_result = httpd_ws_send_frame(req, &ws_responce);
    if (send_result != ESP_OK) {
        ESP_LOGE(TAG, "WS: send_frame failed");
    }

    return send_result;
}
static esp_err_t fwd_handler(httpd_req_t *req) {
    forward(255);
    httpd_resp_send(req, "Forward", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}
static esp_err_t rev_handler(httpd_req_t *req) {
    reverse(255);
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
    config.uri_match_fn = httpd_uri_match_wildcard;
    if (httpd_start(&server, &config) == ESP_OK) {
        // API endpoints
        httpd_uri_t fwd_uri = { "/fwd", HTTP_GET, fwd_handler, nullptr };
        httpd_uri_t rev_uri = { "/rev", HTTP_GET, rev_handler, nullptr };
        httpd_uri_t stop_uri = { "/stop", HTTP_GET, stop_handler, nullptr };
        httpd_uri_t left_uri = { "/left", HTTP_GET, left_handler, nullptr };
        httpd_uri_t right_uri = { "/right", HTTP_GET, right_handler, nullptr };
        httpd_uri_t center_uri = { "/center", HTTP_GET, center_handler, nullptr };
        httpd_register_uri_handler(server, &fwd_uri);
        httpd_register_uri_handler(server, &rev_uri);
        httpd_register_uri_handler(server, &stop_uri);
        httpd_register_uri_handler(server, &left_uri);
        httpd_register_uri_handler(server, &right_uri);
        httpd_register_uri_handler(server, &center_uri);

        // WebSocket endpoint
        httpd_uri_t ws_uri = {
            .uri = "/ws",
            .method = HTTP_GET,
            .handler = ws_handler,
            .is_websocket = true
        };
        httpd_register_uri_handler(server, &ws_uri);
        // Static file handler for all GET requests
        httpd_uri_t static_uri = {
            .uri = "/*",
            .method = HTTP_GET,
            .handler = static_get_handler,
            .user_ctx = nullptr
        };
        httpd_register_uri_handler(server, &static_uri);

        ESP_LOGI(TAG, "HTTP Server started");
    } else {
        ESP_LOGE(TAG, "Failed to start server");
    }
}
