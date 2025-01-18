# glr-2024e-datalogger

This project demonstrates a vehicle data logging system using an Arduino, CAN Bus communication, GPS module and an SD card for data storage. 
The system retrieves vehicle CAN Bus data and GPS location data , then logs them to an SD card for analysis.

## Features

- **CAN Bus Communication:** Reads and processes CAN messages from a vehicle.
- **GPS Integration:** Logs real-time latitude, longitude, date, and time.
- **SD Card Logging:** Stores GPS and CAN Bus data for offline analysis.
- **Error Handling:** Detects and handles initialization issues for the SD card and CAN Bus.

## Hardware Requirements

1. **Arduino Uno Board**
2. **GPS Module** (jst sh connector 6pin)
3. **CAN Bus Shield** (with MCP2515 CAN controller )
4. **SD Card Module** (with compatible microSD card)
5. **Jumper Wires** and other necessary peripherals

---

## Software Requirements

1. **Arduino IDE**
2. **Libraries**:
   - `SPI.h` 
   - `SD.h` 
   - `SoftwareSerial.h`
   - `TinyGPS`
   - `Canbus.h`
   - `mcp2515.h`
   - `mcp2515_defs.h`

*## How It Works

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

