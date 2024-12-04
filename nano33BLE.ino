#include <Arduino_LSM9DS1.h>
#include <ArduinoBLE.h>

BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1214");  // BLE service UUID
BLECharacteristic accDataCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLENotify, 24);  // BLE characteristic UUID for acceleration
BLECharacteristic gyroDataCharacteristic("19B10002-E8F2-537E-4F6C-D104768A1214", BLENotify, 24);  // BLE characteristic UUID for gyroscope
BLEDescriptor imuDataDescriptor("2902", "Client Characteristic Configuration"); // BLE2902 descriptor for notifications

bool isConnected = false;

void setup() {
  Serial.begin(9600);
  //while (!Serial);

  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  BLE.begin();
  BLE.setLocalName("Nano33BLE_IMU3");
  BLE.setAdvertisedService(imuService);

  accDataCharacteristic.addDescriptor(imuDataDescriptor); // Add the BLE2902 descriptor for notifications
  imuService.addCharacteristic(accDataCharacteristic);
  BLE.addService(imuService);

  gyroDataCharacteristic.addDescriptor(imuDataDescriptor); // Add the BLE2902 descriptor for notifications
  imuService.addCharacteristic(gyroDataCharacteristic);
  BLE.addService(imuService);

  BLE.advertise();

  accDataCharacteristic.writeValue((uint8_t*)nullptr, 0); // Initialize the characteristic with an empty array
  gyroDataCharacteristic.writeValue((uint8_t*)nullptr, 0); // Initialize the characteristic with an empty array

  Serial.println("BLE peripheral started.");
}

void loop() {
  if (BLE.connected()) {
    float accData[4];
    IMU.readAcceleration(accData[1], accData[2], accData[3]);
    accData[0] = 3.00;
    Serial.print(accData[0]); // first BLE device indication 
    Serial.print(" ");
    Serial.print(accData[1]);
    Serial.print(" ");
    Serial.print(accData[2]);
    Serial.print(" ");
    Serial.print(accData[3]);
    Serial.print(" ");

    accDataCharacteristic.writeValue(reinterpret_cast<uint8_t*>(accData), sizeof(accData));

    float gyroData[3];
    IMU.readGyroscope(gyroData[0], gyroData[1], gyroData[2]);
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
