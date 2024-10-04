#include <Arduino.h>
#include "BLEDevice.h"
#include "BLEUtils.h"
#include "BLEServer.h"
#include "BLE2902.h"


// Define the name of the device
constexpr char deviceName[] = "ESP32 BLE Joystick";

// Define the UUIDs for the service and the characteristic
// See the following for generating UUIDs: https://www.uuidgenerator.net/
constexpr char serviceUUID[] = "91bad492-b950-4226-aa2b-4ede9fa42f59";
constexpr char characteristicUUID[] = "cba1d466-344c-4be3-ab3f-189f80dd7518";

// Create data characteristic
BLECharacteristic dataCharacteristic(characteristicUUID, BLECharacteristic::PROPERTY_NOTIFY);


// Flag to check if a device is connected
bool deviceConnected = false;
bool gotDisconnected = false;
// Setup callbacks onConnect and onDisconnect to update the deviceConnected flag
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    gotDisconnected = true;
  }
};


// define variables for joystick input
void getJoystickInput();
const uint8_t xPin = 14;
const uint8_t yPin = 13;
uint16_t xValue = 0;
uint16_t yValue = 0;
uint16_t midVal = 1920;
uint16_t deadzone = 200;
uint8_t direction = 0;
uint8_t prevDirection = 0;


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

  // Setup advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(serviceUUID);
  // Start advertising
  BLEDevice::startAdvertising();
  //pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");

}


void loop() {

  if (deviceConnected)
    Serial.println("Device connected!");
  else
    Serial.println("Waiting for device connection...");
  
  while (deviceConnected) 
  {
    getJoystickInput();
    Serial.print("Send direction: ");
    Serial.println(direction);
    dataCharacteristic.setValue(&direction, sizeof(uint8_t));
    dataCharacteristic.notify();
    delay(50);
  }
  
  if (gotDisconnected)
  {
    Serial.println("Device disconnected!");
    Serial.println("Restarting advertising...");
    BLEDevice::startAdvertising();
    gotDisconnected = false;
    direction = 0;
  }

  delay(5000); 
}


void getJoystickInput()
{
  // read analog X and Y values
  xValue = analogRead(xPin);
  yValue = analogRead(yPin);
  // map analog values to directions
  if (xValue < (midVal - deadzone) && prevDirection != 3) 
    direction = 1; // left
  else if (yValue < (midVal - deadzone) && prevDirection != 4)
    direction = 2; // up
  else if (xValue > (midVal + deadzone) && prevDirection != 1)
    direction = 3; // right
  else if (yValue > (midVal + deadzone) && prevDirection != 2)
    direction = 4; // down
  // remember previous direction
  prevDirection = direction;
}
