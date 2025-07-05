#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>
#include <math.h>

// WiFi credentials
const char* ssid = "R A G N A R  ®️";
const char* password = "12345678";

// Sensor setup
const int SENSOR_PIN = 4;
float calibrationFactor = 7.5;
volatile uint16_t pulseCount = 0;
unsigned long lastMillis = 0;
const long interval = 1000;

// Measurements
float flowRate = 0.0;
float totalLiters = 0.0;
int totalUnits = 0;
float velocity = 0.0;
float pressureBar = 0.0;

// Pipe properties
const float pipeDiameter = 0.0127;  // meters
const float pipeArea = 3.1416 * pow(pipeDiameter / 2, 2);  // m^2

// Relay pin
#define RELAY_PIN 5

// LCD setup
LiquidCrystal_I2C lcd(0x27, 16, 2);

WebServer server(80);
Preferences preferences;

void IRAM_ATTR pulseCounter() {
  pulseCount++;
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
    <!DOCTYPE html>
    <html>
    <head>
      <title>Water Flow Monitor</title>
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <link href="https://fonts.googleapis.com/icon?family=Material+Icons" rel="stylesheet">
      <style>
        body {
          background: #f0f4f8;
          font-family: 'Segoe UI', Arial, sans-serif;
          margin: 0; padding: 0;
        }
        .container {
          max-width: 400px;
          margin: 40px auto;
          background: #fff;
          border-radius: 16px;
          box-shadow: 0 4px 16px rgba(0,0,0,0.08);
          padding: 32px 24px;
          text-align: center;
        }
        h1 { color: #2196F3; margin-bottom: 16px; }
        .reading { font-size: 2.5em; margin: 24px 0 8px 0; font-weight: bold; }
        .label { font-size: 1.1em; color: #555; margin-bottom: 12px; }
        .icon { font-size: 2em; vertical-align: middle; margin-right: 8px; }
        .footer { margin-top: 32px; font-size: 0.95em; color: #888; }
        .live { display: inline-block; width: 10px; height: 10px; background: #4caf50; border-radius: 50%; margin-left: 10px; animation: blink 1s infinite; }
        button {
          margin-top: 12px;
          padding: 8px 14px;
          background: #f44336;
          color: white;
          border: none;
          border-radius: 8px;
          cursor: pointer;
          font-size: 0.95em;
        }
        button:hover { background: #d32f2f; }
        @keyframes blink { 0%, 100% { opacity: 1; } 50% { opacity: 0.3; } }
      </style>
      <script>
        function fetchData() {
          fetch('/data')
            .then(response => response.json())
            .then(data => {
              document.getElementById('flow').textContent = data.flowRate.toFixed(2);
              document.getElementById('total').textContent = data.totalLiters.toFixed(3);
              document.getElementById('units').textContent = data.totalUnits;
              document.getElementById('pressure').textContent = data.pressureBar.toFixed(3);
            });
        }

        function resetUsage() {
          if (confirm("Are you sure you want to reset the monthly usage?")) {
            fetch('/reset')
              .then(() => fetchData());
          }
        }

        function relayOn() {
          fetch('/on');
        }

        function relayOff() {
          fetch('/off');
        }

        setInterval(fetchData, 1000);
        window.onload = fetchData;
      </script>
    </head>
    <body>
      <div class="container">
        <h1>&#128167 Water Flow Monitor <span class="live"></span></h1>

        <div class="label"><span class="icon material-icons">water_drop</span>Flow Rate</div>
        <div class="reading"><span id="flow">0.00</span> <span style="font-size:0.5em;">L/min</span></div>

        <div class="label"><span class="icon material-icons">opacity</span>Total Volume</div>
        <div class="reading"><span id="total">0.000</span> <span style="font-size:0.5em;">L</span></div>

        <div class="label"><span class="icon material-icons">countertops</span>Monthly Units</div>
        <div class="reading"><span id="units">0</span> <span style="font-size:0.5em;">unit(s)</span></div>

        <div class="label"><span class="icon material-icons">speed</span>Pressure</div>
        <div class="reading"><span id="pressure">0.000</span> <span style="font-size:0.5em;">bar</span></div>

        <div class="footer">
          ESP32 • YF-S201 <br>
          <button onclick="relayOn()">Relay ON</button>
          <button onclick="relayOff()">Relay OFF</button><br>
          <button onclick="resetUsage()">Reset Monthly Usage</button>
        </div>
      </div>
    </body>
    </html>
  )rawliteral");
}

void handleData() {
  totalUnits = totalLiters / 1000;
  String json = "{";
  json += "\"flowRate\":" + String(flowRate, 2) + ",";
  json += "\"totalLiters\":" + String(totalLiters, 3) + ",";
  json += "\"totalUnits\":" + String(totalUnits) + ",";
  json += "\"pressureBar\":" + String(pressureBar, 3);
  json += "}";
  server.send(200, "application/json", json);
}

void handleReset() {
  totalLiters = 0;
  totalUnits = 0;
  preferences.putInt("units", 0);
  server.send(200, "text/plain", "Monthly usage reset.");
}

void handleRelayOn() {
  digitalWrite(RELAY_PIN, LOW);  // Relay ON (active-low)
  server.send(200, "text/plain", "Relay turned ON");
}

void handleRelayOff() {
  digitalWrite(RELAY_PIN, HIGH); // Relay OFF
  server.send(200, "text/plain", "Relay turned OFF");
}

void setup() {
  Serial.begin(115200);

  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);  // OFF at start

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! IP: " + WiFi.localIP().toString());

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(" Water Monitor");

  pinMode(SENSOR_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseCounter, FALLING);
  lastMillis = millis();

  preferences.begin("water-data", false);
  totalUnits = preferences.getInt("units", 0);
  totalLiters = totalUnits * 1000.0;

  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.on("/reset", handleReset);
  server.on("/on", handleRelayOn);
  server.on("/off", handleRelayOff);
  server.begin();
}

void loop() {
  server.handleClient();
  unsigned long currentMillis = millis();

  if (currentMillis - lastMillis >= interval) {
    detachInterrupt(SENSOR_PIN);

    flowRate = (pulseCount / calibrationFactor);          // L/min
    totalLiters += (flowRate / 60.0);                     // L added per second
    totalUnits = totalLiters / 1000;

    float flowRateM3ps = flowRate / 1000.0 / 60.0;
    velocity = flowRateM3ps / pipeArea;

    float rho = 997.0;
    float pressurePa = 0.5 * rho * velocity * velocity;
    pressureBar = pressurePa / 100000.0;

    preferences.putInt("units", totalUnits);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Flow:");
    lcd.print(flowRate, 1);
    lcd.print("L/m");

    lcd.setCursor(0, 1);
    lcd.print("P:");
    lcd.print(pressureBar, 2);
    lcd.print(" bar");

    pulseCount = 0;
    lastMillis = currentMillis;
    attachInterrupt(digitalPinToInterrupt(SENSOR_PIN), pulseCounter, FALLING);
  }
}
