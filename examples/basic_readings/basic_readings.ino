// Basic demo for accelerometer readings from Vertex_MPU6050
//
// Adapted from the Adafruit_MPU6050 "basic_readings" example
// Copyright (c) 2019 Bryan Siepert for Adafruit Industries (BSD license).
// Changes: one Vertex_MPU6050 include replaces the three original includes,
// the class is Vertex_MPU6050, and the startup message names this library.
// See LICENSE in the library folder for the full license text.

#include <Vertex_MPU6050.h>

Vertex_MPU6050 mpu;

void setup(void) {
  Serial.begin(115200);

  while (!Serial) {
    delay(10);
  }

  Serial.println("Vertex_MPU6050 test!");

  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");

    while (1) {
      delay(10);
    }
  }

  Serial.println("MPU6050 Found!");
  Serial.println("");
  delay(100);
}

void loop() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */

  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");

  Serial.print("Temperature: ");
  Serial.print(temp.temperature);
  Serial.println(" degC");

  Serial.println("");
  delay(500);
}
