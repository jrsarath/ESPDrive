# ESPDrive
**A feature-packed, hackable, and fun ESP32-based RC car platform!**

## ✨ Features
- **Multi-protocol control:**
  - Bluetooth Low Energy (BLE)
  - WiFi/WebSocket (browser-based UI)
  - Xbox controller support
- **Interactive LED system:**
  - Turn signals
  - Reverse indication
  - Sport mode lighting
- **Real-time controls:**
  - Joystick steering & throttle
  - Headlights & horn
  - Speed control
- **Modern web dashboard:**
  - Installable PWA (Progressive Web App)
  - Responsive React/Vite frontend
  - Live status indicators
- **ESP-IDF powered:**
  - Reliable, extensible firmware
  - Easy to add new features


## 🚀 Getting Started

### 1. Prerequisites
- **Hardware:**
  - ESP32, ESP32-C3, ESP32-C6, or compatible board
  - Motor driver, servo, LEDs, and power supply
- **Software:**
  - [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html) (v4.4+ recommended)
  - [Node.js](https://nodejs.org/) & [Yarn](https://yarnpkg.com/) for frontend

### 2. Clone the Project
```sh
git clone https://github.com/yourusername/ESPDrive.git
cd ESPDrive
```

### 3. Build the Frontend
```sh
cd app
yarn install
yarn build
```

### 4. Build & Flash the Firmware
```sh
cd ..
idf.py set-target esp32 # or your chip (esp32c3, etc)
idf.py build
idf.py flash
```

### 5. Connect & Drive!
- **WiFi:** Connect to the ESP32 AP, open the web dashboard in your browser.
- **Bluetooth:** Pair with your phone or Xbox controller.
- **WebSocket:** Use the web app for real-time control.

## 🛠️ Project Structure
```
├── app/           # Vite/React frontend (web app)
├── main/          # ESP-IDF firmware (C++ source)
├── partitions.csv # Partition table
├── README.md      # This file
└── ...            # Other config and build files
```

## 💡 Customization & Hacking
- Add new LED patterns, sensors, or control modes in `main/`
- Tweak the web dashboard UI in `app/src/pages/`
- Integrate with other controllers or automation systems

## 📚 Resources
- [ESP-IDF Docs](https://docs.espressif.com/projects/esp-idf/en/latest/)
- [Vite](https://vitejs.dev/)
- [React](https://react.dev/)
- [Shadcn UI](https://ui.shadcn.com)

## 🔩 Hardware Used
- **N20 motor**
- **Mini servo**
- **ESP32-C6 mini**
- **WS2812B LED strip**
- **3.7V to 5V buck boost converter**
- **1000mAh LiPo battery**
- **TB6612FNG motor driver**
- **TP4056 USB-C LiPo charger**

## Credits
- [esp-idf](https://github.com/espressif/esp-idf)

## 🏁 License & Credits
- MIT License
- Created by *Sarath "Delta" Singh* and contributors
  
Made in Kolkata with ❤️