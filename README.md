# 💧 ESP32 Water Flow Monitor

A compact IoT solution that turns an ESP32, a YF‑S201 hall‑effect flow sensor and an I²C LCD into a **real‑time water‑usage dashboard**.  
It exposes a modern mobile‑friendly web interface, logs cumulative monthly usage to on‑chip NVS (Preferences) and lets you **remotely switch a relay‑driven solenoid valve**.

![demo-gif-placeholder](docs/demo.gif)

---

## ✨ Features
| | |
|---|---|
| **Live readings** | Flow rate (L/min), total volume (L), monthly units (m³) & estimated dynamic pressure (bar) refresh every second |
| **Responsive UI** | Pure HTML/CSS/JS—no frameworks—works great on phones 📱 |
| **Relay control** | On/Off buttons for a pump/valve (active‑low GPIO 5) |
| **Monthly reset** | One‑click reset; value is also saved persistently every second |
| **LCD feedback** | 16×2 I²C display shows flow & pressure when no browser is open |
| **Self‑contained** | Runs a lightweight HTTP server on port 80—no cloud needed |

---

## 🛠 Bill of Materials

| Qty | Item | Notes |
|----|------|-------|
| 1 | **ESP32 DevKit** | Any Wi‑Fi capable ESP32 board |
| 1 | **YF‑S201** or similar flow sensor | Pulse output on GPIO 4 |
| 1 | **5 V relay module** | Active‑low control on GPIO 5 (or change in code) |
| 1 | **16 × 2 I²C LCD** (0x27) | SDA→21, SCL→22 (default pins) |
| 1 | 5 V Dynamo Generator | kinetic energy of flowing water |
| — | Assorted jump wires, ½″ → sensor adapters, enclosure, etc. |

> **Pipe diameter** in code (`pipeDiameter = 0.0127 m`) assumes a ½″ line.  
> Change it if your setup differs to keep velocity/pressure correct.

---

## ⚙️ Wiring



ESP32        Flow Sensor (YF‑S201)
\=====        =====================
3V3  ──────▶ Vcc   (red)
GND  ──────▶ GND   (black)
GPIO4──────▶ Signal(yellow)

ESP32        I²C LCD
\=====        =======
5V  ──────▶ Vcc
GND ──────▶ GND
GPIO21────▶ SDA
GPIO22────▶ SCL

ESP32        Relay
\=====        =====
5V  ──────▶ Vcc
GND ──────▶ GND
GPIO5─────▶ IN



---

## 🚀 Getting Started

1. **Clone the repo**  
   
   git clone https://github.com/<your‑user>/esp32-water-flow-monitor.git
   cd esp32-water-flow-monitor


2. **Install libraries** in the Arduino IDE / PlatformIO

   * `WiFi.h` *(bundled with ESP32 core)*
   * `WebServer.h` *(bundled)*
   * `LiquidCrystal_I2C` by Frank de Brabander (or compatible)
   * `Preferences` *(bundled)*

3. **Open `main.ino`** and edit:

   ```cpp
   const char* ssid     = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   float calibrationFactor = 7.5;   // Pulses per litre (adjust!)
   ```

4. **Build & flash**
   Compile for the *ESP32 Dev Module* target and upload via USB.

5. **Browse the dashboard**
   Open the serial monitor to find the IP address, then visit:

   ```
   http://<esp32-ip>/
   ```

---

## 🌐 Web API

| Endpoint | Method | Description                                                              |
| -------- | ------ | ------------------------------------------------------------------------ |
| `/`      | GET    | Web dashboard (HTML)                                                     |
| `/data`  | GET    | JSON payload with `flowRate`, `totalLiters`, `totalUnits`, `pressureBar` |
| `/reset` | GET    | Zeroes the monthly counter                                               |
| `/on`    | GET    | Sets relay **ON** (GPIO5 LOW)                                            |
| `/off`   | GET    | Sets relay **OFF** (GPIO5 HIGH)                                          |

---

## 🧪 Calibration

1. Place a measured container (e.g. 1 L beaker) under the flow.
2. Count **pulses per litre** using the serial monitor:

   ```
   flowRate = pulseCount / calibrationFactor;
   ```
3. Compute `calibrationFactor = pulseCount / litres`.
4. Update the constant near the top of the sketch and re‑flash.

---

## 🗂 Data Persistence

* Monthly units (integer m³) are saved in **NVS** (`Preferences`) every loop.
* On boot the value is restored, so brief outages won’t lose totals.
* Pressing **“Reset Monthly Usage”** zeroes both RAM & NVS copies.

---

## 📋 Roadmap / Ideas

* ✅ OTA updates (ArduinoOTA)
* 📦 MQTT publishing for Home Assistant
* 📊 InfluxDB/Grafana integration
* 🔒 WPA2‑AP fallback if Wi‑Fi creds fail

> Contributions are welcome—see below!

---

## 🤝 Contributing

1. Fork the project and create your feature branch:
   `git checkout -b feat/amazing-idea`
2. Commit your changes with conventional commits.
3. Push to the branch, open a **PR**, explain ***why***.

All contributions—code, docs, ideas—make the project better. 💙

---

## 📄 License

This project is licensed under the **MIT License**.
See `LICENSE` for details.

---

## 🙏 Acknowledgements

* **ESP32 Arduino Core** – Espressif Systems
* **LiquidCrystal\_I2C** library
* Inspired by countless open‑source water‑meter hacks 🌍


