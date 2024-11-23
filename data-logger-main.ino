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

