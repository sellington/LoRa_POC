#include <Arduino.h>
#include <RadioLib.h>

// Confirmed working pins from discovery testing
#define LORA_CS         8    // LoRa_NSS (Chip Select)
#define LORA_SCK        9    // LoRa_SCK (Clock)
#define LORA_MOSI       10   // LoRa_MOSI
#define LORA_MISO       11   // LoRa_MISO
#define LORA_RST        12   // LoRa_RST (Reset)
#define LORA_BUSY       13   // Original BUSY pin - confirmed working
#define LORA_DIO1       14   // Original DIO1 pin - confirmed working
#define LED             35   // Built-in LED

// Function declarations
void sendTestPacket();

// Create SX1262 instance with the confirmed working pin mapping
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

// Message counter
int messageCount = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);  // Time to open serial monitor
  
  // Initialize LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);  // LED on during setup
  
  Serial.println("\nHeltec WiFi LoRa 32 V3 - CONFIRMED WORKING CONFIGURATION");
  Serial.println("----------------------------------------------------");
  Serial.println("Using schematic pins confirmed by pin discovery test");
  
  // Initialize SPI explicitly
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  
  // Print pin configuration
  Serial.println("SX1262 Pin Configuration:");
  Serial.print("CS: "); Serial.println(LORA_CS);
  Serial.print("SCK: "); Serial.println(LORA_SCK);
  Serial.print("MOSI: "); Serial.println(LORA_MOSI);
  Serial.print("MISO: "); Serial.println(LORA_MISO);
  Serial.print("RST: "); Serial.println(LORA_RST);
  Serial.print("BUSY: "); Serial.println(LORA_BUSY);
  Serial.print("DIO1: "); Serial.println(LORA_DIO1);
  
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
    // Blink LED rapidly to show error
    while (true) {
      digitalWrite(LED, !digitalRead(LED));
      delay(100);
    }
  }
  
  // Configure radio parameters
  Serial.println("Setting radio parameters...");
  
  // Enter standby mode first
  radio.standby();
  
  // Use the minimal configuration that worked in the discovery test
  radio.setOutputPower(2);                 // Low power for testing
  radio.setSpreadingFactor(6);             // SF6 (fastest data rate)
  radio.setBandwidth(500.0);               // 500 kHz bandwidth
  radio.setCodingRate(5);                  // 4/5 coding rate
  radio.setPreambleLength(8);              // 8 symbol preamble
  radio.setSyncWord(0x34);                 // Sync word for private network (corrected from 0x1424)
  radio.setCRC(true);                      // Enable CRC
  
  // Parameters set
  Serial.println("Radio parameters configured successfully.");
  digitalWrite(LED, LOW);
  
  // Send initial test packet
  sendTestPacket();
  
  // Start listening for incoming packets
  radio.startReceive();
}

void sendTestPacket() {
  // Create a simple message with incrementing counter
  String message = "Packet #" + String(messageCount++);
  
  // Send the packet
  Serial.print("Sending packet: ");
  Serial.println(message);
  digitalWrite(LED, HIGH);
  
  int state = radio.transmit(message);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SUCCESS: Packet sent!");
  } else {
    Serial.print("Failed to send packet, error code: ");
    Serial.println(state);
    
    // Detailed error information
    switch(state) {
      case RADIOLIB_ERR_PACKET_TOO_LONG:
        Serial.println("Error: Packet too long!");
        break;
      case RADIOLIB_ERR_TX_TIMEOUT:
        Serial.println("Error: Transmission timed out!");
        break;
      default:
        Serial.println("Error: Unknown error!");
    }
  }
  
  digitalWrite(LED, LOW);
  delay(100);
  
  // Return to receive mode
  radio.startReceive();
}

void loop() {
  // Check for incoming packets
  if (radio.available()) {
    // Buffer for the received data
    String data;
    
    // Read the received data
    int state = radio.readData(data);
    
    if (state == RADIOLIB_ERR_NONE) {
      // Print the received data
      Serial.println("Received packet:");
      Serial.println(data);
      
      // Print RSSI (signal strength) and SNR (signal to noise ratio)
      Serial.print("RSSI: ");
      Serial.print(radio.getRSSI());
      Serial.println(" dBm");
      
      Serial.print("SNR: ");
      Serial.print(radio.getSNR());
      Serial.println(" dB");
      
      // Blink LED to indicate reception
      digitalWrite(LED, HIGH);
      delay(200);
      digitalWrite(LED, LOW);
    } else {
      Serial.print("Reception failed, error code: ");
      Serial.println(state);
    }
  }
  
  // Send a test packet periodically (every 10 seconds)
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime > 10000) {
    lastSendTime = millis();
    sendTestPacket();
  }
}
