#include <Arduino.h>
#include "lora_communication.h"
#include "power_management.h"
#include "display_manager.h"
#include "metrics.h"

// Pin for a button to cycle display pages (optional)
#define BUTTON_PIN 0  // Usually GPIO0 has a button on ESP32 dev boards

// Interval between data transmissions (when not sleeping)
#define DATA_TRANSMISSION_INTERVAL 30000  // 30 seconds

// Last transmission time
unsigned long lastTransmissionTime = 0;

// Function prototypes
void setupHardware();
void transmitMetricsData();
void handleButton();
void printDebugInfo();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println(F("\n\nLoRa Remote Device Starting..."));
  
  // Initialize hardware
  setupHardware();
  
  // Display welcome message
  displayManager.showStatus("System Ready");
  
  // Initial metrics transmission
  transmitMetricsData();
}

void loop() {
  // Check for button press to cycle display pages
  handleButton();
  
  // Update display
  displayManager.update();
  
  // Update metrics
  metrics.update();
  
  // Check if it's time to transmit metrics
  if (millis() - lastTransmissionTime >= DATA_TRANSMISSION_INTERVAL) {
    transmitMetricsData();
  }
  
  // Print debug info periodically
  printDebugInfo();
  
  // Check battery status and sleep if needed
  if (powerManagement.getBatteryStatus() != BATTERY_STATUS_NORMAL) {
    // Prepare for sleep
    loraCommunication.sleep();
    
    // Enter sleep mode
    powerManagement.smartSleep();
    
    // Wake up LoRa after sleep
    loraCommunication.wakeup();
    
    // Reset transmission timer
    lastTransmissionTime = millis();
  }
  
  // Small delay to prevent CPU hogging
  delay(100);
}

void setupHardware() {
  // Set up button pin if used
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize power management
  Serial.println(F("Initializing power management..."));
  powerManagement.begin();
  
  // Initialize LoRa communication
  Serial.println(F("Initializing LoRa communication..."));
  if (!loraCommunication.begin()) {
    displayManager.showStatus("LoRa Init Failed");
    Serial.println(F("Failed to initialize LoRa!"));
    while (1) {
      // Flash the LED or display an error message
      delay(500);
    }
  }
  
  // Initialize display
  Serial.println(F("Initializing display..."));
  if (!displayManager.begin()) {
    Serial.println(F("Failed to initialize display!"));
    // Continue anyway, display is non-critical
  }
  
  // Initialize metrics system
  Serial.println(F("Initializing metrics system..."));
  metrics.begin();
  
  // Update power metrics display
  float batteryVoltage = powerManagement.getBatteryVoltage();
  uint8_t batteryPercentage = powerManagement.getBatteryPercentage();
  bool isCharging = (powerManagement.getChargingStatus() == CHARGING);
  
  displayManager.updatePowerMetrics(batteryVoltage, batteryPercentage, isCharging);
  
  Serial.println(F("Hardware initialization complete"));
}

void transmitMetricsData() {
  // Create metrics document
  StaticJsonDocument<512> metricsDoc;
  
  // Add system metrics
  metrics.getSystemMetrics(metricsDoc);
  
  // Add power metrics
  metricsDoc["battery"] = powerManagement.getBatteryVoltage();
  metricsDoc["battery_percent"] = powerManagement.getBatteryPercentage();
  metricsDoc["charging"] = (powerManagement.getChargingStatus() == CHARGING ? 1 : 0);
  
  // Create a temporary document for performance metrics
  StaticJsonDocument<256> perfDoc;
  
  // Get performance metrics
  metrics.getPerformanceMetrics(perfDoc);
  
  // Copy all performance metrics to the main document
  for (JsonPair kv : perfDoc.as<JsonObject>()) {
    metricsDoc[kv.key()] = kv.value();
  }
  
  // Display status
  displayManager.showStatus("Sending data...");
  
  // Send metrics to base station
  int rssi = 0;
  float snr = 0;
  unsigned long startTime = millis();
  
  bool success = loraCommunication.sendMetrics(metricsDoc);
  
  unsigned long latency = millis() - startTime;
  
  // Record transmission in metrics
  metrics.recordTransmission(
    metricsDoc["id"] | 0,  // Extract ID or use 0 if not available
    success,
    rssi,
    snr,
    0,  // retries
    latency
  );
  
  // Update display with new signal metrics
  displayManager.updateSignalMetrics(rssi, snr, latency);
  
  // Update power metrics display
  float batteryVoltage = powerManagement.getBatteryVoltage();
  uint8_t batteryPercentage = powerManagement.getBatteryPercentage();
  bool isCharging = (powerManagement.getChargingStatus() == CHARGING);
  
  displayManager.updatePowerMetrics(batteryVoltage, batteryPercentage, isCharging);
  
  // Update status
  if (success) {
    displayManager.showStatus("Data sent successfully");
  } else {
    displayManager.showStatus("Failed to send data");
  }
  
  // Update last transmission time
  lastTransmissionTime = millis();
  
  Serial.println(success ? F("Data sent successfully") : F("Failed to send data"));
}

void handleButton() {
  // Check if button is pressed (active low)
  static bool lastButtonState = HIGH;
  static unsigned long lastDebounceTime = 0;
  static const unsigned long debounceDelay = 50;
  
  // Read the current button state
  bool buttonState = digitalRead(BUTTON_PIN);
  
  // Check if the button state has changed
  if (buttonState != lastButtonState) {
    // Reset the debounce timer
    lastDebounceTime = millis();
  }
  
  // Only change the current page if the button has been stable for the debounce period
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // If the button state has changed and it's now pressed
    if (buttonState == LOW && lastButtonState == HIGH) {
      // Cycle to the next display page
      displayManager.nextPage();
      Serial.println(F("Display page changed"));
    }
  }
  
  // Save the current button state for the next comparison
  lastButtonState = buttonState;
}

void printDebugInfo() {
  static unsigned long lastDebugTime = 0;
  const unsigned long debugInterval = 10000;  // 10 seconds
  
  // Only print debug info if enough time has passed
  if (millis() - lastDebugTime < debugInterval) {
    return;
  }
  
  // Print debug information
  Serial.println(F("\n--- Debug Information ---"));
  
  // Battery info
  Serial.print(F("Battery: "));
  Serial.print(powerManagement.getBatteryVoltage());
  Serial.print(F("V ("));
  Serial.print(powerManagement.getBatteryPercentage());
  Serial.print(F("%), Status: "));
  
  switch (powerManagement.getBatteryStatus()) {
    case BATTERY_STATUS_NORMAL:
      Serial.print(F("Normal"));
      break;
    case BATTERY_STATUS_LOW:
      Serial.print(F("Low"));
      break;
    case BATTERY_STATUS_CRITICAL:
      Serial.print(F("Critical"));
      break;
    default:
      Serial.print(F("Unknown"));
      break;
  }
  
  Serial.print(F(", Charging: "));
  Serial.println(powerManagement.getChargingStatus() == CHARGING ? F("Yes") : F("No"));
  
  // Signal info
  Serial.print(F("Signal: RSSI "));
  Serial.print(metrics.getAverageRSSI());
  Serial.print(F("dBm, SNR "));
  Serial.print(metrics.getAverageSNR());
  Serial.println(F("dB"));
  
  // Performance info
  Serial.print(F("Success rate: "));
  Serial.print(metrics.getPacketSuccessRate() * 100);
  Serial.print(F("%, Latency: "));
  Serial.print(metrics.getAverageLatency());
  Serial.println(F("ms"));
  
  // System info
  Serial.print(F("Uptime: "));
  unsigned long uptime = millis() / 1000;
  Serial.print(uptime / 3600);  // Hours
  Serial.print(F("h "));
  Serial.print((uptime % 3600) / 60);  // Minutes
  Serial.print(F("m "));
  Serial.print(uptime % 60);  // Seconds
  Serial.println(F("s"));
  
  Serial.println(F("------------------------\n"));
  
  // Update last debug time
  lastDebugTime = millis();
  
  // Update debug info on display
  char debugInfo[64];
  snprintf(debugInfo, sizeof(debugInfo), "Batt: %.1fV, RSSI: %d, SR: %.0f%%", 
           powerManagement.getBatteryVoltage(),
           metrics.getAverageRSSI(),
           metrics.getPacketSuccessRate() * 100);
  
  displayManager.showDebugInfo(debugInfo);
}
