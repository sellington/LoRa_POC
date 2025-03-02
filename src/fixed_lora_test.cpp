#include <Arduino.h>
#include <RadioLib.h>

// Original pin definitions from the Heltec WiFi LoRa 32 V3 pinout diagram
// #define LORA_CS         8    // LoRa_NSS (Chip Select) - Working
// #define LORA_SCK        9    // LoRa_SCK (Clock) - Working
// #define LORA_MOSI       10   // LoRa_MOSI - Working
// #define LORA_MISO       11   // LoRa_MISO - Working
// #define LORA_RST        12   // LoRa_RST (Reset) - Working
// #define LORA_BUSY       13   // LoRa_BUSY - BAD, STUCK LOW
// #define LORA_DIO1       14   // DIO1 - BAD, STUCK LOW

// Fixed pin definitions - using alternative working pins
#define LORA_CS         8    // LoRa_NSS (Chip Select)
#define LORA_SCK        9    // LoRa_SCK (Clock)
#define LORA_MOSI       10   // LoRa_MOSI
#define LORA_MISO       11   // LoRa_MISO
#define LORA_RST        12   // LoRa_RST (Reset)
#define LORA_BUSY       15   // Using alternative pin for BUSY (verified working)
#define LORA_DIO1       16   // Using alternative pin for DIO1 (verified working)
#define LED             35   // Built-in LED

// Create SX1262 instance with the new pin mapping
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);  // Time to open serial monitor
  
  // Initialize LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);  // LED on during setup
  
  Serial.println("\nHeltec WiFi LoRa 32 V3 - FIXED PIN CONFIGURATION");
  Serial.println("------------------------------------------------");
  Serial.println("Using alternative pins for BUSY (15) and DIO1 (16)");
  
  // Initialize SPI explicitly
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  
  // Print pin configuration
  Serial.println("SX1262 Pin Configuration:");
  Serial.print("SCK: "); Serial.println(LORA_SCK);
  Serial.print("MISO: "); Serial.println(LORA_MISO);
  Serial.print("MOSI: "); Serial.println(LORA_MOSI);
  Serial.print("CS: "); Serial.println(LORA_CS);
  Serial.print("RST: "); Serial.println(LORA_RST);
  Serial.print("BUSY: "); Serial.println(LORA_BUSY);  // Alternative pin!
  Serial.print("DIO1: "); Serial.println(LORA_DIO1);  // Alternative pin!
  
  // Using built-in SPI for ESP32-S3
  Serial.println("Using built-in ESP32 SPI with alternative pins");
  
  // Reset the radio module
  Serial.println("Resetting SX1262 module...");
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(100);
  
  // Try multiple frequencies to find one that works
  float frequencies[] = {915.0, 868.0, 433.0, 923.0};
  bool initSuccess = false;
  
  for (int i = 0; i < 4 && !initSuccess; i++) {
    Serial.print("Trying frequency: ");
    Serial.print(frequencies[i]);
    Serial.println(" MHz");
    
    // Initialize SX1262 with the current frequency
    int state = radio.begin(frequencies[i]);
    
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("SUCCESS - SX1262 initialized!");
      initSuccess = true;
    } else {
      Serial.print("FAILED - error code: ");
      Serial.println(state);
      delay(500);
    }
  }
  
  if (!initSuccess) {
    Serial.println("Failed to initialize SX1262 radio on all frequencies!");
    Serial.println("Check your connections and pin definitions");
    // Blink LED rapidly to show error
    while(1) {
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);
    }
  }
  
  // Set LoRa parameters if initialization was successful
  Serial.println("Setting LoRa parameters...");
  
  // Configure radio parameters - adjusted for reliability (fixed PACKET_TOO_LONG error)
  radio.setOutputPower(14);                 // Reduced to 14 dBm (from 17) for more stability
  radio.setSpreadingFactor(7);              // Reduced to SF7 (from SF10) for shorter air time
  radio.setBandwidth(250.0);                // Increased to 250 kHz (from 125) for more data capacity
  radio.setCodingRate(5);                   // 4/5 coding rate
  radio.setPreambleLength(8);               // 8 symbol preamble
  radio.setSyncWord(0x1424);                // Sync word for private network
  radio.setCRC(true);                       // Enable CRC
  
  // Set explicit header mode to handle variable packet lengths better
  radio.explicitHeader();
  
  // Set modem in standby mode before configuring
  radio.standby();
  
  // Print parameters
  Serial.println("LoRa parameters set.");
  Serial.println("SX1262 initialization complete!");
  digitalWrite(LED, LOW);
  
  // Send a short test packet (smaller payload due to PACKET_TOO_LONG error)
  Serial.println("Sending test packet...");
  digitalWrite(LED, HIGH);
  int state = radio.transmit("Hello LoRa!");
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SUCCESS: Packet sent successfully!");
  } else {
    Serial.print("Failed to send packet, error code: ");
    Serial.println(state);
  }
  digitalWrite(LED, LOW);
  
  // Set radio to receive mode
  Serial.println("Setting radio to continuous receive mode");
  radio.startReceive();
}

void loop() {
  // Check if received data is available
  if (radio.available()) {
    digitalWrite(LED, HIGH);
    
    // Create buffer for received data
    String receivedData;
    int state = radio.readData(receivedData);
    
    if (state == RADIOLIB_ERR_NONE) {
      // Print received data
      Serial.println("Received packet:");
      Serial.println(receivedData);
      
      // Print RSSI (signal strength)
      Serial.print("RSSI: ");
      Serial.print(radio.getRSSI());
      Serial.println(" dBm");
      
      // Print SNR (signal-to-noise ratio)
      Serial.print("SNR: ");
      Serial.print(radio.getSNR());
      Serial.println(" dB");
    } else {
      Serial.print("Failed to read packet, error code: ");
      Serial.println(state);
    }
    
    digitalWrite(LED, LOW);
    
    // Set radio back to receive mode
    radio.startReceive();
  }
  
  // Send a test packet every 10 seconds
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime > 10000) {
    lastSendTime = millis();
    
    Serial.println("Sending periodic test packet...");
    digitalWrite(LED, HIGH);
    
    // Shorter message to avoid the PACKET_TOO_LONG error
    String message = "LoRa Test " + String(millis() % 1000);
    int state = radio.transmit(message);
    
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("SUCCESS: Packet sent successfully!");
    } else {
      Serial.print("Failed to send packet, error code: ");
      Serial.println(state);
    }
    
    digitalWrite(LED, LOW);
    
    // Set radio back to receive mode
    radio.startReceive();
  }
}
