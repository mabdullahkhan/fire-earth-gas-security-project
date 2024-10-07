#include <AFMotor.h>

// Define motor control for the motor driver shield
AF_DCMotor motor1(1, MOTOR12_1KHZ); 
AF_DCMotor motor2(2, MOTOR12_1KHZ);
AF_DCMotor motor3(3, MOTOR34_1KHZ);
AF_DCMotor motor4(4, MOTOR34_1KHZ);

// Define flame sensor pin
#define FLAME_SENSOR_PIN 2  // Digital pin for flame sensor

void setup() {
  Serial.begin(9600);

  // Setup flame sensor pin as input
  pinMode(FLAME_SENSOR_PIN, INPUT);

  // Initially move forward
  moveForward();
  Serial.println("Setup complete. Motor moving forward...");
}

void loop() {
  // Read the flame sensor value (digital)
  int flameDetected = digitalRead(FLAME_SENSOR_PIN);
  
  // Print flame sensor status for debugging
  Serial.print("Flame Sensor Status: ");
  Serial.println(flameDetected);
  
  // If flame is detected (LOW signal), stop the motors
  if (flameDetected == LOW) {
    moveStop();  // Stop motors
    Serial.println("Flame detected! Motor stopped.");
  } 
  // If flame is not detected, move the motors forward
  else {
    moveForward();  // Resume moving forward
    Serial.println("No flame detected. Motor moving forward.");
  }
  
  delay(100);  // Small delay to avoid rapid polling
}

void moveStop() {
  motor1.run(RELEASE); 
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
} 
  
void moveForward() {
  motor1.run(FORWARD);      
  motor2.run(FORWARD);
  motor3.run(FORWARD); 
  motor4.run(FORWARD);     
  
  // Set motor speed
  motor1.setSpeed(190);  // Adjust speed value if needed
  motor2.setSpeed(190);
  motor3.setSpeed(190);
  motor4.setSpeed(190);
}
