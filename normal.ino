#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>
#include <WiFi.h>

// Wi-Fi credentials
const char* ssid = "Apple pie";          // Your WiFi SSID
const char* password = "passWord@69";    // Your WiFi password

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

// Define threshold values for gas sensors (Adjust based on your sensor calibration)
const int mq2Threshold = 300;
const int mq7Threshold = 300;

void setup() {
  Serial.begin(115200);
  
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize the ADXL345 sensor
  if (!accel.begin()) {
    Serial.println("Failed to initialize ADXL345");
    while (1);
  }
  accel.setRange(ADXL345_RANGE_16_G);  

  // Set up pins for Room 1
  pinMode(flameSensorPin1, INPUT);
  pinMode(mq2Pin1, INPUT);
  pinMode(mq7Pin1, INPUT);

  // Set up pins for Room 2
  pinMode(flameSensorPin2, INPUT);
  pinMode(mq2Pin2, INPUT);
  pinMode(mq7Pin2, INPUT);

  // Set up pin for Buzzer
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);  // Turn buzzer off initially
}

void loop() {
  bool fireDetected = false;
  bool earthquakeDetected = false;

  // Get a new sensor event for earthquake detection
  sensors_event_t event;
  accel.getEvent(&event);

  // Print acceleration data
  Serial.print("X: "); Serial.print(event.acceleration.x); Serial.print(" ");
  Serial.print("Y: "); Serial.print(event.acceleration.y); Serial.print(" ");
  Serial.print("Z: "); Serial.println(event.acceleration.z);

  // Check if acceleration exceeds the threshold
  if (abs(event.acceleration.x) > earthquakeThreshold ||
      abs(event.acceleration.y) > earthquakeThreshold ||
      abs(event.acceleration.z) > earthquakeThreshold) {
    Serial.println("Earthquake Detected!");
    earthquakeDetected = true;
    digitalWrite(buzzerPin, HIGH); // Turn on buzzer
  } else {
    digitalWrite(buzzerPin, LOW); // Turn off buzzer
  }

  // ---- Room 1 ----
  int flameState1 = digitalRead(flameSensorPin1);
  int mq2Value1 = analogRead(mq2Pin1);
  int mq7Value1 = analogRead(mq7Pin1);

  Serial.println("---- Room 1 ----");
  Serial.print("Flame Sensor: ");
  Serial.println(flameState1 == LOW ? "Fire Detected!" : "No Fire");
  Serial.print("MQ2 Value: ");
  Serial.println(mq2Value1);
  Serial.print("MQ7 Value: ");
  Serial.println(mq7Value1);

  if (flameState1 == LOW) {
    Serial.println("** Fire detected in Room 1! **");
    fireDetected = true;
  }

  // ---- Room 2 ----
  int flameState2 = digitalRead(flameSensorPin2);
  int mq2Value2 = analogRead(mq2Pin2);
  int mq7Value2 = analogRead(mq7Pin2);

  Serial.println("---- Room 2 ----");
  Serial.print("Flame Sensor: ");
  Serial.println(flameState2 == LOW ? "Fire Detected!" : "No Fire");
  Serial.print("MQ2 Value: ");
  Serial.println(mq2Value2);
  Serial.print("MQ7 Value: ");
  Serial.println(mq7Value2);

  if (flameState2 == LOW) {
    Serial.println("** Fire detected in Room 2! **");
    fireDetected = true;
  }

  // Control the Buzzer based on fire detection
  if (fireDetected) {
    Serial.println("** Buzzer ON! **");
    for (int i = 0; i < 3; i++) {
      digitalWrite(buzzerPin, HIGH);
      delay(200);  // Reduce delay between buzzer beeps
      digitalWrite(buzzerPin, LOW);
      delay(200);
    }
  } else {
    digitalWrite(buzzerPin, LOW);
  }

  delay(500);  // Reduce delay between sensor checks
}
