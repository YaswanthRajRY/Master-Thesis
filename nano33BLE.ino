#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>

// BLE service and characteristic UUIDs
BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1214"); 
BLECharacteristic accDataCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLENotify, 24);
BLECharacteristic gyroDataCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLENotify, 24);
BLEDescriptor imuDataDescriptor("2902", "Client Characteristic Configuration");

bool isConnected = false;

// Kalman filter structure
struct KalmanFilter {
  float Q; // Process noise covariance
  float R; // Measurement noise covariance
  float X; // Value
  float P; // Estimation error covariance
  float K; // Kalman gain

  KalmanFilter(float q, float r, float initial_value) {
    Q = q;
    R = r;
    X = initial_value;
    P = 1.0;
    K = 0.0;
  }

  float update(float measurement) {
    // Prediction update
    P = P + Q;

    // Measurement update
    K = P / (P + R);
    X = X + K * (measurement - X);
    P = (1 - K) * P;

    return X;
  }
};

// Initialize Kalman filters for accelerometer and gyroscope axes
KalmanFilter kalmanAccX(0.01, 0.1, 0.0);
KalmanFilter kalmanAccY(0.01, 0.1, 0.0);
KalmanFilter kalmanAccZ(0.01, 0.1, 0.0);

KalmanFilter kalmanGyroX(0.01, 0.1, 0.0);
KalmanFilter kalmanGyroY(0.01, 0.1, 0.0);
KalmanFilter kalmanGyroZ(0.01, 0.1, 0.0);

void setup() {
  Serial.begin(9600);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  BLE.begin();
  BLE.setLocalName("Nano33BLE_IMU3");
  BLE.setAdvertisedService(imuService);

  accDataCharacteristic.addDescriptor(imuDataDescriptor);
  imuService.addCharacteristic(accDataCharacteristic);
  BLE.addService(imuService);

  gyroDataCharacteristic.addDescriptor(imuDataDescriptor);
  imuService.addCharacteristic(gyroDataCharacteristic);
  BLE.addService(imuService);

  BLE.advertise();

  accDataCharacteristic.writeValue((uint8_t*)nullptr, 0);
  gyroDataCharacteristic.writeValue((uint8_t*)nullptr, 0);

  Serial.println("BLE peripheral started.");
}

void loop() {
  if (BLE.connected()) {
    float accData[4];
    float rawAccX, rawAccY, rawAccZ;

    IMU.readAcceleration(rawAccX, rawAccY, rawAccZ);

    // Apply Kalman filter to accelerometer data
    accData[0] = 3.00; // indicates nano device number (ex. 1, 2, 3)
    accData[1] = kalmanAccX.update(rawAccX);
    accData[2] = kalmanAccY.update(rawAccY);
    accData[3] = kalmanAccZ.update(rawAccZ);

    Serial.print(accData[0]);
    Serial.print(" ");
    Serial.print(accData[1]);
    Serial.print(" ");
    Serial.print(accData[2]);
    Serial.print(" ");
    Serial.print(accData[3]);
    Serial.print(" ");

    accDataCharacteristic.writeValue(reinterpret_cast<uint8_t*>(accData), sizeof(accData));

    float gyroData[3];
    float rawGyroX, rawGyroY, rawGyroZ;

    IMU.readGyroscope(rawGyroX, rawGyroY, rawGyroZ);

    // Apply Kalman filter to gyroscope data
    gyroData[0] = kalmanGyroX.update(rawGyroX);
    gyroData[1] = kalmanGyroY.update(rawGyroY);
    gyroData[2] = kalmanGyroZ.update(rawGyroZ);

    Serial.print(gyroData[0]);
    Serial.print(" ");
    Serial.print(gyroData[1]);
    Serial.print(" ");
    Serial.print(gyroData[2]);
    Serial.println();

    gyroDataCharacteristic.writeValue(reinterpret_cast<uint8_t*>(gyroData), sizeof(gyroData));

    delay(50);
  }
}

