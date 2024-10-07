#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

// Wi-Fi credentials
const char* ssid = "Apple pie";
const char* password = "passWord@69";

// Create an ADXL345 sensor instance
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(12345);

// Threshold for detecting strong movement (adjust based on testing)
const float earthquakeThreshold = 10.5;  // g value, adjust this based on tests

// Pin Definitions for Room 1
const int flameSensorPin1 = 13;
const int mq2Pin1 = 34;
const int mq7Pin1 = 35;

// Pin Definitions for Room 2
const int flameSensorPin2 = 32;
const int mq2Pin2 = 33;
const int mq7Pin2 = 27;

// Buzzer Pin
const int buzzerPin = 12;

// Gas sensor thresholds
const int mq2ThresholdLow = 300;
const int mq2ThresholdMedium = 600;
const int mq7ThresholdLow = 300;
const int mq7ThresholdMedium = 600;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// HTML & JavaScript for the dashboard
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <title>ESP32 Sensor Dashboard</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html { font-family: Arial; text-align: center; }
    h1 { font-size: 2.0rem; }
    p { font-size: 1.5rem; }
    .sensor { font-size: 1.2rem; margin-bottom: 20px; }
    .room { padding: 20px; margin-bottom: 30px; background-color: lightgray; }
    .fire-detected { background-color: red; color: white; }
    .earthquake-detected { background-color: orange; color: white; }
  </style>
</head>
<body>
  <h1>ESP32 Sensor Dashboard</h1>
  <div id="room1" class="room">
    <h2>Room 1</h2>
    <div class="sensor">Flame Sensor: <span id="flame1Value">No Fire</span></div>
    <div class="sensor">MQ2 Sensor: <span id="mq2Value1">Low</span></div>
    <div class="sensor">MQ7 Sensor: <span id="mq7Value1">Low</span></div>
  </div>
  <div id="room2" class="room">
    <h2>Room 2</h2>
    <div class="sensor">Flame Sensor: <span id="flame2Value">No Fire</span></div>
    <div class="sensor">MQ2 Sensor: <span id="mq2Value2">Low</span></div>
    <div class="sensor">MQ7 Sensor: <span id="mq7Value2">Low</span></div>
  </div>
  <div id="earthquake" class="room">
    <h2>Earthquake Status</h2>
    <div class="sensor">Earthquake: <span id="earthquakeStatus">No</span></div>
  </div>
  <script>
    setInterval(function () {
      fetch('/sensor')
        .then(response => response.json())
        .then(data => {
          // Update flame sensor values for Room 1
          document.getElementById('flame1Value').innerHTML = data.flame1 ? 'No Fire' : 'Fire Detected!';

          // Update MQ2 sensor for Room 1 (Low, Medium, High)
          document.getElementById('mq2Value1').innerHTML = data.mq2Value1 === 'Low' ? 'Low' : (data.mq2Value1 === 'Medium' ? 'Medium' : 'High');

          // Update MQ7 sensor for Room 1 (Low, Medium, High)
          document.getElementById('mq7Value1').innerHTML = data.mq7Value1 === 'Low' ? 'Low' : (data.mq7Value1 === 'Medium' ? 'Medium' : 'High');

          // Update flame sensor values for Room 2
          document.getElementById('flame2Value').innerHTML = data.flame2 ? 'No Fire' : 'Fire Detected!';

          // Update MQ2 sensor for Room 2 (Low, Medium, High)
          document.getElementById('mq2Value2').innerHTML = data.mq2Value2 === 'Low' ? 'Low' : (data.mq2Value2 === 'Medium' ? 'Medium' : 'High');

          // Update MQ7 sensor for Room 2 (Low, Medium, High)
          document.getElementById('mq7Value2').innerHTML = data.mq7Value2 === 'Low' ? 'Low' : (data.mq7Value2 === 'Medium' ? 'Medium' : 'High');

          // Earthquake detection status
          document.getElementById('earthquakeStatus').innerHTML = data.earthquake ? 'Yes' : 'No';

          // Only turn Room 1 red if a fire is detected
          document.getElementById('room1').classList.toggle('fire-detected', !data.flame1);
          
          // Only turn Room 2 red if a fire is detected
          document.getElementById('room2').classList.toggle('fire-detected', !data.flame2);
          
          // Show earthquake warning if detected
          document.getElementById('earthquake').classList.toggle('earthquake-detected', data.earthquake);
        });
    }, 1000);  // Update sensor values every second
  </script>
</body>
</html>
)rawliteral";

// Setup function
void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize ADXL345 accelerometer
  if (!accel.begin()) {
    Serial.println("Failed to initialize ADXL345");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);

  // Set up flame and gas sensor pins
  pinMode(flameSensorPin1, INPUT);
  pinMode(mq2Pin1, INPUT);
  pinMode(mq7Pin1, INPUT);
  pinMode(flameSensorPin2, INPUT);
  pinMode(mq2Pin2, INPUT);
  pinMode(mq7Pin2, INPUT);

  // Set up buzzer pin
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);  // Turn off buzzer initially

  // Serve the HTML file
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  // Route to get sensor data
  server.on("/sensor", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Sensor data for Room 1 and Room 2
    int flameState1 = digitalRead(flameSensorPin1);
    int mq2Value1 = analogRead(mq2Pin1);
    int mq7Value1 = analogRead(mq7Pin1);
    int flameState2 = digitalRead(flameSensorPin2);
    int mq2Value2 = analogRead(mq2Pin2);
    int mq7Value2 = analogRead(mq7Pin2);

    // Earthquake detection
    sensors_event_t event;
    accel.getEvent(&event);
    bool earthquakeDetected = abs(event.acceleration.x) > earthquakeThreshold || 
                              abs(event.acceleration.y) > earthquakeThreshold || 
                              abs(event.acceleration.z) > earthquakeThreshold;

    // Convert gas sensor values to Low, Medium, High
    String mq2Status1 = (mq2Value1 < mq2ThresholdLow) ? "Low" : (mq2Value1 < mq2ThresholdMedium) ? "Medium" : "High";
    String mq7Status1 = (mq7Value1 < mq7ThresholdLow) ? "Low" : (mq7Value1 < mq7ThresholdMedium) ? "Medium" : "High";
    String mq2Status2 = (mq2Value2 < mq2ThresholdLow) ? "Low" : (mq2Value2 < mq2ThresholdMedium) ? "Medium" : "High";
    String mq7Status2 = (mq7Value2 < mq7ThresholdLow) ? "Low" : (mq7Value2 < mq7ThresholdMedium) ? "Medium" : "High";

    // Prepare JSON response
    String json;
    DynamicJsonDocument doc(1024);
    doc["flame1"] = flameState1;
    doc["mq2Value1"] = mq2Status1;
    doc["mq7Value1"] = mq7Status1;
    doc["flame2"] = flameState2;
    doc["mq2Value2"] = mq2Status2;
    doc["mq7Value2"] = mq7Status2;
    doc["earthquake"] = earthquakeDetected;
    serializeJson(doc, json);
    request->send(200, "application/json", json);
  });

  // Start server
  server.begin();
}

void loop() {
  // Your main loop code here, if needed
}
