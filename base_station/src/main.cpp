#include <Arduino.h>
#include "lora_communication.h"
#include "display_manager.h"
#include "serial_manager.h"

// Pin for a button to cycle display pages (optional)
#define BUTTON_PIN 0  // Usually GPIO0 has a button on ESP32 dev boards

// Statistics
unsigned long totalPacketsReceived = 0;
unsigned long errorPackets = 0;
unsigned long lastPacketTime = 0;
unsigned long uptimeStart = 0;

// Remote device status
float remoteBatteryVoltage = 0.0;
uint8_t remoteBatteryPercentage = 0;
bool remoteIsCharging = false;
unsigned long remoteLastSeen = 0;

// Signal metrics
int lastRssi = -120;
float lastSnr = 0.0;
float packetLossRate = 0.0;
float avgLatency = 0.0;

// Function prototypes
void setupHardware();
void handleButton();
void handleIncomingMessage(const char* type, JsonDocument& doc, int rssi, float snr);
void updateRemoteMetrics(JsonDocument& doc);
void updateSignalMetrics(int rssi, float snr);
void updateDisplay();
void checkSerialCommands();
void sendStatusToSerial();

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println(F("\n\nLoRa Base Station Starting..."));
  
  // Record start time for uptime calculation
  uptimeStart = millis();
  
  // Initialize hardware
  setupHardware();
  
  // Display welcome message
  displayManager.showStatus("Base Station Ready");
  serialManager.sendStatus("Base Station Ready");
}

void loop() {
  // Check for button press to cycle display pages
  handleButton();
  
  // Check for incoming LoRa messages
  loraCommunication.checkForIncomingMessages(handleIncomingMessage);
  
  // Update the display
  updateDisplay();
  
  // Process any serial commands
  checkSerialCommands();
  
  // Small delay to prevent CPU hogging
  delay(10);
}

void setupHardware() {
  // Set up button pin if used
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize LoRa communication
  Serial.println(F("Initializing LoRa communication..."));
  if (!loraCommunication.begin()) {
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
  
  // Initialize serial manager
  Serial.println(F("Initializing serial manager..."));
  serialManager.begin();
  
  Serial.println(F("Hardware initialization complete"));
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

void handleIncomingMessage(const char* type, JsonDocument& doc, int rssi, float snr) {
  // Update signal metrics
  updateSignalMetrics(rssi, snr);
  
  // Record message receipt
  totalPacketsReceived++;
  lastPacketTime = millis();
  
  // Send data to serial
  serialManager.sendRemoteData(doc);
  
  // Handle different message types
  if (strcmp(type, MSG_TYPE_DATA) == 0) {
    // Update remote device metrics
    updateRemoteMetrics(doc);
    
    // Update display with remote status
    displayManager.showStatus("Data Received");
    
    // Log the data receipt
    serialManager.log("Data received from remote device");
  }
  else if (strcmp(type, MSG_TYPE_STATUS) == 0) {
    // Update remote device metrics
    updateRemoteMetrics(doc);
    
    // Display status message if present
    if (doc.containsKey("payload")) {
      const char* status = doc["payload"];
      displayManager.showStatus(status);
      serialManager.sendStatus(status);
    } else {
      displayManager.showStatus("Status Received");
    }
    
    // Log the status receipt
    serialManager.log("Status update received from remote device");
  }
  else if (strcmp(type, MSG_TYPE_PING) == 0) {
    // Ping was already automatically acknowledged by lora_communication
    displayManager.showStatus("Ping Received");
    serialManager.log("Ping received from remote device");
  }
}

void updateRemoteMetrics(JsonDocument& doc) {
  // Extract battery information if present
  if (doc.containsKey("metrics")) {
    JsonObject metrics = doc["metrics"];
    
    if (metrics.containsKey("battery")) {
      remoteBatteryVoltage = metrics["battery"];
    }
    
    if (metrics.containsKey("battery_percent")) {
      remoteBatteryPercentage = metrics["battery_percent"];
    }
    
    if (metrics.containsKey("charging")) {
      remoteIsCharging = (metrics["charging"] == 1);
    }
    
    // Update last seen time
    remoteLastSeen = millis();
    
    // Update display with remote status
    unsigned long lastSeenSeconds = (millis() - remoteLastSeen) / 1000;
    displayManager.updateRemoteStatus(remoteBatteryVoltage, remoteBatteryPercentage, remoteIsCharging, lastSeenSeconds);
  }
}

void updateSignalMetrics(int rssi, float snr) {
  // Update signal metrics
  lastRssi = rssi;
  lastSnr = snr;
  
  // Calculate packet loss rate (placeholder logic)
  // In real implementation, would track expected vs received packets
  packetLossRate = 0.05;  // Example: 5% loss
  
  // Calculate average latency (placeholder)
  // In real implementation, would track round-trip times
  avgLatency = 150.0;  // Example: 150ms
  
  // Update display with signal metrics
  displayManager.updateSignalMetrics(lastRssi, lastSnr, packetLossRate, avgLatency);
  
  // Send to serial
  serialManager.sendSignalMetrics(lastRssi, lastSnr, packetLossRate, avgLatency);
}

void updateDisplay() {
  // Update system metrics
  unsigned long uptime = (millis() - uptimeStart) / 1000;  // In seconds
  displayManager.updateSystemMetrics(uptime, totalPacketsReceived, errorPackets);
  
  // Update the display
  displayManager.update();
}

void checkSerialCommands() {
  // Process any pending serial commands
  serialManager.processCommands();
  
  // If there's a command available, handle it
  if (serialManager.isCommandAvailable()) {
    String command = serialManager.getNextCommand();
    
    // Handle the command (most commands are handled by serialManager)
    if (command.startsWith(PING_COMMAND)) {
      // Create a ping message
      StaticJsonDocument<200> pingDoc;
      // Send a ping to the remote device
      loraCommunication.sendMessage(MSG_TYPE_PING, pingDoc);
      
      // Update display
      displayManager.showStatus("Ping Sent");
      serialManager.log("Ping sent to remote device");
    }
    else if (command.startsWith(STATUS_COMMAND)) {
      // Send current status to serial
      sendStatusToSerial();
    }
  }
}

void sendStatusToSerial() {
  // Create status document
  StaticJsonDocument<512> statusDoc;
  
  // Add system status
  statusDoc["uptime"] = (millis() - uptimeStart) / 1000;
  statusDoc["packets_received"] = totalPacketsReceived;
  statusDoc["errors"] = errorPackets;
  
  // Add remote device status
  JsonObject remote = statusDoc.createNestedObject("remote_device");
  remote["battery"] = remoteBatteryVoltage;
  remote["battery_percent"] = remoteBatteryPercentage;
  remote["charging"] = remoteIsCharging;
  remote["last_seen"] = (millis() - remoteLastSeen) / 1000;
  
  // Add signal metrics
  JsonObject signal = statusDoc.createNestedObject("signal");
  signal["rssi"] = lastRssi;
  signal["snr"] = lastSnr;
  signal["packet_loss"] = packetLossRate;
  signal["latency"] = avgLatency;
  
  // Send to serial
  serialManager.sendMetrics(statusDoc);
}
