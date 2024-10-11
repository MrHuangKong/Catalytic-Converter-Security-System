// Nicolas Huang
// 4/1/2023
// Sensor Connect to Hub

#include <ArduinoBLE.h>

// Bluetooth Low Energy LED Service
BLEService sensorService("PLACE UUID HERE"); 

// Bluetooth Low Energy Battery Characteristic - custom 128-bit UUID, read only
BLEByteCharacteristic batteryCharacteristic("PLACE UUID HERE", BLERead);

// Set LED Pins
#define ledB LEDB
#define ledR LED_BUILTIN

void setup() {
  Serial.begin(9600);
  // while (!Serial);  // Wait for Serial connection

  // set LED pins to output mode
  pinMode(ledB, OUTPUT);
  pinMode(ledR, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth Low Energy module failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Sensor");
  BLE.setAdvertisedService(sensorService);

  // add the characteristic to the service
  sensorService.addCharacteristic(batteryCharacteristic);

  // add service
  BLE.addService(sensorService);

  // set the initial value for the characeristic:
  batteryCharacteristic.writeValue(100);

  // start advertising
  BLE.advertise();

  Serial.println("BLE Sensor");
}

void loop() {
  // listen for BLE clients to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    // print the central's MAC address:
    Serial.print("Connected to central hub: ");
    Serial.println(central.address());

  // while the central is still connected to peripheral:
    while (central.connected()) {
      digitalWrite(ledB, LOW);  // Set blue led to on
      // Enter battery reading code
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central hub: "));
    Serial.println(central.address());
  }
  // Flash Red LED for disconnection
  digitalWrite(ledB, HIGH);
  digitalWrite(ledR, HIGH);
  delay(50);
  digitalWrite(ledR, LOW);
  delay(50);
  digitalWrite(ledR, HIGH);
  delay(50);
}