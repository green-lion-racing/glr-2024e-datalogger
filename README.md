# GLR2024e-datalogger

This project demonstrates the GLR2024e data logging system using an Arduino, CAN Bus communication, GPS module and an SD card for data storage. 
The system retrieves vehicle CAN Bus data and GPS location data , then logs them to an SD card for analysis.


![overview](overview.jpg "overview")



## Features

- **CAN Bus Communication:** Reads and processes CAN messages from a vehicle.
- **GPS Integration:** Logs real-time latitude, longitude, date, and time.
- **SD Card Logging:** Stores GPS and CAN Bus data for offline analysis.

## Hardware Requirements

**main components** 
   1. **Arduino Uno Board**
   2. **GPS Module** (jst sh connector 6pin)
   3. **CAN Bus Shield** (with MCP2515 CAN controller )

**Additional items**
   1. **Connector Shield to GPS** (https://eu.robotshop.com/de/products/pololu-6-poliges-weibliches-jst-sh-kabel-12-cm)
   2. **Connector IPEX to SMA** (https://eu.robotshop.com/products/dfrobot-ipex-to-sma-female-connector-cable-2x)
   3. **External GPS Antenna SMA** (https://co-en.rs-online.com/product/siretta/alpha4a-1m-smam-s-s-26/73525599/)
   4. **FAT32 formartierte SD Karte**
   5. **GPS Battery** (RTC Batt: CR1220)

---

## Software Requirements

1. **Arduino IDE**
2. **Libraries**:
   - `SPI` (Included in Arduino Standard Libraries)
   - `SD` (Included in Arduino Standard Libraries)
   - `SoftwareSerial` (Included in Arduino Standard Libraries)
   - `TinyGPS` (exist in the Arduino library manager)
   - `Canbus.h` (https://github.com/sparkfun/SparkFun_CAN-Bus_Arduino_Library/tree/master/src)
   - `mcp2515.h` (https://github.com/sparkfun/SparkFun_CAN-Bus_Arduino_Library/tree/master/src)
   - `mcp2515_defs.h` (https://github.com/sparkfun/SparkFun_CAN-Bus_Arduino_Library/tree/master/src)

## How It Works

1. **Setup:**
   - Initializes GPS, CAN Bus, and SD card modules.
   - Displays initialization status on the Serial Monitor.

2. **Loop:**
   - Continuously reads GPS data and logs it to the SD card.
   - Checks for incoming CAN messages and logs them to the SD card.

3. **Logging:**
   - Data is logged to `datalog.txt` on the SD card in the format:
     ```
     <timestamp> ms, <Label>: <Data>

