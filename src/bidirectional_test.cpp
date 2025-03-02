/*
 * Bidirectional LoRa Communication Test
 * 
 * This is a simplified test program to verify bidirectional communication
 * between two Heltec WiFi LoRa 32 V3 boards. It implements basic ping/pong
 * functionality without the complexity of the full application.
 * 
 * Instructions:
 * 1. Upload this code to both devices
 * 2. Set MODE to DEVICE_A on one device and DEVICE_B on the other
 * 3. Open serial monitors for both devices
 * 4. Observe the ping/pong communication pattern
 * 
 * This test uses the confirmed working pin configuration and LoRa parameters.
 */

#include <Arduino.h>
#include <RadioLib.h>

// Device mode - change for each device
#define DEVICE_A 1  // Sends pings, expects pongs (like remote device)
#define DEVICE_B 2  // Listens for pings, sends pongs (like base station)
#define MODE DEVICE_A  // <<< CHANGE THIS FOR EACH DEVICE >>>

// LED pin for status indication
#define LED_PIN 35

// Correct pin definitions for Heltec WiFi LoRa 32 V3
#define LORA_CS     8   // LoRa_NSS (Chip Select)
#define LORA_SCK    9   // LoRa_SCK (Clock)
#define LORA_MOSI   10  // LoRa_MOSI
#define LORA_MISO   11  // LoRa_MISO
#define LORA_RST    12  // LoRa_RST (Reset)
#define LORA_BUSY   13  // BUSY pin - confirmed working
#define LORA_DIO1   14  // DIO1 pin - confirmed working

// LoRa parameters - using the minimal working configuration
#define LORA_FREQUENCY       915.0   // MHz (US frequency)
#define LORA_BANDWIDTH       500.0   // kHz - maximum bandwidth for reliable comms
#define LORA_SPREADING_FACTOR 6      // Minimum spreading factor (fastest)
#define LORA_CODING_RATE     5       // 4/5 coding rate
#define LORA_SYNC_WORD       0x34    // Sync word that worked in our tests
#define LORA_POWER           10      // dBm - increased for more reliable testing
#define LORA_PREAMBLE_LENGTH 8       // symbols

// Message types
#define MSG_PING "PING"
#define MSG_PONG "PONG"

// Communication parameters
#define MAX_RETRIES 5       // Maximum transmission attempts
#define ACK_TIMEOUT 2000    // Timeout for acknowledgment in ms (increased)
#define PING_INTERVAL 5000  // Interval between pings in ms

// Create SX1262 instance with the correct pin configuration
SX1262 radio = new Module(LORA_CS, LORA_DIO1, LORA_RST, LORA_BUSY);

// Function prototypes
void setupRadio();
bool sendMessage(const char* message);
bool receiveMessage(String& message);
void blinkLED(int times, int duration);

// Timing variables
unsigned long lastPingTime = 0;
unsigned long messageCount = 0;
unsigned long successCount = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(2000);  // Give time for serial monitor to open
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // Turn on during initialization
  
  // Print device mode
  Serial.println("\n\nBidirectional LoRa Communication Test");
  Serial.println("---------------------------------------");
  Serial.print("Device Mode: ");
  Serial.println(MODE == DEVICE_A ? "A (Sender/Remote)" : "B (Receiver/Base)");
  
  // Initialize the radio
  setupRadio();
  
  // Setup complete
  Serial.println("Setup complete, starting operation");
  digitalWrite(LED_PIN, LOW);
  
  // If device B, start listening immediately
  if (MODE == DEVICE_B) {
    Serial.println("Listening for incoming messages...");
    radio.startReceive();
  }
}

void loop() {
  if (MODE == DEVICE_A) {
    // Device A: Send ping periodically and wait for pong
    unsigned long currentTime = millis();
    
    // Time to send a new ping?
    if (currentTime - lastPingTime >= PING_INTERVAL) {
      lastPingTime = currentTime;
      messageCount++;
      
      Serial.print("\nSending PING #");
      Serial.println(messageCount);
      
      // Send ping
      digitalWrite(LED_PIN, HIGH);  // LED on while sending
      
      // Try to send the message
      if (sendMessage(MSG_PING)) {
        // Wait for response
        Serial.println("Waiting for PONG response...");
        String response;
        
        // Listen for some time
        radio.startReceive();
        unsigned long startTime = millis();
        
        while (millis() - startTime < ACK_TIMEOUT) {
          if (receiveMessage(response)) {
            if (response == MSG_PONG) {
              Serial.println("PONG received! Round-trip successful.");
              successCount++;
              blinkLED(3, 100);  // Blink to show success
              break;
            } else {
              Serial.print("Unexpected response: ");
              Serial.println(response);
            }
          }
          delay(10);  // Short delay to prevent CPU hogging
        }
        
        if (millis() - startTime >= ACK_TIMEOUT) {
          Serial.println("Timeout waiting for PONG response!");
        }
      }
      
      // Print success rate
      Serial.print("Success rate: ");
      Serial.print((successCount * 100.0) / messageCount);
      Serial.println("%");
      
      digitalWrite(LED_PIN, LOW);  // Turn off LED
    }
  } else {
    // Device B: Listen for pings and respond with pongs
    String message;
    
    if (receiveMessage(message)) {
      digitalWrite(LED_PIN, HIGH);  // LED on when receiving
      
      // Check if it's a ping
      if (message == MSG_PING) {
        Serial.println("\nPING received! Sending PONG response...");
        
        // Short delay to let the other device switch to receive mode
        delay(100);
        
        // Send response
        if (sendMessage(MSG_PONG)) {
          Serial.println("PONG sent successfully!");
          blinkLED(2, 100);  // Blink to indicate success
        } else {
          Serial.println("Failed to send PONG response!");
        }
      } else {
        Serial.print("Received unknown message: ");
        Serial.println(message);
      }
      
      digitalWrite(LED_PIN, LOW);  // Turn off LED
      
      // Start listening again
      radio.startReceive();
    }
  }
}

void setupRadio() {
  // Initialize SPI with correct pins
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
  
  // Set pin modes
  pinMode(LORA_CS, OUTPUT);
  pinMode(LORA_BUSY, INPUT);
  pinMode(LORA_DIO1, INPUT);
  
  // Initialize the radio
  Serial.print("Initializing SX1262 with frequency ");
  Serial.print(LORA_FREQUENCY);
  Serial.print(" MHz... ");
  
  int state = radio.begin(LORA_FREQUENCY);
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SUCCESS!");
  } else {
    Serial.print("FAILED! Error code: ");
    Serial.println(state);
    
    // Blink LED rapidly to indicate error
    while (true) {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      delay(100);
    }
  }
  
  // Configure radio parameters
  Serial.println("Setting radio parameters...");
  
  // Enter standby mode first
  radio.standby();
  
  // Set the parameters that worked in our tests
  radio.setBandwidth(LORA_BANDWIDTH);
  radio.setSpreadingFactor(LORA_SPREADING_FACTOR);
  radio.setCodingRate(LORA_CODING_RATE);
  radio.setSyncWord(LORA_SYNC_WORD);
  radio.setOutputPower(LORA_POWER);
  radio.setPreambleLength(LORA_PREAMBLE_LENGTH);
  radio.setCRC(true);
  
  Serial.println("Radio parameters configured successfully.");
}

bool sendMessage(const char* message) {
  Serial.print("Sending: ");
  Serial.println(message);
  
  // Try to send the message
  int state = radio.transmit(message);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("Transmission successful!");
    return true;
  } else {
    Serial.print("Transmission failed! Error code: ");
    Serial.println(state);
    return false;
  }
}

bool receiveMessage(String& message) {
  // Check if a packet is available
  if (!radio.available()) {
    return false;
  }
  
  // Try to receive the packet
  int state = radio.readData(message);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.print("Received: ");
    Serial.println(message);
    
    // Get and print signal quality
    Serial.print("RSSI: ");
    Serial.print(radio.getRSSI());
    Serial.print(" dBm, SNR: ");
    Serial.print(radio.getSNR());
    Serial.println(" dB");
    
    return true;
  } else {
    Serial.print("Reception failed! Error code: ");
    Serial.println(state);
    return false;
  }
}

void blinkLED(int times, int duration) {
  for (int i = 0; i < times; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(duration);
    digitalWrite(LED_PIN, LOW);
    if (i < times - 1) {
      delay(duration);
    }
  }
}
