#include <Arduino.h>
#include <SPI.h>

// LoRa module pins for ESP32-S3
#define LORA_CS_PIN    10  // SPI CS
#define LORA_RST_PIN   11  // Reset
#define LORA_DIO0_PIN  12  // DIO0 (IRQ)
#define LORA_DIO1_PIN  13  // DIO1

// SPI pins for ESP32-S3
#define SPI_SCK_PIN    36
#define SPI_MISO_PIN   37
#define SPI_MOSI_PIN   35

// SX1276 Registers (for direct access testing)
#define REG_VERSION    0x42  // Version register - should return 0x12 for SX1276

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(2000);  // Allow time for serial monitor to connect
  
  Serial.println("\n\nSPI Communication Test for LoRa Module");
  
  // Initialize SPI pins
  pinMode(LORA_CS_PIN, OUTPUT);
  pinMode(LORA_RST_PIN, OUTPUT);
  pinMode(LORA_DIO0_PIN, INPUT);
  pinMode(LORA_DIO1_PIN, INPUT);
  
  // Initialize SPI
  SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, LORA_CS_PIN);
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE0));
  
  Serial.println("SPI initialized");
  
  // Reset the LoRa module
  digitalWrite(LORA_CS_PIN, HIGH); // Deselect chip
  
  // Hardware reset
  digitalWrite(LORA_RST_PIN, LOW);
  delay(10);
  digitalWrite(LORA_RST_PIN, HIGH);
  delay(100);
  
  Serial.println("LoRa module reset complete");
  
  // Try to read the version register
  digitalWrite(LORA_CS_PIN, LOW); // Select chip
  SPI.transfer(REG_VERSION & 0x7F); // Read operation (MSB = 0)
  uint8_t version = SPI.transfer(0x00); // Read the register value
  digitalWrite(LORA_CS_PIN, HIGH); // Deselect chip
  
  Serial.print("SX1276 Version Register (0x42) value: 0x");
  Serial.println(version, HEX);
  
  if (version == 0x12) {
    Serial.println("SUCCESS! Found SX1276 chip with correct version (0x12)");
  } else {
    Serial.println("ERROR! Could not communicate with SX1276 or wrong version detected");
    
    // Try with different SPI modes
    Serial.println("\nTrying with different SPI modes...");
    
    for (int mode = 0; mode <= 3; mode++) {
      SPI.endTransaction();
      SPI.beginTransaction(SPISettings(1000000, MSBFIRST, mode));
      
      Serial.print("Testing SPI_MODE");
      Serial.print(mode);
      Serial.print(": ");
      
      digitalWrite(LORA_CS_PIN, LOW);
      SPI.transfer(REG_VERSION & 0x7F);
      version = SPI.transfer(0x00);
      digitalWrite(LORA_CS_PIN, HIGH);
      
      Serial.print("Version: 0x");
      Serial.println(version, HEX);
      
      if (version == 0x12) {
        Serial.print("SUCCESS with SPI_MODE");
        Serial.println(mode);
        break;
      }
      
      delay(100);
    }
  }
  
  Serial.println("\nTrying with different CS pins...");
  // Try a few different potential CS pins
  uint8_t potential_cs_pins[] = {10, 5, 7, 8, 9, 15};
  
  for (int i = 0; i < sizeof(potential_cs_pins); i++) {
    uint8_t cs_pin = potential_cs_pins[i];
    
    // Skip if same as current CS pin
    if (cs_pin == LORA_CS_PIN) continue;
    
    pinMode(cs_pin, OUTPUT);
    digitalWrite(cs_pin, HIGH);
    
    Serial.print("Testing CS Pin ");
    Serial.print(cs_pin);
    Serial.print(": ");
    
    SPI.endTransaction();
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, cs_pin);
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    
    digitalWrite(cs_pin, LOW);
    SPI.transfer(REG_VERSION & 0x7F);
    version = SPI.transfer(0x00);
    digitalWrite(cs_pin, HIGH);
    
    Serial.print("Version: 0x");
    Serial.println(version, HEX);
    
    if (version == 0x12) {
      Serial.print("SUCCESS with CS Pin ");
      Serial.println(cs_pin);
      break;
    }
    
    delay(100);
  }
  
  Serial.println("\nSPI test complete");
}

void loop() {
  // Nothing to do in loop
  delay(1000);
}
