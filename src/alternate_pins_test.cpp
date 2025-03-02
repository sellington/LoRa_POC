#include <Arduino.h>

// GPIO pins to test from the Heltec WiFi LoRa 32 V3 diagram
#define LED_PIN 35  // Onboard LED

// Define the problematic pins we identified
#define BUSY_PIN_ORIGINAL 13
#define DIO1_PIN_ORIGINAL 14

// Define alternative pins to test as replacements
// Choosing pins that are likely to be free and usable for digital I/O
#define NUM_ALT_PINS 6
const uint8_t alternativePins[NUM_ALT_PINS] = {
  15, // Not used by default in LoRa connections
  16, // Not used by default in LoRa connections
  17, // Not used by default in LoRa connections
  18, // Not used by default in LoRa connections
  21, // Not used by default in LoRa connections
  47  // Not used by default in LoRa connections
};

// Function prototypes
void testAlternativePins();
void toggleLED(int count);

// Setup function
void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);  // Give time for serial monitor to connect
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  
  Serial.println("\nHeltec WiFi LoRa 32 V3 - Alternative Pin Test");
  Serial.println("--------------------------------------------");
  Serial.println("Testing alternative pins for BUSY and DIO1 functionality");
  
  // Confirm problematic pins
  Serial.println("\nVerifying previously identified problematic pins:");
  
  Serial.print("BUSY Pin (13): ");
  pinMode(BUSY_PIN_ORIGINAL, INPUT_PULLUP);
  delay(10);
  int busy_value = digitalRead(BUSY_PIN_ORIGINAL);
  Serial.println(busy_value == HIGH ? "HIGH (OK)" : "LOW (Problem confirmed)");
  
  Serial.print("DIO1 Pin (14): ");
  pinMode(DIO1_PIN_ORIGINAL, INPUT_PULLUP);
  delay(10);
  int dio1_value = digitalRead(DIO1_PIN_ORIGINAL);
  Serial.println(dio1_value == HIGH ? "HIGH (OK)" : "LOW (Problem confirmed)");
  
  // Test alternative pins
  Serial.println("\nTesting alternative pins as potential replacements:");
  testAlternativePins();
  
  // Signal completion with LED flashes
  toggleLED(5);
}

// Test each alternative pin as INPUT_PULLUP
void testAlternativePins() {
  for (int i = 0; i < NUM_ALT_PINS; i++) {
    uint8_t pin = alternativePins[i];
    
    // Configure pin as input with pull-up
    pinMode(pin, INPUT_PULLUP);
    delay(10);
    
    // Read the pin value
    int value = digitalRead(pin);
    
    Serial.print("Pin ");
    Serial.print(pin);
    Serial.print(": ");
    
    // We expect HIGH if pin is usable
    if (value == HIGH) {
      Serial.print("HIGH (Usable as replacement) - ");
      
      // Try the pin as OUTPUT
      pinMode(pin, OUTPUT);
      digitalWrite(pin, HIGH);
      delay(10);
      digitalWrite(pin, LOW);
      delay(10);
      digitalWrite(pin, HIGH);
      
      Serial.println("OUTPUT test successful");
      
      // Return to INPUT_PULLUP for next test
      pinMode(pin, INPUT_PULLUP);
    } else {
      Serial.println("LOW (Not usable as replacement)");
    }
    
    delay(50);
  }
  
  // Recommendations for alternative pins
  Serial.println("\n--- RECOMMENDATIONS ---");
  Serial.println("Based on the tests, consider these pin replacements in your code:");
  Serial.println("1. Replace BUSY_PIN (13) with: [First HIGH pin from the list]");
  Serial.println("2. Replace DIO1_PIN (14) with: [Second HIGH pin from the list]");
  Serial.println("\nTo implement this change:");
  Serial.println("- Update the LORA_BUSY and LORA_DIO1 definitions in your code");
  Serial.println("- Make sure to update both remote_device and base_station configurations");
  
  Serial.println("\nPin Replacement Test Completed!");
}

// Toggle LED for visual indications
void toggleLED(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(200);
  }
}

void loop() {
  // Blink the LED slowly to show the program is running
  digitalWrite(LED_PIN, HIGH);
  delay(1000);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
}
