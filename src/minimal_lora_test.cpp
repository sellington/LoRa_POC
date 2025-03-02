#include <Arduino.h>
#include <RadioLib.h>

// Pin definitions from the Heltec WiFi LoRa 32 V3 schematic
#define LORA_CS         8    // NSS/CS
#define LORA_SCK        9    // SCK
#define LORA_MOSI       10   // MOSI
#define LORA_MISO       11   // MISO
#define LORA_RST        12   // RESET
#define LORA_BUSY       15   // BUSY (using alternative pin verified working)
#define LORA_DIO1       16   // DIO1 (using alternative pin verified working)
#define LED             35   // Built-in LED

// Create SX1262 instance with the fixed pin mapping
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);  // Time to open serial monitor
  
  // Initialize LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);  // LED on during setup
  
  Serial.println("\nHeltec WiFi LoRa 32 V3 - MINIMAL TEST");
  Serial.println("--------------------------------------");
  
  // Initialize SPI explicitly
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  
  // Print pin configuration
  Serial.println("SX1262 Pin Configuration:");
  Serial.print("CS: "); Serial.println(LORA_CS);
  Serial.print("SCK: "); Serial.println(LORA_SCK);
  Serial.print("MOSI: "); Serial.println(LORA_MOSI);
  Serial.print("MISO: "); Serial.println(LORA_MISO);
  Serial.print("RST: "); Serial.println(LORA_RST);
  Serial.print("BUSY: "); Serial.println(LORA_BUSY);  // Alternative pin
  Serial.print("DIO1: "); Serial.println(LORA_DIO1);  // Alternative pin
  
  // Reset the radio module
  Serial.println("Resetting SX1262 module...");
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(100);
  
  // Initialize with 915 MHz frequency
  Serial.print("Initializing SX1262 with 915 MHz... ");
  int state = radio.begin(915.0);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SUCCESS!");
  } else {
    Serial.print("FAILED, error code: ");
    Serial.println(state);
    while (true) {
      digitalWrite(LED, !digitalRead(LED));
      delay(100);
    }
  }
  
  // Set the absolute minimum configuration for reliable transmission
  Serial.println("Setting minimal parameters...");
  
  // Enter standby mode before configuration
  radio.standby();
  
  // Set very minimal parameters
  radio.setOutputPower(10);                  // Lower power
  radio.setSpreadingFactor(7);               // Lowest SF for fastest transmission
  radio.setBandwidth(500.0);                 // Widest bandwidth (500 kHz)
  radio.setCodingRate(5);                    // 4/5 coding rate
  radio.setPreambleLength(8);                // Minimum preamble length
  radio.setCRC(true);                        // Keep CRC enabled for reliability
  
  // Enable explicit header mode for variable packet lengths
  radio.explicitHeader();
  
  // Set the maximum payload length to a small value
  // This is critical - the SX1262 has a limited buffer size
  Serial.println("Setting maximum packet length to 32 bytes");
  radio.implicitHeader(32);  // Set maximum packet size to 32 bytes
  
  Serial.println("Parameters set. Ready to transmit minimal packets.");
  digitalWrite(LED, LOW);
  
  // Send a minimal test packet (just a few bytes)
  Serial.println("\nSending minimal test packet (1 byte)...");
  digitalWrite(LED, HIGH);
  
  // Just send a single byte
  uint8_t testByte = 0x42;  // Just the byte 'B' (ASCII 66)
  state = radio.transmit(&testByte, 1);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SUCCESS: Minimal packet sent!");
  } else {
    Serial.print("FAILED, error code: ");
    Serial.println(state);
    Serial.println("Detailed error explanation:");
    
    switch(state) {
      case RADIOLIB_ERR_PACKET_TOO_LONG:
        Serial.println("Packet too long for the current settings!");
        break;
      case RADIOLIB_ERR_TX_TIMEOUT:
        Serial.println("Transmission timed out!");
        break;
      case RADIOLIB_ERR_SPI_CMD_TIMEOUT:
        Serial.println("SPI command timed out!");
        break;
      case RADIOLIB_ERR_CHIP_NOT_FOUND:
        Serial.println("Chip not found!");
        break;
      default:
        Serial.println("Unknown error!");
    }
  }
  
  digitalWrite(LED, LOW);
  
  // Try transmitting another minimal packet with different parameters
  Serial.println("\nAttempting transmission with even simpler configuration...");
  
  // Reset the radio again
  radio.reset();
  delay(100);
  
  // Re-initialize with minimal configuration
  radio.standby();
  radio.setSpreadingFactor(6);          // Minimum SF
  radio.setBandwidth(500.0);            // Maximum bandwidth
  radio.setCodingRate(5);               // Lowest coding rate
  radio.setPreambleLength(4);           // Minimum preamble
  radio.setOutputPower(2);              // Very low power
  
  // Try to send a single byte again
  Serial.println("Sending single byte with minimal configuration...");
  digitalWrite(LED, HIGH);
  
  // Send a different byte
  uint8_t secondTestByte = 0x41;  // ASCII 'A'
  state = radio.transmit(&secondTestByte, 1);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SUCCESS: Second minimal packet sent!");
  } else {
    Serial.print("FAILED, error code: ");
    Serial.println(state);
  }
  
  delay(100);  // Wait for transmission to complete
  
  digitalWrite(LED, LOW);
  
  Serial.println("Test complete. Check for any error codes above.");
}

void loop() {
  // Just blink LED to show the program is running
  digitalWrite(LED, HIGH);
  delay(500);
  digitalWrite(LED, LOW);
  delay(500);
}
