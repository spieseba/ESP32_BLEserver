#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLE2902.h"


// Define the name of the device
constexpr char deviceName[] = "ESP32 Controller";
constexpr char characteristicName[] = "Data";

// Define the UUIDs for the service and the characteristic
// See the following for generating UUIDs: https://www.uuidgenerator.net/
constexpr char serviceUUID[] = "91bad492-b950-4226-aa2b-4ede9fa42f59";
constexpr char characteristicUUID[] = "cba1d466-344c-4be3-ab3f-189f80dd7518";

// Create data characteristic
BLECharacteristic dataCharacteristic(characteristicUUID, BLECharacteristic::PROPERTY_NOTIFY);


// Flag to check if a device is connected
bool deviceConnected = false;
// Setup callbacks onConnect and onDisconnect to update the deviceConnected flag
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};

// Initialize data to be sent - this is a counter in this example
uint8_t data = 0;

void setup() {
  Serial.begin(115200);

  // Create BLE Device
  BLEDevice::init(deviceName);
  
  // Create BLE Server
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService* pService = pServer->createService(serviceUUID);

  // Create BLE Characteristics
  pService->addCharacteristic(&dataCharacteristic);
  dataCharacteristic.addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

}


void loop() {
  
  if (deviceConnected)
    Serial.println("Device connected!");
  else
    Serial.println("Waiting for device connection...");
  
  while (deviceConnected) 
  {
    dataCharacteristic.setValue(&data, sizeof(uint8_t));
    dataCharacteristic.notify();
    Serial.print("Sent data: ");
    Serial.println(data);
    data++;

    delay(1000);
  }

  delay(5000);
}
