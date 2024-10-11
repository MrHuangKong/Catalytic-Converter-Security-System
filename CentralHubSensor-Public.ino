// Nicolas Huang
// 3/19/2023
// Central Hub Sensor for Anti-Theft Catalyctic Converter

// Define Blynk Cloud Parameters
#define BLYNK_TEMPLATE_ID "PLACE TEMPLATE_ID HERE"
#define BLYNK_TEMPLATE_NAME "Catalytic Converter Security System"
#define BLYNK_AUTH_TOKEN "PLACE BLYNK AUTH_TOKEN HERE"

// Define GSM Modem
#define TINY_GSM_MODEM_SIM7000

// Import Libraries
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
#include <Arduino.h>
#include <BLEDevice.h>

// Define Pin Layout
#define SerialAT Serial1  // Define SerialAT command monitor
#define UART_BAUD   115200
#define PIN_DTR     25
#define PIN_TX      27
#define PIN_RX      26
#define PWR_PIN     4
#define LED_PIN     12  
#define TIMEOUT_LENGTH 60000  // Set timeout length to 60 seconds 

// Auth Token
char auth[] = BLYNK_AUTH_TOKEN;

BlynkTimer timer;  // Create timer to not spam server

// GSM APN Credentials
char apn[]  = "hologram";
char user[] = "";
char pass[] = "";

// BLE Sensor UUID's to Connect to 
static BLEUUID serviceUUID("PLACE CAT SERVICE UUID HERE");
static BLEUUID charUUID("PLACE CAT NAME UUID HERE");

// Create necessary BLE objects to store info
BLEClient* pClient = nullptr;
BLEAdvertisedDevice* sensor = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristic = nullptr;

// Create BLE Condition States
bool deviceFound = false;
bool scanning = false;
bool connected = false;

// ------------------------
// BLE Classes & Functions
// ------------------------

// Called by Client when Connecting/Disconnecting to Server
class ClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pclient) {
  }

  void onDisconnect(BLEClient* pclient) {
    connected = false;
    Serial.println("onDisconnect");
  }
};

// Called by the Scanner to check if Service UUID's are correct
class AdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  // Callback for each server that is connected to
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    // Print and check if the BLE Device we found is the right one, if so stop scan
    Serial.print("BLE Device found: ");
    Serial.println(advertisedDevice.toString().c_str());

    if (advertisedDevice.haveServiceUUID() && advertisedDevice.isAdvertisingService(serviceUUID)) {
      BLEDevice::getScan()->stop();
      sensor = new BLEAdvertisedDevice(advertisedDevice);  // Save the advertised BLE device
      deviceFound = true;
      scanning = true;
    }
  } 
};

/**
  Connects to scanned peripheral and guarantees if it is the designated peripheral we're searching for
  
  @return If we're able to connect to correct peripheral/server
*/
bool connectToServer() {
  Serial.printf("Forming connection to device: %s \n", sensor->getAddress().toString().c_str());
  pClient = BLEDevice::createClient();  // Create BLEClient instance
  pClient->setClientCallbacks(new ClientCallback());
  
  // Connect client to server
  pClient->connect(sensor);
  Serial.println("Connected to Sensor!");

  // Save peripheral/server's service id
  BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {  // Service id not found
    auto serviceID = serviceUUID.toString();
    Serial.print("Unable to find Service ID: ");
    Serial.println(serviceID.c_str());
    pClient->disconnect();
    Blynk.virtualWrite(V3, "Not Connected");
    return false;
  }
  else {
    Serial.println("Service ID found...");
  }

  // Save peripheral/server's characteristic id
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr) {  // Characteristic id not found
    std::string charID = charUUID.toString();
    Serial.print("Unable to find Characteristic ID: ");
    Serial.println(charID.c_str());
    pClient->disconnect();
    
    return false;
  }
  else {
    Serial.println("Characteristic ID found...");
  }

  if(pRemoteCharacteristic->canRead()) {
    std::string charID = pRemoteCharacteristic->readValue();
    Serial.printf("Characteristic ID is: ");
    Serial.println(charID.c_str());
  }

  connected = true;  // Change our connected status
  return true;
}
// ------------------------
// ------------------------


// Set modem to communicate with SerialAT monitor
TinyGsm modem(SerialAT);

// Sync all vpin status with server
BLYNK_CONNECTED() {
Blynk.syncVirtual(V0);  // latitude pin 
Blynk.syncVirtual(V1);  // longitude pin
Blynk.syncVirtual(V2);  // reload/request gps button
Blynk.setProperty(V2, "color", "#64C466");  // Set color to green
Blynk.syncVirtual(V3);  // sensor pin status
}


// ------------------------
// Blynk vpin functions
// ------------------------

/**
  Request GPS Coordinates (Figure out a way to prevent spamming)
  @param V2 Virtual pin
*/
BLYNK_WRITE(V2) {
  // Enable GPS, grab longitude and latitude 
  if (param.asInt() == 1) {
    // Store lat and lon values
    float lat, lon;

    // Create string buffer for latitude and longitude
    char lat_buffer[11];
    char lon_buffer[11];

    // Change button appearance
    Blynk.setProperty(V2, "offLabel", "Please Wait");  // Change button to display "Please wait"
    Blynk.setProperty(V2, "color", "#EB4D3D");  // Set color to red
    
    // Get the current GPS coordinates
    queryGPS(lat, lon);

    // Convert floats to strings (float val, min. string length, precision #, buffer)
    dtostrf(lat, 9, 6, lat_buffer);
    dtostrf(lon, 9, 6, lon_buffer);

    Blynk.setProperty(V2, "offLabel", "Reload GPS");  // Change button label back
    Blynk.setProperty(V2, "color", "#64C466");  // Set color to green
    Serial.printf("lat: %f, lon: %f\n", lat, lon);
    Serial.write(lat_buffer, strlen(lat_buffer));
    Serial.write(lon_buffer, strlen(lon_buffer));
    Blynk.virtualWrite(V0, lat_buffer);  // Write latitude value to lat pin
    Blynk.virtualWrite(V1, lon_buffer); // Write longitude value to lon pin

  }
}

/**
  Queries GPS coordinates by setting antenna to active

  @param lat Variable to hold the latitude coordinates
  @param lon Variable to hold the longitude coordinates
*/
void queryGPS(float &lat, float &lon) {
  int start_time = millis(); // Save beginning time
    modem.sendAT("+SGPIO=0,4,1,1");  // Set pin 4 (GPS antenna) to high
    if (modem.waitResponse(10000L) != 1) {
      DBG(" SGPIO=0,4,1,1 false ");
    }
    modem.enableGPS();
    while (1) {
      int current_time = millis();  // Save current time to calculate timeout
      if (modem.getGPS(&lat, &lon)) {
        break;
      } 
      else {
        Serial.print("getGPS ");
        Serial.println(millis());
        if ((current_time - start_time) > TIMEOUT_LENGTH) {
          Serial.println("GPS TIMEOUT...");
          Blynk.virtualWrite(V0, "ERROR:");  // Write latitude value to lat pin
          Blynk.virtualWrite(V1, "Time Out");  // Write to longitude pin
          return;  // Break out of GPS call function
        } 
        
      }
      delay(2000);
    }
  modem.disableGPS();
  
  modem.sendAT("+SGPIO=0,4,1,0");  // Set antenna to low
  if (modem.waitResponse(10000L) != 1) {
    DBG(" SGPIO=0,4,1,0 false ");
  }
}

void setup() {

  // Set LED Pin out, and to off
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  
  Serial.begin(115200); // Set console baud rate
  Serial.println("\nWaiting...");
  delay(3000);  // Wait for 3 seconds

  // Start monitor
  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

  // Start up Modem connection
  // Use init() if it takes too long
  Serial.println("Initializing modem...");
  if (!modem.init()) {
    Serial.println("Failed to reset modem, attempting to continue without resetting");
  }

  String name = modem.getModemName();
  delay(500);
  Serial.println("Modem Name: " + name);

  // Set modem parameters
  // modem.setNetworkMode(38);  // LTE only
  // modem.setPreferredMode(3);  // NB-IoT mode only

  // Wait for connection to network
  Serial.println("Waiting for network connection...");
  if (!modem.waitForNetwork()) {
    delay(5000);
    return;
  }

  // Check if we have GSM network connection
  if (modem.isNetworkConnected()) {
    Serial.println("Network connected");
    // Start session with Blynk server
    Blynk.begin(auth, modem, apn, user, pass);

    // Set LED to on for good connection
    digitalWrite(LED_PIN, LOW);
  }
  else { 
    // Blink LED 3 times for bad connection, restart modem
    for (int i = 0; i < 3; i++) {
      digitalWrite(LED_PIN, LOW);
      delay(250);
      digitalWrite(LED_PIN, HIGH);
      delay(250);
    }
    Serial.print("Unable to connect to network, please restart...");
  }

  // Set Sensor Status pin to unconnected:
  Blynk.virtualWrite(V3, "Not Connected");

  // Begin BLE Client, and establish connection to sensor
  Serial.println("Starting BLE Client Session...\n");
  BLEDevice::init("");  // Call constructor for client

  // Set up a scanner, where we will set to active scan status for 5 seconds
  BLEScan* pBLEScan = BLEDevice::getScan();

  // Update Sensor Status to searching...
  Blynk.virtualWrite(V3, "Searching...");
  // Set scanners parameters
  pBLEScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
  pBLEScan->setInterval(1500);  // in ms
  pBLEScan->setWindow(500);
  pBLEScan->setActiveScan(true);
  pBLEScan->start(5, false);

  if (!connected) {
    Blynk.virtualWrite(V3, "Not Connected");
    delay(500);
    scanning = true;
  }
}

void loop() {
  // Establish connection to Blynk Server
  Blynk.run();

  // Prevent calls from spamming to server
  timer.run();

  // Update GPS Coordinates Every 15 minutes (900,000ms) to conserve battery
  if (millis() % 900000) {
    // Store lat and lon values
    float lat, lon;

    // Create string buffer for latitude and longitude
    char lat_buffer[11];
    char lon_buffer[11];

    // Change button appearance
    Blynk.setProperty(V2, "offLabel", "Please Wait");  // Change button to display "Please wait"
    Blynk.setProperty(V2, "color", "#EB4D3D");  // Set color to red
    
    // Get the current GPS coordinates
    queryGPS(lat, lon);

    // Convert floats to strings (float val, min. string length, precision #, buffer)
    dtostrf(lat, 9, 6, lat_buffer);
    dtostrf(lon, 9, 6, lon_buffer);

    Blynk.setProperty(V2, "offLabel", "Reload GPS");  // Change button label back
    Blynk.setProperty(V2, "color", "#64C466");  // Set color to green
    Serial.printf("lat: %f, lon: %f\n", lat, lon);
    Serial.write(lat_buffer, strlen(lat_buffer));
    Serial.write(lon_buffer, strlen(lon_buffer));
    Blynk.virtualWrite(V0, lat_buffer);  // Write latitude value to lat pin
    Blynk.virtualWrite(V1, lon_buffer); // Write longitude value to lon pin    
  }

  // Begin BLE Connection portion
  // If peripheral is found, then connect to it
  if (deviceFound) {
    if (connectToServer()) {  // Connect to server/peripheral
      Serial.println("Connected to Sensor...");
      Blynk.virtualWrite(V3, "Connected");
    }
    else {
      Serial.println("Unable to connect to Sensor, please get within range or restart!");
    }

    deviceFound = false;
  }

  // If sensor isn't found the first time, do a preliminary scan again
  if (!connected) {
    Serial.println("CHECK 1");
    scanning = true;
    Blynk.virtualWrite(V3, "Searching...");
  }  
  // If we are connected to server, read, or write to its characteristic
  if (connected) {
    if (!(millis() % 500)) {
      // Gather Signal Strength every half second
      int rssi = pClient->getRssi();
      Serial.println("RSSI: " + String(rssi));
      Serial.println(rssi);

      // Sensor distance is too far
      if (rssi < -110) {  // Measure signal strength of sensor under car ranges from -72 to -92
        Blynk.logEvent("send_alert");
      }
      
      Blynk.virtualWrite(V3, "Connected");
    }

    scanning = false;
  }
  else if (scanning) {  // If we aren't connected, continue scanning
    Serial.println("CHECK 2");
    Blynk.virtualWrite(V3, "Searching...");
    Blynk.logEvent("sensor_disconnected");  
    BLEDevice::getScan()->start(0);
    delay(1000);
  }
}
