#include <SPI.h>      // Library for SPI communication (used by MCP2515 CAN module)
#include <mcp_can.h>  // Library for interfacing with the MCP2515 CAN controller
#include <SD.h>       // Library for SD card operations

// Define the SPI Chip Select pin for the CAN module
const int CAN_CS_PIN = 10;

// Create a CAN object using the defined CS pin
MCP_CAN CAN(CAN_CS_PIN);

// Define the SPI Chip Select pin for the SD card module
const int SD_CS_PIN = 4;

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(115200);
  
  // Initialize CAN bus with a baud rate of 500kbps and an 8MHz clock on the MCP2515
  if (CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Bus Initialized Successfully!");
  } else {
    Serial.println("CAN Bus Initialization Failed!");
    while (1);  // Stop execution if CAN initialization fails
  }

  // Set the CAN bus to Normal mode to allow transmission and reception of messages
  CAN.setMode(MCP_NORMAL);
  Serial.println("CAN Bus set to Normal Mode");

  // Initialize the SD card
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("SD Card Initialization Failed!");
    while (1);  // Stop execution if SD card initialization fails
  }
  Serial.println("SD Card Initialized Successfully!");
}

void loop() {
  long unsigned int rxId;  // Variable to store the CAN message ID
  unsigned char len = 0;   // Variable to store the length of the received message
  unsigned char rxBuf[8];  // Buffer to store the received CAN message data
  String dataString;       // String to format data for logging

  // Check if a new message has been received on the CAN bus
  if (CAN.checkReceive() == CAN_MSGAVAIL) {
    // Read the received message into rxId, len, and rxBuf
    CAN.readMsgBuf(&rxId, &len, rxBuf);

    // Format the received message as a string
    dataString = "ID: 0x" + String(rxId, HEX) + " Data: ";
    for (int i = 0; i < len; i++) {
      dataString += String(rxBuf[i], HEX) + " ";  // Append each byte of data
    }

    // Print the received data to the Serial Monitor
    Serial.println(dataString);

    // Open the SD card file for appending data
    File dataFile = SD.open("can_log.txt", FILE_WRITE);
    if (dataFile) {
      // Write the received CAN data to the SD card
      dataFile.println(dataString);
      dataFile.close();  // Close the file after writing
      Serial.println("Data logged to SD card.");
    } else {
      // If the file could not be opened, display an error
      Serial.println("Error writing to SD card.");
    }
  }

  // Small delay to prevent overwhelming the Serial Monitor
  delay(100);
}
--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
#include <SPI.h>             // SPI library for CAN Bus and SD card communication
#include <SD.h>              // SD card library for logging data
#include <SoftwareSerial.h>  // Library to use software-based serial communication
#include <TinyGPS.h>         // TinyGPS library for handling GPS data
#include <mcp_can.h>         // MCP_CAN library for handling CAN Bus communication

// Pin definitions for GPS, SD, and CAN Bus
#define RXPIN 4              // GPS Receiver pin (RX)
#define TXPIN 5              // GPS Transmitter pin (TX)
#define GPSBAUD 4800         // Baud rate for GPS module
#define chipSelect 9         // Pin for the SD card chip select
#define CAN_CS_PIN 10        // Chip select pin for the MCP2515 CAN module

// Create instances for GPS and CAN Bus
SoftwareSerial uart_gps(RXPIN, TXPIN);  // Initialize SoftwareSerial for GPS
TinyGPS gps;                           // TinyGPS instance for parsing GPS data
MCP_CAN CAN(CAN_CS_PIN);               // MCP_CAN instance for handling CAN Bus communication

// Variables for CAN Bus and user input
char buffer[456];  // Buffer for CAN Bus data
char UserInput;    // Variable to store user input from Serial Monitor

//********************************* Setup Function *********************************//
void setup() {
  Serial.begin(9600);           // Initialize serial communication for debugging
  uart_gps.begin(GPSBAUD);      // Initialize GPS communication at the specified baud rate

  // Initialize the SD card
  /*Serial.print("Initializing SD card...");
  pinMode(chipSelect, OUTPUT);  // Set the chip select pin for the SD card
  if (!SD.begin(chipSelect)) {  // Check if the SD card is initialized successfully
    Serial.println("Card failed, or not present");
     while (1);  // Stop execution if SD card initialization fails
  }
  Serial.println("card initialized.");*/

  // Initialize CAN Bus
  if (CAN.begin(MCP_STDEXT, CAN_500KBPS, MCP_8MHZ) == CAN_OK) {
    Serial.println("CAN Bus initialized successfully.");
  } else {
    Serial.println("CAN Bus initialization failed.");
    while (1);  // Stop execution if CAN Bus initialization fails
  }

  // Display menu exampel for the user to select which data to log. additional parameters could be added based on our specific needs.
  Serial.println("Please choose a menu option:");
  Serial.println("1. Speed");
  Serial.println("2. RPM");
  Serial.println("3. Throttle");
  Serial.println("4. Coolant Temperature");
  Serial.println("5. O2 Voltage");
  Serial.println("6. MAF Sensor");
}

//******************************** Main Loop *********************************//
void loop() {
  // Check if the user has entered input via the Serial Monitor
  if (Serial.available()) {
    UserInput = Serial.read();  // Read the user input

    // Handle each menu option based on the user input
    switch (UserInput) {
      case '1':
        requestCANData(0x123, "Speed");  // Replace 0x123 with actual ID for vehicle speed
        break;
      case '2':
        requestCANData(0x456, "RPM");   // Replace 0x456 with actual ID for engine RPM
        break;
      case '3':
        requestCANData(0x789, "Throttle");  // Replace 0x789 with actual ID for throttle position
        break;
      case '4':
        requestCANData(0xABC, "Coolant Temp");  // Replace 0xABC with actual ID
        break;
      case '5':
        requestCANData(0xDEF, "O2 Voltage");    // Replace 0xDEF with actual ID
        break;
      case '6':
        requestCANData(0xFED, "MAF Sensor");    // Replace 0xFED with actual ID
        break;
      default:
        Serial.println("Invalid option. Please try again.");  // Handle invalid input
    }
  }

  // Process GPS data
  while (uart_gps.available()) {
    int c = uart_gps.read();  // Read incoming data from the GPS module
    if (gps.encode(c)) {      // Check if valid GPS data is available
      getGPSData();           // Parse and log the GPS data
    }
  }

  // Log analog sensor data from pins A0, A1, and A2 to the SD card
  //logAnalogDataToSD();

  // Check for incoming CAN messages and process them
  readCANMessages();
}

//******************************** GPS Function *********************************//
// Function to get GPS data and log it
void getGPSData() {
  float latitude, longitude;  // Variables to store latitude and longitude
  gps.f_get_position(&latitude, &longitude);  // Get GPS coordinates

  int year;
  byte month, day, hour, minute, second, hundredths;
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);  // Get date and time

  // Print the GPS data to the Serial Monitor
  Serial.print("Lat/Long: ");
  Serial.print(latitude, 5);
  Serial.print(", ");
  Serial.println(longitude, 5);

  // Format GPS data as a string for logging
  String gpsData = String("Lat: ") + latitude + ", Long: " + longitude;
  gpsData += String(", Date: ") + month + "/" + day + "/" + year;
  gpsData += String(" Time: ") + hour + ":" + minute + ":" + second;

  // Log GPS data to the SD card
  //logToSD("GPS Data", gpsData.c_str());
}

//******************************** CAN Functions *********************************//
// Function to request data from the CAN Bus
void requestCANData(long id, const char* label) {
  byte len = 8;  // Length of CAN message
  byte data[8];  // Data buffer for CAN message

  if (CAN.sendMsgBuf(id, 0, len, data) == CAN_OK) {  // Send CAN request
    Serial.print(label);
    Serial.println(" request sent.");
  } else {
    Serial.println("Error sending CAN request.");
  }
}

// Function to read incoming CAN messages
void readCANMessages() {
  long unsigned int rxId;  // Variable to store the ID of the received message
  byte len = 0;            // Length of the received message
  byte rxBuf[8];           // Buffer to store received data

  if (CAN.checkReceive() == CAN_MSGAVAIL) {  // Check if a CAN message is available
    CAN.readMsgBuf(&rxId, &len, rxBuf);      // Read the CAN message

    // Print the received CAN message
    Serial.print("Received CAN message with ID: 0x");
    Serial.println(rxId, HEX);
    Serial.print("Data: ");
    for (byte i = 0; i < len; i++) {
      Serial.print(rxBuf[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
  }
}

//******************************** Log Functions *********************************//
// Function to log data to the SD card
/*void logToSD(const char* label, const char* data) {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);  // Open the file for writing
  if (dataFile) {
    dataFile.print(millis());  // Log the current timestamp in milliseconds
    dataFile.print(" ms, ");
    dataFile.print(label);     // Log the label (e.g., Speed, RPM, etc.)
    dataFile.print(": ");
    dataFile.println(data);    // Log the actual data
    dataFile.close();          // Close the file
  } else {
    Serial.println("Error opening datalog.txt");  // Handle file open error
  }
}*/

// Function to log analog sensor data from pins A0, A1, and A2
/*void logAnalogDataToSD() {
  File dataFile = SD.open("datalog.txt", FILE_WRITE);  // Open the file for writing
  if (dataFile) {
    dataFile.print(millis());  // Log the current timestamp in milliseconds
    dataFile.print(" ms, ");

    // Loop through analog pins A0, A1, and A2
    for (int analogPin = 0; analogPin < 3; analogPin++) {
      int sensorVal = analogRead(analogPin);  // Read the sensor value
      dataFile.print("A");
      dataFile.print(analogPin);
      dataFile.print("=");
      dataFile.print(sensorVal);
      if (analogPin < 2) dataFile.print(", ");  // Separate values with a comma
    }
    dataFile.println();  // Add a newline for the next entry
    dataFile.close();    // Close the file
  } else {
    Serial.println("Error opening datalog.txt");  // Handle file open error
  }
}*/


