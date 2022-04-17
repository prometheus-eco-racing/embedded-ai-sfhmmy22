#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "model.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

#define dirPIN1 5
#define dirPIN2 6
#define PWM 2
#define measurementsPerSample 10

Adafruit_MPU6050 mpu;
int measurementsRead = 0;

// global variables used for TensorFlow Lite (Micro)
tflite::MicroErrorReporter tflErrorReporter;

// pull in all the TFLM ops, you can remove this line and
// only pull in the TFLM ops you need, if would like to reduce
// the compiled size of the sketch.
tflite::AllOpsResolver tflOpsResolver;

const tflite::Model *tflModel = nullptr;
tflite::MicroInterpreter *tflInterpreter = nullptr;
TfLiteTensor *tflInputTensor = nullptr;
TfLiteTensor *tflOutputTensor = nullptr;

// Create a static memory buffer for TFLM, the size may need to
// be adjusted based on the model you are using
constexpr int tensorArenaSize = 8 * 1024;
byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));

const char *CLASS[] = {
  "balanced",
  "unbalanced"
};

void setup()
{
  pinMode(dirPIN1, OUTPUT);
  pinMode(dirPIN2, OUTPUT);
  pinMode(PWM, OUTPUT);

  Serial.begin(115200);
  while (!Serial)
    delay(10); // pause until serial console opens

  Wire.begin();
  
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1)
    {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // get the TFL representation of the model byte array
  tflModel = tflite::GetModel(model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema mismatch!");
    while (1)
      ;
  }

  // Create an interpreter to run the model
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, tensorArena, tensorArenaSize, &tflErrorReporter);

  // Allocate memory for the model's input and output tensors
  tflInterpreter->AllocateTensors();

  // Get pointers for the model's input and output tensors
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  // Start the motor and wait to reach nominal velocity
  analogWrite(PWM, 85);
  digitalWrite(dirPIN1, HIGH);
  digitalWrite(dirPIN2, LOW);
  Serial.print("Wait to reach input velocity");
  delay(8000);
}

void loop() {
  if (measurementsRead < measurementsPerSample) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    /************************************************\ 
    |* Fill code here: read from sensors and feed   *|
    |* the model's input                            *|
    \************************************************/

    //Hint: Each sample has 6*measurementsPerSample features
    
    //You need to fill the input tensor with the 6 measurement types 
    //(aX, aY, aZ, gX, gY, gZ) measurementsPerSample times. 

    //Remember you are inside a loop counting measurementsPerSample times.

    tflInputTensor->data.f[measurementsRead*6 + 0] = a.acceleration.x;
    tflInputTensor->data.f[measurementsRead*6 + 1] = a.acceleration.y;
    tflInputTensor->data.f[measurementsRead*6 + 2] = a.acceleration.z;
    tflInputTensor->data.f[measurementsRead*6 + 3] = g.gyro.x;
    tflInputTensor->data.f[measurementsRead*6 + 4] = g.gyro.y;
    tflInputTensor->data.f[measurementsRead*6 + 5] = g.gyro.z;

    measurementsRead++;

  }
  else {
    measurementsRead = 0;
          
    // Run inferencing
    TfLiteStatus invokeStatus = tflInterpreter->Invoke();
    if (invokeStatus != kTfLiteOk) {
      Serial.println("Invoke failed!");
      while (1)
        ;
      return;
    }

    /************************************************\ 
    |* Fill code here: fetch the model's prediction *|
    |* and print into serial the output class       *|
    \************************************************/
    
    //Step 1: Get the output tensor value from the model
    float prediction = tflOutputTensor->data.f[0];
    //Step 2: Based on the output value decide on the output class and print
    if(prediction>0.5){
      Serial.println("balanced");
      
    }
    else{
      Serial.println("unbalanced");
    }
    
  

  }
  delay(4);
}
