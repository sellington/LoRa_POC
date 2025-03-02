#include <Arduino.h>
#include <SPI.h>

// Pin definitions from the actual Heltec WiFi LoRa 32 Pin Map
// Set #1 - From the provided pin map image
#define LORA_CS_1         8  // LoRa_NSS (Chip Select)
#define LORA_SCK_1        9  // LoRa_SCK (Clock)
#define LORA_MOSI_1       10 // LoRa_MOSI
#define LORA_MISO_1       11 // LoRa_MISO
#define LORA_RST_1        12 // LoRa_RST (Reset)
#define LORA_BUSY_1       13 // LoRa_BUSY 
#define LORA_DIO1_1       14 // DIO1

// Set #2 - Alternative mapping (sometimes physical pin numbers differ from GPIO numbers)
#define LORA_CS_2         8
#define LORA_SCK_2        9
#define LORA_MOSI_2       10
#define LORA_MISO_2       11
#define LORA_RST_2        12
#define LORA_BUSY_2       13
#define LORA_DIO1_2       14

// Set #3 - Another alternative from community feedback
#define LORA_SCK_3        36
#define LORA_MISO_3       37
#define LORA_MOSI_3       35
#define LORA_CS_3         34
#define LORA_RST_3        38
#define LORA_DIO1_3       39

// Common definitions
#define LED             35   // Built-in LED

// SX1262 registers for direct access
#define REG_VERSION     0x42  // Version register, should return 0x12 for SX1262

// Active pin set (will try each set)
uint8_t SCK_PIN, MISO_PIN, MOSI_PIN, CS_PIN, RST_PIN, DIO1_PIN, BUSY_PIN;

// Function prototypes
bool testSpiConnection(uint8_t cs, uint8_t sck, uint8_t miso, uint8_t mosi);
uint8_t readRegister(uint8_t reg);

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);  // Time to open serial monitor
  
  // Initialize LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  Serial.println("\nHeltec WiFi LoRa 32 V3 - SPI Connection Test");
  Serial.println("---------------------------------------------");
  
  // Store success status for each pin configuration
  bool set1Success = false;
  bool set2Success = false;
  bool set3Success = false;
  
  // Test pin set #1
  Serial.println("\nTesting Pin Configuration #1 (Heltec documentation):");
  SCK_PIN = LORA_SCK_1;
  MISO_PIN = LORA_MISO_1;
  MOSI_PIN = LORA_MOSI_1;
  CS_PIN = LORA_CS_1;
  RST_PIN = LORA_RST_1;
  DIO1_PIN = LORA_DIO1_1;
  BUSY_PIN = LORA_BUSY_1;
  
  Serial.print("SCK: "); Serial.println(SCK_PIN);
  Serial.print("MISO: "); Serial.println(MISO_PIN);
  Serial.print("MOSI: "); Serial.println(MOSI_PIN);
  Serial.print("CS: "); Serial.println(CS_PIN);
  Serial.print("RST: "); Serial.println(RST_PIN);
  Serial.print("DIO1: "); Serial.println(DIO1_PIN);
  Serial.print("BUSY: "); Serial.println(BUSY_PIN);
  
  set1Success = testSpiConnection(CS_PIN, SCK_PIN, MISO_PIN, MOSI_PIN);
  
  // Test pin set #2
  Serial.println("\nTesting Pin Configuration #2 (Legacy Heltec):");
  SCK_PIN = LORA_SCK_2;
  MISO_PIN = LORA_MISO_2;
  MOSI_PIN = LORA_MOSI_2;
  CS_PIN = LORA_CS_2;
  RST_PIN = LORA_RST_2;
  DIO1_PIN = LORA_DIO1_2;
  
  Serial.print("SCK: "); Serial.println(SCK_PIN);
  Serial.print("MISO: "); Serial.println(MISO_PIN);
  Serial.print("MOSI: "); Serial.println(MOSI_PIN);
  Serial.print("CS: "); Serial.println(CS_PIN);
  Serial.print("RST: "); Serial.println(RST_PIN);
  Serial.print("DIO1: "); Serial.println(DIO1_PIN);
  
  set2Success = testSpiConnection(CS_PIN, SCK_PIN, MISO_PIN, MOSI_PIN);
  
  // Test pin set #3
  Serial.println("\nTesting Pin Configuration #3 (Community option):");
  SCK_PIN = LORA_SCK_3;
  MISO_PIN = LORA_MISO_3;
  MOSI_PIN = LORA_MOSI_3;
  CS_PIN = LORA_CS_3;
  RST_PIN = LORA_RST_3;
  DIO1_PIN = LORA_DIO1_3;
  
  Serial.print("SCK: "); Serial.println(SCK_PIN);
  Serial.print("MISO: "); Serial.println(MISO_PIN);
  Serial.print("MOSI: "); Serial.println(MOSI_PIN);
  Serial.print("CS: "); Serial.println(CS_PIN);
  Serial.print("RST: "); Serial.println(RST_PIN);
  Serial.print("DIO1: "); Serial.println(DIO1_PIN);
  
  set3Success = testSpiConnection(CS_PIN, SCK_PIN, MISO_PIN, MOSI_PIN);
  
  // Summary of results
  Serial.println("\n----- TEST RESULTS -----");
  Serial.print("Pin Set #1 (Heltec docs): ");
  Serial.println(set1Success ? "SUCCESS" : "FAILED");
  
  Serial.print("Pin Set #2 (Legacy): ");
  Serial.println(set2Success ? "SUCCESS" : "FAILED");
  
  Serial.print("Pin Set #3 (Community): ");
  Serial.println(set3Success ? "SUCCESS" : "FAILED");
  
  // Final recommendation
  Serial.println("\n----- RECOMMENDATION -----");
  if (set1Success || set2Success || set3Success) {
    Serial.println("SPI connection successful with at least one pin configuration.");
    Serial.println("Use the successful pin set in your RadioLib configuration.");
    
    if (set1Success) {
      Serial.println("Recommended: Use Pin Set #1 (Heltec documentation)");
    } else if (set2Success) {
      Serial.println("Recommended: Use Pin Set #2 (Legacy Heltec)");
    } else {
      Serial.println("Recommended: Use Pin Set #3 (Community option)");
    }
  } else {
    Serial.println("All pin configurations failed. Check your hardware connections.");
    Serial.println("The LoRa module may be damaged or incorrectly wired.");
  }
  
  // Visual indicator - Blink LED based on test results
  if (set1Success || set2Success || set3Success) {
    // Success - solid LED
    digitalWrite(LED, HIGH);
  } else {
    // Failure - rapid blinking
    while (1) {
      digitalWrite(LED, HIGH);
      delay(100);
      digitalWrite(LED, LOW);
      delay(100);
    }
  }
}

void loop() {
  // Nothing to do in loop
  delay(1000);
}

// Test SPI connection by trying to read the Version register of SX1262
bool testSpiConnection(uint8_t cs, uint8_t sck, uint8_t miso, uint8_t mosi) {
  // Initialize pins
  pinMode(cs, OUTPUT);
  pinMode(sck, OUTPUT);
  pinMode(mosi, OUTPUT);
  pinMode(miso, INPUT);
  
  // Start with CS high (inactive)
  digitalWrite(cs, HIGH);
  
  // End any previous SPI transactions
  SPI.end();
  
  // Initialize SPI with the given pins
  Serial.println("Initializing SPI...");
  SPI.begin(sck, miso, mosi, cs);
  
  // Try different SPI settings
  for (int mode = 0; mode <= 3; mode++) {
    for (int speed = 0; speed < 3; speed++) {
      uint32_t clock = 1000000; // Start with 1 MHz
      if (speed == 1) clock = 2000000; // 2 MHz
      if (speed == 2) clock = 4000000; // 4 MHz
      
      SPI.beginTransaction(SPISettings(clock, MSBFIRST, mode));
      
      Serial.print("Testing SPI Mode ");
      Serial.print(mode);
      Serial.print(" at ");
      Serial.print(clock / 1000000);
      Serial.print(" MHz: ");
      
      uint8_t version = readRegister(REG_VERSION);
      
      Serial.print("0x");
      Serial.println(version, HEX);
      
      if (version == 0x12 || version == 0x22) {
        Serial.println("SUCCESS! Found SX126x chip");
        SPI.endTransaction();
        return true;
      }
      
      SPI.endTransaction();
      delay(10);
    }
  }
  
  Serial.println("No SX126x chip detected with this pin configuration");
  return false;
}

// Read a register from the SX1262
uint8_t readRegister(uint8_t reg) {
  digitalWrite(CS_PIN, LOW);
  SPI.transfer(reg & 0x7F); // Read operation (bit 7 = 0)
  uint8_t response = SPI.transfer(0x00);
  digitalWrite(CS_PIN, HIGH);
  return response;
}
