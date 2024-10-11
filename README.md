# Catalytic Converter Security Device

## Overview
This project addresses the **theft of catalytic converters**, which has increased by over **1,215%** from 2019 to 2022. The solution is a _**proof of concept**_, which consists of two main components: a **central hub** and a **sensor**, which work together to detect unauthorized removal of the catalytic converter from a vehicle. The system uses **Bluetooth Low Energy (BLE)** for communication between the sensor and the hub, while the hub connects to a 3rd party mobile app through **4G LTE** to provide real-time updates. This device is designed to be **affordable**, **easy to install**, and **feature-rich**, with minimal maintenance required.

## Motivation
With the theft of catalytic converters on the rise and **replacement costs** ranging from **$2,000-$3,000**, this project aims to provide an affordable solution that offers:
- **Inexpensive setup (<$100)**
- **User-friendly installation**
- **Long battery life (1 year+)**
- **Real-time GPS tracking**
- **Notifications via mobile app**

Catalytic converter theft affects both individuals and institutions _(including my fellow classmates at Cap U!)_, making this project applicable for both **private car owners** and **fleet operators**.

## Features
- **GPS & Tracking:** Real-time GPS location of the vehicle, provided via 4G LTE, with updates sent periodically or triggered by potential theft.
- **Bluetooth Low Energy (BLE) Communication:** The central hub communicates with the sensor over BLE to monitor the catalytic converter's status. If the BLE signal weakens or disconnects, the hub notifies the user via the app.
- **3rd Party App Integration:** The hub connects to a mobile app (e.g., Blynk) to notify the user of the converter’s location and possible theft attempts.
- **Long Battery Life:** With present hardware, it is able to operate for more than a year without requiring frequent recharging.
- **Low Power Consumption:** The sensor uses **Bluetooth 5.0** and runs efficiently on minimal power.

## System Components

### Central Hub
The **central hub** is based on the **LilyGo T-SIM7000G** development board, which integrates:
- **SIM7000G** for 4G LTE (Cat-M & NB-IoT) network communication.
- **GPS module** for location tracking.
- **ESP32 microcontroller** for BLE communication and processing.

Key features of the central hub:
- **BLE Client Role:** The hub plays the client role, constantly monitoring the BLE connection to the sensor.
- **4G LTE for Notifications:** If the connection to the sensor weakens (potentially indicating tampering), the central hub sends an alert to the user via the mobile app.
- **Power Source:** The hub is powered via the vehicle's **12V car outlet** and a single **18650 9900mAh** battery, ensuring constant operation as long as the vehicle is in use.

Code: **`CentralHubSensor-Public.ino`**
- Sets up the LTE connection to send GPS data to the app.
- Manages BLE connection and scans for the sensor.
- Implements GPS functionality to track vehicle location.
- Communicates with the 3rd party app (Blynk) for real-time updates and notifications.

### Sensor
The **sensor** is built on the **Seeed Xiao nRF52840 Sense** board, featuring:
- **Bluetooth 5.0** for low-power communication with the central hub.
- **IMU (Inertial Measurement Unit):** Includes an accelerometer and gyroscope for detecting movement or tampering (not yet implemented).
  - Ability to use IMU data in machine learning to detect specific vibrations generated from saws via _TensorFlow Lite (not yet implemented)_ 
- **Low Power Consumption:** The sensor is designed to run on minimal power (80uA in idle mode), extending its battery life.

Key features of the sensor:
- **BLE Peripheral Role:** The sensor constantly broadcasts its signal, acting as a BLE server for the central hub to connect to.
- **Attachment to Catalytic Converter:** The sensor is securely attached to the catalytic converter via brackets, ensuring it withstands harsh conditions (up to 900°F).
- **Battery:** The sensor can be powered by a **CR2032 coin cell battery** or other long-life options, depending on user preference.

Code: **`CatSensor-Public.ino`**
- Sets up the BLE session as a peripheral, awaiting connection from the hub.
- Sends sensor data to the hub, including signal strength (RSSI) to detect proximity.

## 3rd Party App (Blynk)
The central hub connects to a **3rd party mobile app** (e.g., Blynk) that allows users to:
- **Track vehicle location** in real time.
- **Receive notifications** if the catalytic converter is tampered with or the sensor is out of range.
- **Monitor battery levels** for both the sensor and the central hub.
- **Customize alerts** (e.g., SMS or app notifications).

## Future Considerations
- **Battery Monitoring:** Implement a feature to monitor battery percentage for both the hub and the sensor, with alerts when battery levels fall below a threshold.
- **Deep Sleep Mode:** Integrate deep sleep functions to further extend battery life when the vehicle is not in use.
- **Machine Learning:** Add machine learning on the sensor to distinguish between regular vehicle vibrations and tampering (e.g., saw activity).
- **Additional Alerts:** Add support for **SMS alerts** to notify users in case of poor mobile app connectivity.

## Constraints and Challenges
- **BLE Range and Signal Strength:** Ensuring reliable communication between the hub and sensor, even in garages or buildings with poor signal penetration.
- **Battery Life:** Maximizing battery life for the sensor while maintaining reliable performance.
- **Heat Resistance:** Ensuring the sensor can withstand high temperatures due to the proximity to the exhaust system.

## How to Use the Code
1. **Central Hub (LilyGo T-SIM7000G):**
   - Clone this repository and open the `CentralHubSensor-Public.ino` file in the Arduino IDE.
   - Upload the code to the LilyGo board after setting up the necessary libraries for BLE, GPS, and 4G LTE.
   - Appropriate API keys/tokens will need to be added, as well as a UUID for the NRF82540 sensor.
   - **NOTICE**: For 4G LTE capabilities, you will need to have a SIM card that is compatible with the SIM7000G. I have used Hologram for many 4G/IOT Projects, and highly recommend them as a Pay-as-you-go model. This project assumes you have an understanding of IoT SIM cards, the setup and procedure. 

2. **Sensor (Seeed Xiao nRF52840):**
   - Open the `CatSensor-Public.ino` file in the Arduino IDE.
   - Upload the code to the Seeed Xiao board.
   - Include the UUID need for broadcasting, which matches what is in the central hub. 

3. **Mobile App:**
   - Download the **Blynk app** and set up an account.
   - Configure the app to receive GPS data and alerts from the central hub.
   - 
## References
- [Espressif - ESP32 Specifications](https://www.espressif.com/en/products/socs/esp32)
- [SimCom - SIM7000G LTE Module](https://www.simcomm2m.com/product/SIM7000X.html)
- [Nordic Semiconductor - nRF52840 Specifications](https://www.nordicsemi.com/products/nrf52840)
  
## License
This project is licensed under the **Apache 2.0 License**. See the LICENSE file for details.

**Want to Clone this Repository?**
   ```bash
   git clone https://github.com/MrHuangKong/Catalytic-Converter-Security-System.git

