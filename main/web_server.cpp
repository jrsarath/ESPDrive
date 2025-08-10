#include "includes/controls.hpp"
#include "includes/web_server.hpp"
#include "esp_log.h"
#include "esp_http_server.h"
#include <string>

static const char* TAG = "web_server";

static httpd_handle_t server = nullptr;

static esp_err_t root_get_handler(httpd_req_t *req) {
    const char* html_response = R"rawliteral(
        <!DOCTYPE html>
        <html lang="en">
            <head>
                <meta charset="UTF-8" />
                <meta name="viewport" content="width=device-width, initial-scale=1" />
                <title>ESPDrive</title>
                <style>
                    * {
                        box-sizing: border-box;
                    }
                    body {
                        font-family: 'Inter', sans-serif;
                        background: #f8fafc;
                        margin: 0; padding: 20px;
                        color: #0f172a;
                        display: flex;
                        flex-direction: column;
                        height: 100vh;
                    }
                    h1 {
                        font-weight: 900;
                        font-size: 2rem;
                        margin: 0 0 5px 0;
                    }
                    h2 {
                        font-weight: 700;
                        font-size: 1.25rem;
                        margin: 0 0 8px 0;
                    }
                    p.subtitle {
                        margin: 0 0 10px 0;
                        font-weight: 500;
                        color: #64748b;
                    }
                    .status {
                        display: inline-flex;
                        align-items: center;
                        font-weight: 700;
                        color: #ef4444; /* red */
                        background: #fee2e2; /* light red bg */
                        border-radius: 9999px;
                        padding: 14px;
                        font-size: 1rem;
                        user-select: none;
                    }
                    .status.connected {
                        color: #16a34a !important; /* green text */
                        background: #d1fae5; /* light green bg */
                    }
                    .status-dot {
                        width: 10px; 
                        height: 10px;
                        background: #ef4444; /* red */
                        border-radius: 50%;
                    }
                    .status.connected .status-dot {
                        background: #16a34a; /* green */
                    }
                    .header {
                        display: flex;
                        justify-content: space-between;
                        align-items: center;
                        margin-bottom: 2rem;
                    }

                    .btn {
                        display: inline-flex;
                        align-items: center;
                        gap: 6px;
                        background-color: #2563eb;
                        color: white;
                        font-weight: 600;
                        border: none;
                        border-radius: 0.5rem;
                        padding: 0.5rem 1.2rem;
                        cursor: pointer;
                        user-select: none;
                        font-size: 1rem;
                        transition: background-color 0.2s ease;
                    }
                    .btn:hover {
                        background-color: #1d4ed8;
                    }
                    .btn-icon {
                        display: inline-flex;
                        align-items: center;
                        justify-content: center;
                    }
                    /* Bluetooth icon from SVG path */
                    .icon-bluetooth {
                        width: 22px; height: 22px;
                        fill: #2563eb;
                    }
                    .icon-power {
                        width: 18px; height: 18px;
                        stroke: white;
                        stroke-width: 2;
                        stroke-linecap: round;
                        stroke-linejoin: round;
                        fill: none;
                    }

                    .card {
                        background: white;
                        border-radius: 1rem;
                        box-shadow:
                        0 0 20px -10px rgba(100, 116, 139, 0.2),
                        0 4px 6px -4px rgba(100, 116, 139, 0.1);
                        padding: 1.6rem 2rem;
                        margin-bottom: 2rem;
                        flex-grow: 1;
                        display: flex;
                        flex-direction: column;
                        justify-content: space-between;
                    }
                    .connect-container {
                        display: flex;
                        justify-content: space-between;
                        align-items: center;
                        gap: 1rem;
                        margin-bottom: 2rem;
                    }
                    .connect-info {
                        display: flex;
                        align-items: center;
                        gap: 1rem;
                    }
                    .connect-text {
                        display: flex;
                        flex-direction: column;
                    }
                    .connect-text strong {
                        font-weight: 700;
                        font-size: 1.1rem;
                        color: #0f172a;
                    }
                    .connect-text span {
                        color: #64748b;
                        font-weight: 500;
                    }

                    /* Joystick container */
                    #joystick {
                        touch-action: none;
                        user-select: none;
                        margin: 0 auto;
                        width: 220px;
                        height: 220px;
                        background: #f1f5f9;
                        border-radius: 50%;
                        position: relative;
                        box-shadow:
                        inset 0 0 4px rgba(100, 116, 139, 0.2);
                    }
                    /* Concentric circles */
                    #joystick::before,
                    #joystick::after {
                        content: '';
                        position: absolute;
                        border-radius: 50%;
                        border: 1px solid #cbd5e1;
                        top: 50%; left: 50%;
                        transform: translate(-50%, -50%);
                    }
                    #joystick::before {
                        width: 180px;
                        height: 180px;
                    }
                    #joystick::after {
                        width: 140px;
                        height: 140px;
                    }
                    #joystick .circle-ring {
                        position: absolute;
                        top: 50%; left: 50%;
                        transform: translate(-50%, -50%);
                        border-radius: 50%;
                        border: 1px solid #e2e8f0;
                        width: 100px; height: 100px;
                    }

                    #joystick .handle {
                        position: absolute;
                        background: #2563eb;
                        width: 60px;
                        height: 60px;
                        border-radius: 50%;
                        top: 50%;
                        left: 50%;
                        transform: translate(-50%, -50%);
                        touch-action: none;
                        box-shadow: 0 0 15px #2563ebaa;
                        transition: background-color 0.15s ease;
                    }
                    #joystick .handle:active {
                        background-color: #1d4ed8;
                    }
                    .coords {
                        text-align: center;
                        color: #64748b;
                        font-weight: 600;
                        margin-top: 1rem;
                    }
                    .values {
                        margin-top: 0.6rem;
                        display: flex;
                        justify-content: center;
                        gap: 1.5rem;
                    }
                    .values > div {
                        background: #f1f5f9;
                        border-radius: 0.5rem;
                        padding: 0.4rem 1rem;
                        color: #64748b;
                        font-weight: 600;
                        min-width: 90px;
                        text-align: center;
                    }
                    .values strong {
                        color: #0f172a;
                        font-weight: 700;
                    }

                    /* Controls container for toggles */
                    .controls-row {
                        width: 100%;
                        background: white;
                        border-radius: 1rem;
                        box-shadow:
                        0 0 20px -10px rgba(100, 116, 139, 0.2),
                        0 4px 6px -4px rgba(100, 116, 139, 0.1);
                        padding: 1.6rem 2rem;
                        display: flex;
                        gap: 2rem;
                        margin: 0 auto;
                        justify-content: space-between;
                    }
                    .toggle {
                        display: flex;
                        align-items: center;
                        gap: 0.75rem;
                        font-weight: 700;
                        color: #0f172a;
                        user-select: none;
                    }
                    .toggle svg {
                        stroke: #2563eb;
                        fill: none;
                        width: 22px;
                        height: 22px;
                    }
                    .toggle input[type="checkbox"] {
                        appearance: none;
                        width: 44px;
                        height: 24px;
                        background: #e2e8f0;
                        border-radius: 9999px;
                        position: relative;
                        cursor: pointer;
                        outline: none;
                        transition: background-color 0.2s ease;
                    }
                    .toggle input[type="checkbox"]:checked {
                        background: #2563eb;
                    }
                    .toggle input[type="checkbox"]::before {
                        content: "";
                        position: absolute;
                        width: 18px;
                        height: 18px;
                        background: white;
                        border-radius: 50%;
                        top: 3px;
                        left: 4px;
                        transition: transform 0.2s ease;
                    }
                    .toggle input[type="checkbox"]:checked::before {
                        transform: translateX(20px);
                    }
                    button.horn-btn {
                        border: none;
                        background: #f1f5f9;
                        border-radius: 0.5rem;
                        padding: 0.5rem 1.2rem;
                        font-weight: 700;
                        color: #0f172a;
                        cursor: pointer;
                        user-select: none;
                        box-shadow:
                        0 0 20px -10px rgba(100, 116, 139, 0.2),
                        0 4px 6px -4px rgba(100, 116, 139, 0.1);
                        display: flex;
                        align-items: center;
                        gap: 8px;
                    }
                    button.horn-btn svg {
                        stroke: #0f172a;
                        width: 20px;
                        height: 20px;
                    }
                </style>
            </head>
            <body>
                <div class="header">
                    <div>
                        <h1>ESPDrive</h1>
                        <p class="subtitle">An expensive yet fun-filled project</p>
                    </div>
                    <div class="status connected" aria-live="polite" aria-label="Connection status">
                        <span class="status-dot"></span>
                    </div>
                </div>

                <div class="card" aria-label="Control Joystick">
                    <h2>Control Joystick</h2>
                    <p class="subtitle" style="margin-bottom: 1rem;">Steer and throttle with a single control</p>

                    <div id="joystick" role="application" aria-label="Joystick control">
                        <div class="circle-ring"></div>
                        <div class="handle" id="handle"></div>
                    </div>

                    <div class="coords" aria-live="polite" aria-atomic="true">
                        x: <span id="coordX">0.00</span> · y: <span id="coordY">0.00</span>
                    </div>
                    <div class="values" aria-live="polite" aria-atomic="true">
                        <div>Throttle: <strong id="throttleVal">0.00</strong></div>
                        <div>Steering: <strong id="steeringVal">0.00</strong></div>
                    </div>
                </div>

                <div class="controls-row">
                    <label class="toggle" for="headlightsToggle">
                        <svg width="800" height="800" viewBox="0 0 48 48" fill="none" xmlns="http://www.w3.org/2000/svg"><path d="M24 16v6m14.142-.142L33.9 26.1M44 36h-6M4 36h6m-.142-14.142L14.1 26.1M18 36h12" stroke="#000" stroke-width="4" stroke-linecap="round" stroke-linejoin="round"/></svg>
                        <input type="checkbox" id="headlightsToggle" aria-checked="false" />
                    </label>
                    <button class="horn-btn" type="button" aria-label="Activate horn">
                        <svg xmlns="http://www.w3.org/2000/svg" width="800" height="800" viewBox="0 0 32 32" xml:space="preserve"><path d="M16 4c-2.45 0-3.78 2.086-4.967 3.478C10.319 8.315 8.835 9 7.735 9H6c-1.1 0-2 .9-2 2v10c0 1.1.9 2 2 2h1.735c1.1 0 2.584.685 3.298 1.522C12.226 25.92 13.546 28 16 28c1.1 0 2-.9 2-2V6c0-1.1-.9-2-2-2M8 21.017C7.912 21.013 6 21 6 21V11s1.912.005 2 0zM16 26c-1.389 0-2.25-1.375-3.445-2.776-.627-.735-1.558-1.347-2.555-1.744V10.52c.997-.397 1.928-1.009 2.555-1.744C13.742 7.384 14.605 6 16 6zm8.456-1.445-1.418-1.418a10.08 10.08 0 0 0 0-14.274l1.418-1.418c4.729 4.729 4.721 12.388 0 17.11m-3.582-13.688a7.26 7.26 0 0 1 0 10.266l-1.416-1.416a5.25 5.25 0 0 0 0-7.434z" style="fill:#0b1719"/></svg>
                        Horn
                    </button>
                </div>

                <script>
                    (() => {
                        const joystick = document.getElementById('joystick');
                        const handle = document.getElementById('handle');
                        const coordX = document.getElementById('coordX');
                        const coordY = document.getElementById('coordY');
                        const throttleVal = document.getElementById('throttleVal');
                        const steeringVal = document.getElementById('steeringVal');

                        const maxRadius = joystick.clientWidth / 2;
                        const handleRadius = handle.clientWidth / 2;

                        let dragging = false;

                        // Track last sent commands to avoid duplicates
                        let lastThrottleCommand = null;
                        let lastSteeringCommand = null;

                        // Simple debounce timer (milliseconds)
                        const COMMAND_INTERVAL = 150;
                        let lastCommandTime = 0;

                        function clamp(val, min, max) {
                            return Math.min(Math.max(val, min), max);
                        }

                        function updateValues(x, y) {
                            // Normalize -1 to 1 range (center is 0,0)
                            let normX = clamp(x / (maxRadius - handleRadius), -1, 1);
                            let normY = clamp(-y / (maxRadius - handleRadius), -1, 1);

                            coordX.textContent = normX.toFixed(2);
                            coordY.textContent = normY.toFixed(2);
                            throttleVal.textContent = normY.toFixed(2);
                            steeringVal.textContent = normX.toFixed(2);

                            // Send control commands based on normalized values
                            sendControlCommands(normX, normY);
                        }

                        function setHandlePosition(x, y) {
                            const dist = Math.sqrt(x*x + y*y);
                            const maxDist = maxRadius - handleRadius;
                            let newX = x;
                            let newY = y;
                            if(dist > maxDist) {
                            const angle = Math.atan2(y, x);
                            newX = Math.cos(angle) * maxDist;
                            newY = Math.sin(angle) * maxDist;
                            }
                            handle.style.transform = `translate(calc(-50% + ${newX}px), calc(-50% + ${newY}px))`;
                            updateValues(newX, newY);
                        }

                        function resetHandle() {
                            handle.style.transform = `translate(-50%, -50%)`;
                            updateValues(0, 0);
                            // Send stop/center on release
                            sendStopCommands();
                        }

                        function pointerDown(e) {
                            e.preventDefault();
                            dragging = true;
                            joystick.setPointerCapture(e.pointerId);
                            pointerMove(e);
                        }
                        function pointerMove(e) {
                            if(!dragging) return;
                            const rect = joystick.getBoundingClientRect();
                            let x = e.clientX - rect.left - maxRadius;
                            let y = e.clientY - rect.top - maxRadius;
                            setHandlePosition(x, y);
                        }
                        function pointerUp(e) {
                            dragging = false;
                            joystick.releasePointerCapture(e.pointerId);
                            resetHandle();
                        }

                        joystick.addEventListener('pointerdown', pointerDown);
                        joystick.addEventListener('pointermove', pointerMove);
                        joystick.addEventListener('pointerup', pointerUp);
                        joystick.addEventListener('pointercancel', pointerUp);
                        joystick.addEventListener('lostpointercapture', pointerUp);

                        // Send fetch commands with debounce
                        async function sendCommand(endpoint) {
                            try {
                            const now = Date.now();
                            if (now - lastCommandTime < COMMAND_INTERVAL) return; // skip if too soon
                            lastCommandTime = now;

                            const res = await fetch(endpoint);
                            if (!res.ok) {
                                console.warn(`Failed to send command: ${endpoint}`);
                            }
                            } catch (e) {
                            console.warn('Network error:', e);
                            }
                        }

                        function sendStopCommands() {
                            lastThrottleCommand = null;
                            lastSteeringCommand = null;
                            sendCommand('/stop');
                            sendCommand('/center');
                        }

                        function sendControlCommands(normX, normY) {
                            // Decide throttle command
                            let throttleCmd = null;
                            if (normY > 0.3) throttleCmd = '/fwd';
                            else if (normY < -0.3) throttleCmd = '/rev';
                            else throttleCmd = '/stop';

                            // Decide steering command
                            let steeringCmd = null;
                            if (normX > 0.3) steeringCmd = '/right';
                            else if (normX < -0.3) steeringCmd = '/left';
                            else steeringCmd = '/center';

                            // Send only if command changed
                            if (throttleCmd !== lastThrottleCommand) {
                            sendCommand(throttleCmd);
                            lastThrottleCommand = throttleCmd;
                            }
                            if (steeringCmd !== lastSteeringCommand) {
                            sendCommand(steeringCmd);
                            lastSteeringCommand = steeringCmd;
                            }
                        }

                        // Headlights toggle ARIA update
                        const headlightsToggle = document.getElementById('headlightsToggle');
                        headlightsToggle.addEventListener('change', () => {
                            headlightsToggle.setAttribute('aria-checked', headlightsToggle.checked.toString());
                            // You can also send a fetch call here to toggle headlights if API exists
                            // sendCommand(headlightsToggle.checked ? '/headlights/on' : '/headlights/off');
                        });

                        // Horn button (placeholder)
                        document.querySelector('.horn-btn').addEventListener('click', () => {
                            alert('Horn activated!');
                            // Implement actual horn API call if available
                            // sendCommand('/horn');
                        });

                    })();
                </script>
            </body>
        </html>
    )rawliteral";

    httpd_resp_send(req, html_response, HTTPD_RESP_USE_STRLEN);
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
