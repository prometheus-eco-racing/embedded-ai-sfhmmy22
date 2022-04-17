#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define dirPIN1 5
#define dirPIN2 6
#define PWM 2
#define totalMeasurements 10000

Adafruit_MPU6050 mpu;
int measurementsCount = 0;

void setup() {
  pinMode(dirPIN1, OUTPUT);
  pinMode(dirPIN2, OUTPUT);
  pinMode(PWM, OUTPUT);
  
  Serial.begin(115200);
  while (!Serial)
    delay(10); // pause until serial console opens

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  // Accelerometer Settings
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.print("aX,aY,aZ,gX,gY,gZ");

  analogWrite(PWM, 85);     // Define Speed (values in [0, 255])
  
  // Define Direction of Rotation
  digitalWrite(dirPIN1, HIGH);
  digitalWrite(dirPIN2, LOW);

  // Wait to reach nominal velocity
  delay(8000);
}

void loop() {
  while (measurementsCount++ < totalMeasurements) {
    // Get new sensor events with the readings
    // accelerometer, gyroscope, temperature
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);
    
    // Print to .csv format
    Serial.println();
    Serial.print(a.acceleration.x);
    Serial.print(", ");
    Serial.print(a.acceleration.y);
    Serial.print(", ");
    Serial.print(a.acceleration.z);
    Serial.print(", ");
  
    Serial.print(g.gyro.x);
    Serial.print(", ");
    Serial.print(g.gyro.y);
    Serial.print(", ");
    Serial.print(g.gyro.z);
  
    delay(4);   // Sampling Frequency = 1kHz
  }
   analogWrite(PWM, 0);   // Turn off the Motor
}
