#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

const char* SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214";  // BLE service UUID
const char* ACC_CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214";  // BLE characteristic UUID for acceleration
const char* GYRO_CHARACTERISTIC_UUID = "19B10002-E8F2-537E-4F6C-D104768A1214";  // BLE characteristic UUID for gyroscope

BLEUUID serviceUUID(SERVICE_UUID);
BLEUUID accCharUUID(ACC_CHARACTERISTIC_UUID);
BLEUUID gyroCharUUID(GYRO_CHARACTERISTIC_UUID);

BLEScan* pBLEScan;
BLERemoteService* pRemoteService;
BLEAdvertisedDevice* pConnectedDevice;
BLEClient* pClient = BLEDevice::createClient();
BLERemoteCharacteristic* pAccCharacteristic;
BLERemoteCharacteristic* pGyroCharacteristic;
bool isConnected = false;
bool connectToDevice1 = true; // Flag to indicate which device to connect to
bool connectToDevice2 = false;
bool connectToDevice3 = false;
bool dev3Timer = false;
unsigned long startTime = 0;
unsigned long pausedTime = 0;

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
private:
  BLEAdvertisedDevice* pDevice1 = nullptr;
  BLEAdvertisedDevice* pDevice2 = nullptr;
  BLEAdvertisedDevice* pDevice3 = nullptr;

public:
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getAddress().toString() == "8f:27:e9:28:44:4f") {
      pDevice1 = new BLEAdvertisedDevice(advertisedDevice);
    } else if (advertisedDevice.getAddress().toString() == "98:a9:7e:e3:fd:e8") {
      pDevice2 = new BLEAdvertisedDevice(advertisedDevice);
    } else if (advertisedDevice.getAddress().toString() == "b2:50:16:6a:c5:a0") {
      pDevice3 = new BLEAdvertisedDevice(advertisedDevice);
    }
  }

  BLEAdvertisedDevice* getDevice1() {
    return pDevice1;
  }

  BLEAdvertisedDevice* getDevice2() {
    return pDevice2;
  }

  BLEAdvertisedDevice* getDevice3() {
    return pDevice3;
  }
};

MyAdvertisedDeviceCallbacks deviceCallbacks;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(&deviceCallbacks);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5);

  Serial.println("Scanning for BLE devices...");
  while (!deviceCallbacks.getDevice1() || !deviceCallbacks.getDevice2() || !deviceCallbacks.getDevice3()) {
    delay(500);
  }
  Serial.println("Devices found!");

  // Start the initial connection
  connectToDevice1 = true;
  connectToDevice2 = false;
  connectToDevice3 = false;
  connectToNano33();
}

void connectToNano33() {
  if (pClient) {
    // Disconnect from the current device if it's connected
    pClient->disconnect();
    delay(100); // Wait for disconnection to complete
  }

  if (connectToDevice1) {
    pConnectedDevice = deviceCallbacks.getDevice1();
  } else if (connectToDevice2){
    pConnectedDevice = deviceCallbacks.getDevice2();
  } else if (connectToDevice3) {
    pConnectedDevice = deviceCallbacks.getDevice3();
    dev3Timer = true;
  }

  if (pConnectedDevice) {
    pClient->connect(pConnectedDevice);
    pRemoteService = pClient->getService(serviceUUID);
    if (pRemoteService) {
      pAccCharacteristic = pRemoteService->getCharacteristic(accCharUUID);
      pGyroCharacteristic = pRemoteService->getCharacteristic(gyroCharUUID);
      if (pAccCharacteristic && pGyroCharacteristic) {
        pAccCharacteristic->registerForNotify(onAccNotify);
        pGyroCharacteristic->registerForNotify(onGyroNotify);
        isConnected = true; // Mark the connection as successful
      }
    }
  }
}

void reconnectIfDisconnected() {
  if (!isConnected) {
    Serial.println("Device disconnected. Reconnecting...");
    
    // Disconnect from the current device if it's connected
    if (pClient->isConnected()) {
      pClient->disconnect();
      delay(500); // Wait for disconnection to complete
    }
    
    // Switch the device connection if needed
    switchDeviceConnection();
  }
}

void onAccNotify(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (!isConnected) return; // Ignore data if not connected
  size_t numFloats = length / sizeof(float);
  if (numFloats == 4) {
    //Serial.print("imu data: ");
    for (size_t i = 0; i < 4; i++) {
      float value;
      memcpy(&value, &pData[i * sizeof(float)], sizeof(float));
      Serial.print(value, 2); // Print with 2 decimal places
      Serial.print(",");
    }
  } else {
    Serial.println("Received unexpected data length (Accelerometer)");
  }
}

void onGyroNotify(BLERemoteCharacteristic* pCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  if (!isConnected) return; // Ignore data if not connected
  size_t numFloats = length / sizeof(float);
  if (numFloats == 3) {
    Serial.print(" ");
    for (size_t i = 0; i < 3; i++) {
      float value;
      memcpy(&value, &pData[i * sizeof(float)], sizeof(float));
      Serial.print(value, 2); // Print with 2 decimal places
      Serial.print(",");
      if (dev3Timer)
      {
        unsigned long currentTime = millis();
        unsigned long elapsedTime = currentTime - startTime - pausedTime;
        Serial.print(elapsedTime);
      }
    }
    Serial.println();
  } else {
    Serial.println("Received unexpected data length (Gyroscope)");
  }
}

void switchDeviceConnection() {
  if (connectToDevice1 || connectToDevice2 || connectToDevice3) {
    pClient->disconnect(); // Disconnect from the current device
    delay(500); // Wait for disconnection to complete
    isConnected = false;
  }

  // Switch the device connection
  if (connectToDevice1) {
    connectToDevice1 = false;
    connectToDevice2 = true;
  } else if (connectToDevice2) {
    connectToDevice2 = false;
    connectToDevice3 = true;
  } else if (connectToDevice3) {
    connectToDevice3 = false;
    connectToDevice1 = true;
    if(dev3Timer)
    {
      pausedTime = millis() - startTime;
      dev3Timer = false;
    }
  }

  // Start the connection for the new device
  connectToNano33();
}

void loop() {
  // Non-blocking device switching
  static unsigned long lastDeviceSwitchTime = 0;
  const unsigned long deviceSwitchInterval = 5000; // Switch devices every 5 seconds

  if (isConnected && millis() - lastDeviceSwitchTime >= deviceSwitchInterval) {
    switchDeviceConnection();
    lastDeviceSwitchTime = millis();
  }
  reconnectIfDisconnected();
}
