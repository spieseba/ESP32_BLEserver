#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLE2902.h"


# define ServerName "ESP32 Controller"

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/
#define SERVICE_UUID "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID "cba1d466-344c-4be3-ab3f-189f80dd7518"

BLECharacteristic dataCharacteristic(CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_NOTIFY);

bool deviceConnected = false;
//Setup callbacks onConnect and onDisconnect
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
  }
};


// joystick input
uint8_t input = 0;

void setup() {
  Serial.begin(115200);

  // Create BLE Device
  BLEDevice::init(ServerName);
  
  // Create BLE Server
  BLEServer* pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  BLEService* pService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics
  pService->addCharacteristic(&dataCharacteristic);
  dataCharacteristic.addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

}

uint8_t counter = 0;

void loop() {
  if (deviceConnected) {
    Serial.println("Device connected");
  }

  
  dataCharacteristic.setValue(&counter, sizeof(uint8_t));
  dataCharacteristic.notify();
  Serial.print("Counter: ");
  Serial.println(counter);
  counter++;

  delay(1000);
}
