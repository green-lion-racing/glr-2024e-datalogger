#include <SPI.h>             // SPI library for CAN Bus and SD card communication 
#include <SD.h>              // SD card library for logging data
#include <SoftwareSerial.h>  // Library to use software-based serial communication
#include <TinyGPS.h>         // TinyGPS library for handling GPS data
#include <Canbus.h>          // Canbus library for handling CAN Bus communication
#include <mcp2515.h>
#include <mcp2515_defs.h>

// Pin definitions for GPS, SD, and CAN Bus
#define RXPIN 4              // GPS Receiver pin (RX)
#define TXPIN 5              // GPS Transmitter pin (TX)
#define GPSBAUD 4800         // Baud rate for GPS module
#define chipSelect 9         // Pin for the SD card chip select

// Create instances for GPS and CAN Bus
SoftwareSerial uart_gps(RXPIN, TXPIN);  // Initialize SoftwareSerial for GPS
TinyGPS gps;                           // TinyGPS instance for parsing GPS data

// Variables for CAN Bus and user input
char buffer[456];  // Buffer for CAN Bus data
char UserInput;    // Variable to store user input from Serial Monitor
bool sdInitialized = false;   // Variable to track SD card initialization status
bool canInitialized = false;  // Variable to track CAN Bus initialization status

//********************************* Setup Function *********************************//
void setup() {
    Serial.begin(9600);           // Initialize serial communication for debugging
    uart_gps.begin(GPSBAUD);      // Initialize GPS communication at the specified baud rate

    // Initialize the SD card
    Serial.print("Initializing SD card...");
    pinMode(chipSelect, OUTPUT);  // Set the chip select pin for the SD card
    if (SD.begin(chipSelect)) {  // Check if the SD card is initialized successfully
        Serial.println("Card initialized.");
        sdInitialized = true;  // Mark SD initialization as successful
    } else {
        Serial.println("Card failed, or not present");
        sdInitialized = false;  // Mark SD initialization as failed
    }

    // Initialize CAN Bus
    Serial.print("Initializing CAN Bus...");
    if (Canbus.init(CANSPEED_500)) {  // Initialize CAN Bus at 500 kbps
        Serial.println("CAN Bus initialized successfully.");
        canInitialized = true;  // Mark CAN Bus initialization as successful
    } else {
        Serial.println("CAN Bus initialization failed.");
        canInitialized = false;  // Mark CAN Bus initialization as failed
    }
     /* //Display menu exampel for the user to select which data to log. additional     parameters could be added based on our specific needs.
    Serial.println("Please choose a menu option:");
    Serial.println("1. placeholder");*/
}

//******************************** Main Loop *********************************//
void loop() {
  // Check if the user has entered input via the Serial Monitor
  /*if (Serial.available()) {
    UserInput = Serial.read();  // Read the user input

    // Handle each menu option based on the user input
    switch (UserInput) {
      case '1':
        requestCANData(0x123, "placeholder");  // Replace 0x123 with actual ID for vehicle speed
        break;
      default:
        Serial.println("Invalid option. Please try again.");  // Handle invalid input
    }
  }*/
    // Process GPS data
    while (uart_gps.available()) {
        int c = uart_gps.read();  // Read incoming data from the GPS module
        if (gps.encode(c)) {      // Check if valid GPS data is available
            getGPSData();           // Parse and log the GPS data
        }
    }

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
    logToSD("GPS Data", gpsData.c_str());
}

//******************************** CAN Functions *********************************//
// Function to request data from the CAN Bus
void requestCANData(long id, const char* label) {
    // Check if CAN bus is initialized
    if (!canInitialized) {
        Serial.println("CAN Bus not initialized. Skipping requesting.");
        return;  // Exit the function if CAN bus is not initialized
    }

    tCAN message;                      // Create a tCAN message structure
    message.id = id;                   // Set the CAN message ID
    message.header.rtr = 0;            // Remote Transmission Request disabled
    message.header.length = 8;         // Length of the message (8 bytes)
    memset(message.data, 0, 8);        // Clear the message data

    // Send CAN request
    if (mcp2515_send_message(&message)) {  // Correct constant is CAN_OK
        Serial.print(label);
        Serial.println(" request sent.");
        // Log the successful CAN request to the SD card
        String logEntry = String(label) + " request sent with ID: " + String(id, HEX);
        logToSD("CAN Request", logEntry.c_str());
        } else {
        Serial.println("Error sending CAN request.");
        }
}

// Function to read incoming CAN messages
void readCANMessages() {
  // Check if CAN bus is initialized
  if (!canInitialized) {
       Serial.println("CAN Bus not initialized. Skipping message reading.");
      return;  // Exit the function if CAN bus is not initialized
  }

  tCAN message;                      // Create a tCAN message structure

  if (mcp2515_check_message()) {     // Check if a CAN message is available
       if (mcp2515_get_message(&message) ) {  // Correct constant is CAN_OK
          // Print the received CAN message
          Serial.print("ID: ");
          Serial.println(message.id, HEX);
          Serial.print("Data: ");
          Serial.print(message.header.length,DEC);
          for (int i = 0; i < message.header.length; i++) {
              Serial.print(message.data[i], HEX);
              Serial.print(" ");
          }
          Serial.println();
          // Format CAN message data as a string for logging
          String canData = String("ID: ") + String(message.id, HEX) + " Data: ";
          for (int i = 0; i < message.header.length; i++) {
              canData += String(message.data[i], HEX) + " ";
          }

          // Log CAN message data to the SD card
          logToSD("CAN Data", canData.c_str());
          } else {
          Serial.println("Error reading CAN message.");
          }  
  }  
    
}

//******************************** Log Functions *********************************//
// Function to log data to the SD card
void logToSD(const char* label, const char* data) {
    // Check if SD card is initialized
    if (!sdInitialized) {
        Serial.println("SD card not initialized. Skipping log.");
        return;  // Exit the function if SD card is not initialized
    }

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
}