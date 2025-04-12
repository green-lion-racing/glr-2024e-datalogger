#include <SPI.h>             // SPI library for CAN Bus and SD card communication 
#include <SD.h>              // SD card library for logging data
#include <SoftwareSerial.h>  // Library to use software-based serial communication
#include <TinyGPS.h>         // TinyGPS library for handling GPS data
#include <Canbus.h>          // Canbus library for handling CAN Bus communication
#include <mcp2515.h>
#include <mcp2515_defs.h>
#include <MemoryFree.h>

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
int session = 0;
int year = 0;
byte month, day, hour, minute, second, hundredths = 0;
float latitude, longitude;
int memory = 0;
char filename[16];


//********************************* Setup Function *********************************//
void setup() {
  Serial.begin(9600);           // Initialize serial communication for debugging
  uart_gps.begin(GPSBAUD);      // Initialize GPS communication at the specified baud rate

  // Initialize the SD card
  Serial.println("\nInitializing SD card...");
  pinMode(chipSelect, OUTPUT);  // Set the chip select pin for the SD card
  if (SD.begin(chipSelect)) {  // Check if the SD card is initialized successfully
    Serial.println("Card initialized.");
    sdInitialized = true;  // Mark SD initialization as successful
  } else {
    Serial.println("Card failed, or not present");
    sdInitialized = false;  // Mark SD initialization as failed
  }

  // Initialize CAN Bus
  Serial.println("Initializing CAN Bus...");
  if (Canbus.init(CANSPEED_500)) {  // Initialize CAN Bus at 500 kbps
    Serial.println("CAN Bus initialized successfully.");
    canInitialized = true;  // Mark CAN Bus initialization as successful
  } else {
    Serial.println("CAN Bus initialization failed.");
    canInitialized = false;  // Mark CAN Bus initialization as failed
  }
    /* //Display menu example for the user to select which data to log. additional     parameters could be added based on our specific needs.
  Serial.println("Please choose a menu option:");
  Serial.println("1. placeholder");*/

  if (sdInitialized) {
    int n = 0;
    snprintf(filename, sizeof(filename), "log-%03d.csv", n);
    while(SD.exists(filename)) {
      n++;
      snprintf(filename, sizeof(filename), "log-%03d.csv", n);
    }
    Serial.print("Starting in session ");
    Serial.print(n);
    Serial.println(".");
  }
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

  if (memory != freeMemory()) {
    Serial.print("Free memory ");
    Serial.print(freeMemory());
    Serial.println(".");
    memory = freeMemory();
    
  }
    
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
  // Variables to store latitude and longitude
  gps.f_get_position(&latitude, &longitude);  // Get GPS coordinates
  gps.crack_datetime(&year, &month, &day, &hour, &minute, &second, &hundredths);  // Get date and time
  
  if (!sdInitialized) {
    Serial.println("SD card not initialized. Skipping log.");
    return;  // Exit the function if SD card is not initialized
  }
  
  File dataFile = SD.open(filename, FILE_WRITE);  // Open the file for writing
  if (dataFile) {
    dataFile.print(millis());
    dataFile.print(",");
    char buffer4[4];
    sprintf (buffer4, "%04d", year);
    dataFile.print(buffer4);
    dataFile.print("-");
    char buffer2[2];
    sprintf (buffer2, "%02d", month);
    dataFile.print(buffer2);
    dataFile.print("-");
    sprintf (buffer2, "%02d", day);
    dataFile.print(buffer2);
    dataFile.print(" ");
    sprintf (buffer2, "%02d", hour);
    dataFile.print(buffer2);
    dataFile.print(":");
    sprintf (buffer2, "%02d", minute);
    dataFile.print(buffer2);
    dataFile.print(":");
    sprintf (buffer2, "%02d", second);
    dataFile.print(buffer2);
    dataFile.print(",");
    char buffer3[3];
    sprintf (buffer3, "%03d", hundredths);
    dataFile.print(buffer3);
    dataFile.print(",,,");
    dataFile.print(longitude);
    dataFile.print(",");
    dataFile.print(latitude);
    dataFile.println("");
    dataFile.close();
  } else {
    Serial.println("Error opening file.");  // Handle file open error
  }
}

//******************************** CAN Functions *********************************//
// Function to request data from the CAN Bus
/*void requestCANData(long id, const char* label) {
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
    } else {
      Serial.println("Error sending CAN request.");
  }
}*/

// Function to read incoming CAN messages
void readCANMessages() {
  // Check if CAN bus is initialized
  if (!canInitialized) {
    Serial.println("CAN Bus not initialized. Skipping message reading.");
    return;  // Exit the function if CAN bus is not initialized
  }

  tCAN message;                      // Create a tCAN message structure

  if (mcp2515_check_message()) {     // Check if a CAN message is available
    //Serial.print(".");
    if (mcp2515_get_message(&message) ) {  // Correct constant is CAN_OK
      if (!sdInitialized) {
        Serial.println("SD card not initialized. Skipping log.");
        return;  // Exit the function if SD card is not initialized
      }

      File dataFile = SD.open(filename, FILE_WRITE);  // Open the file for writing
      if (dataFile) {
        dataFile.print(millis());
        dataFile.print(",");
        char buffer4[4];
        sprintf (buffer4, "%04d", year);
        dataFile.print(buffer4);
        dataFile.print("-");
        char buffer2[2];
        sprintf (buffer2, "%02d", month);
        dataFile.print(buffer2);
        dataFile.print("-");
        sprintf (buffer2, "%02d", day);
        dataFile.print(buffer2);
        dataFile.print(" ");
        sprintf (buffer2, "%02d", hour);
        dataFile.print(buffer2);
        dataFile.print(":");
        sprintf (buffer2, "%02d", minute);
        dataFile.print(buffer2);
        dataFile.print(":");
        sprintf (buffer2, "%02d", second);
        dataFile.print(buffer2);
        dataFile.print(",");
        char buffer3[3];
        sprintf (buffer3, "%03d", hundredths);
        dataFile.print(buffer3);
        dataFile.print(",");
        dataFile.print(message.id, HEX);
        dataFile.print(",");
        for (int i = 0; i < message.header.length; i++) {
          sprintf (buffer3, "%02x", message.data[i]);
          dataFile.print(buffer3);
        }
        dataFile.println(",,");
        dataFile.close();
      } else {
        Serial.println("Error opening file.");  // Handle file open error
      }
    } else {
      Serial.println("Error reading CAN message.");
    }  
  }     
}
