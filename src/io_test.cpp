#include <Arduino.h>

// GPIO pins to test from the Heltec WiFi LoRa 32 V3 diagram
#define LED_PIN 35  // Onboard LED

// Define pins we want to test - focusing on the LoRa pins
#define NUM_PINS_TO_TEST 7
const uint8_t pinsToTest[NUM_PINS_TO_TEST] = {
  8,   // LoRa NSS/CS
  9,   // LoRa SCK
  10,  // LoRa MOSI
  11,  // LoRa MISO
  12,  // LoRa RST
  13,  // LoRa BUSY
  14   // LoRa DIO1
};

// Function prototypes
void testPins();

// Setup function
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);  // Give time for serial monitor to connect
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("\nHeltec WiFi LoRa 32 V3 - GPIO Test");
  Serial.println("-----------------------------------");
  Serial.println("Testing board connectivity - ESP32-S3 only");
  
  // Show device info
  Serial.print("ESP32 Chip model: ");
  Serial.println(ESP.getChipModel());
  Serial.print("Chip revision: ");
  Serial.println(ESP.getChipRevision());
  Serial.print("Flash size: ");
  Serial.println(ESP.getFlashChipSize() / 1024 / 1024);
  Serial.print("CPU frequency: ");
  Serial.println(ESP.getCpuFreqMHz());
  
  // Test LED
  Serial.println("\nTesting onboard LED (pin 35)...");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
  
  // Perform I/O tests
  Serial.println("\nTesting each GPIO pin as INPUT_PULLUP (they should read HIGH):");
  testPins();
}

// Test each pin as INPUT_PULLUP
void testPins() {
  for (int i = 0; i < NUM_PINS_TO_TEST; i++) {
    uint8_t pin = pinsToTest[i];
    
    // Configure pin as input with pull-up
    pinMode(pin, INPUT_PULLUP);
    delay(10);
    
    // Read the pin value
    int value = digitalRead(pin);
    
    Serial.print("Pin ");
    Serial.print(pin);
    Serial.print(": ");
    
    // We expect HIGH since we've enabled the internal pull-up resistor
    if (value == HIGH) {
      Serial.println("HIGH (Expected ✓)");
    } else {
      Serial.println("LOW  (Unexpected! Pin may be shorted to ground)");
    }
    
    // Brief delay
    delay(50);
  }
  
  // Additional test for device-specific pins
  Serial.println("\nTesting internal pins:");
  
  // Test an analog pin - ESP32-S3 doesn't have hall sensor like ESP32
  Serial.print("ADC reading on pin 1: ");
  pinMode(1, INPUT);
  Serial.println(analogRead(1));
  
  // Print a summary
  Serial.println("\nGPIO Test Completed!");
  Serial.println("If all pins show expected values, your ESP32-S3 GPIO system is working.");
  Serial.println("If any pins show unexpected values, there may be hardware issues.");
  Serial.println("Note: MISO pin may show LOW if LoRa module is actively driving it.");
}

void loop() {
  // Blink the LED to show the program is running
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
