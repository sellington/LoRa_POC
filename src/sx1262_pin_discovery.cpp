#include <Arduino.h>
#include <RadioLib.h>

// Define all GPIO pins we want to test with the SX1262
// Based on examining the schematic and previous tests
#define LED_PIN 35  // Built-in LED for visual feedback

// Original pin definitions from the Heltec WiFi LoRa 32 V3 pinout diagram
#define LORA_CS      8     // LoRa_NSS (Chip Select)
#define LORA_SCK     9     // LoRa_SCK (Clock)
#define LORA_MOSI    10    // LoRa_MOSI
#define LORA_MISO    11    // LoRa_MISO
#define LORA_RST     12    // LoRa_RST (Reset)
// Test both original and alternative pins for BUSY and DIO1
#define LORA_BUSY_1  13    // Original BUSY pin (suspected issue)
#define LORA_BUSY_2  15    // Alternative BUSY pin
#define LORA_DIO1_1  14    // Original DIO1 pin (suspected issue)
#define LORA_DIO1_2  16    // Alternative DIO1 pin

// Define additional pins to test as potential BUSY or DIO1 alternatives
#define TEST_PIN_1   17
#define TEST_PIN_2   18
#define TEST_PIN_3   21
#define TEST_PIN_4   33
#define TEST_PIN_5   34
#define TEST_PIN_6   36
#define TEST_PIN_7   37
#define TEST_PIN_8   38
#define TEST_PIN_9   39
#define TEST_PIN_10  40

// Define the combinations to test
struct PinCombo {
  uint8_t busy;
  uint8_t dio1;
  const char* name;
};

// Create an array of combinations to test
PinCombo combos[] = {
  {LORA_BUSY_1, LORA_DIO1_1, "Original pins (13, 14)"},
  {LORA_BUSY_2, LORA_DIO1_2, "Alternative pins (15, 16)"},
  {LORA_BUSY_1, LORA_DIO1_2, "Mixed: Original BUSY (13), Alt DIO1 (16)"},
  {LORA_BUSY_2, LORA_DIO1_1, "Mixed: Alt BUSY (15), Original DIO1 (14)"},
  {TEST_PIN_1, LORA_DIO1_1, "Test BUSY=17, DIO1=14"},
  {TEST_PIN_1, LORA_DIO1_2, "Test BUSY=17, DIO1=16"},
  {LORA_BUSY_1, TEST_PIN_1, "Test BUSY=13, DIO1=17"},
  {LORA_BUSY_2, TEST_PIN_1, "Test BUSY=15, DIO1=17"},
  {TEST_PIN_2, TEST_PIN_3, "Test BUSY=18, DIO1=21"},
  {TEST_PIN_4, TEST_PIN_5, "Test BUSY=33, DIO1=34"},
  {TEST_PIN_6, TEST_PIN_7, "Test BUSY=36, DIO1=37"},
  {TEST_PIN_8, TEST_PIN_9, "Test BUSY=38, DIO1=39"},
  {TEST_PIN_9, TEST_PIN_10, "Test BUSY=39, DIO1=40"}
};

// Current index in the combinations array
int currentCombo = 0;
const int totalCombos = sizeof(combos) / sizeof(combos[0]);

// Radio instance - will be created for each combination
SX1262* radio = nullptr;

// Flag to indicate if the radio initialized successfully
bool initSuccess = false;

// Simple packet to send
uint8_t testByte = 0x42; // ASCII 'B'

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  delay(3000);  // Time to open serial monitor
  
  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);  // LED on during setup
  
  Serial.println("\nHeltec WiFi LoRa 32 V3 - SX1262 Pin Discovery");
  Serial.println("--------------------------------------------");
  
  // Initialize SPI explicitly for the LoRa module
  SPI.begin(LORA_SCK, LORA_MISO, LORA_MOSI, LORA_CS);
  
  // Setup complete, turn off LED
  digitalWrite(LED_PIN, LOW);
  
  // Testing will start in the loop
  Serial.println("Testing will begin with the first pin combination...");
  Serial.print("Total combinations to test: ");
  Serial.println(totalCombos);
}

void testCombination(PinCombo combo) {
  Serial.println("\n---------------------------------------");
  Serial.print("Testing combination: ");
  Serial.println(combo.name);
  Serial.print("BUSY pin: ");
  Serial.print(combo.busy);
  Serial.print(", DIO1 pin: ");
  Serial.println(combo.dio1);
  
  // Reset all pins to input before testing
  pinMode(combo.busy, INPUT);
  pinMode(combo.dio1, INPUT);
  
  // Create a new Module instance for testing
  Module* mod = new Module(LORA_CS, combo.dio1, LORA_RST, combo.busy);
  
  // Delete previous radio instance if it exists
  if (radio != nullptr) {
    delete radio;
    radio = nullptr;
  }
  
  // Create a new radio instance with this pin combination
  radio = new SX1262(mod);
  
  // Reset the radio
  Serial.println("Resetting SX1262 module...");
  pinMode(LORA_RST, OUTPUT);
  digitalWrite(LORA_RST, LOW);
  delay(10);
  digitalWrite(LORA_RST, HIGH);
  delay(100);
  
  // Try to initialize the radio
  Serial.print("Initializing SX1262 with 915 MHz frequency... ");
  int state = radio->begin(915.0);
  
  if (state == RADIOLIB_ERR_NONE) {
    Serial.println("SUCCESS!");
    initSuccess = true;
    
    // Configure minimal settings
    Serial.println("Setting minimal parameters...");
    radio->standby();
    radio->setOutputPower(2);                 // Lower power
    radio->setSpreadingFactor(6);             // Lowest SF
    radio->setBandwidth(500.0);               // Maximum bandwidth
    radio->setCodingRate(5);                  // 4/5 coding rate
    radio->setPreambleLength(4);              // Minimum preamble
    
    // Try to transmit a minimal packet
    Serial.println("Attempting to transmit a single byte...");
    digitalWrite(LED_PIN, HIGH);
    state = radio->transmit(&testByte, 1);
    digitalWrite(LED_PIN, LOW);
    
    if (state == RADIOLIB_ERR_NONE) {
      Serial.println("SUCCESS: Packet transmitted!");
      Serial.println("*** WORKING COMBINATION FOUND! ***");
      Serial.print("Use BUSY pin: ");
      Serial.print(combo.busy);
      Serial.print(" and DIO1 pin: ");
      Serial.println(combo.dio1);
    } else {
      Serial.print("Failed to transmit, error code: ");
      Serial.println(state);
      
      // Detailed error
      switch(state) {
        case RADIOLIB_ERR_PACKET_TOO_LONG:
          Serial.println("Error: Packet too long!");
          break;
        case RADIOLIB_ERR_TX_TIMEOUT:
          Serial.println("Error: Transmission timed out!");
          break;
        case RADIOLIB_ERR_SPI_CMD_TIMEOUT:
          Serial.println("Error: SPI command timed out!");
          break;
        case RADIOLIB_ERR_CHIP_NOT_FOUND:
          Serial.println("Error: Chip not found!");
          break;
        default:
          Serial.println("Error: Unknown error!");
      }
    }
  } else {
    Serial.print("FAILED, error code: ");
    Serial.println(state);
    initSuccess = false;
  }
  
  Serial.println("Test complete for this combination.");
  Serial.println("---------------------------------------");
}

void loop() {
  // Test each combination with delays in between
  if (currentCombo < totalCombos) {
    testCombination(combos[currentCombo]);
    currentCombo++;
    
    if (currentCombo < totalCombos) {
      // More combinations to test, wait before proceeding
      Serial.print("Next test in 5 seconds... ");
      
      // Visual countdown with LED
      for (int i = 5; i > 0; i--) {
        Serial.print(i);
        Serial.print(" ");
        digitalWrite(LED_PIN, HIGH);
        delay(500);
        digitalWrite(LED_PIN, LOW);
        delay(500);
      }
      Serial.println();
    } else {
      // All tests completed
      Serial.println("\n===== ALL TESTS COMPLETED =====");
      Serial.println("Check the results above to find working combinations.");
      Serial.println("The loop will now pause to prevent repeating tests.");
      
      // Blink LED to signal completion
      for (int i = 0; i < 10; i++) {
        digitalWrite(LED_PIN, HIGH);
        delay(200);
        digitalWrite(LED_PIN, LOW);
        delay(200);
      }
    }
  } else {
    // All combinations tested, just blink LED slowly to indicate completion
    digitalWrite(LED_PIN, HIGH);
    delay(1000);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
  }
}
