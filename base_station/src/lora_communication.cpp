#include "lora_communication.h"
#include <time.h>

// Initialize message ID counter
uint32_t nextMessageId = 1;

// Global instance
LoRaCommunication loraCommunication;

LoRaCommunication::LoRaCommunication() : 
    // For Heltec WiFi LoRa 32 V3 with original schematic pins
    // The pin discovery test confirmed these are the correct pins
    lora(new Module(LORA_CS_PIN, LORA_DIO1_PIN, LORA_RST_PIN, LORA_BUSY_PIN, SPI, SPISettings(2000000, MSBFIRST, SPI_MODE0))),
    isInitialized(false) {
    // Initialize SPI explicitly for ESP32-S3 with the working pin configuration
    SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_CS_PIN); // SCK, MISO, MOSI, SS
}

bool LoRaCommunication::begin() {
    // Initialize the LoRa module
    Serial.print(F("Initializing LoRa module with correct pin configuration... "));
    
    // Make sure SPI is initialized with the correct pins
    SPI.begin(LORA_SCK_PIN, LORA_MISO_PIN, LORA_MOSI_PIN, LORA_CS_PIN); // SCK, MISO, MOSI, SS
    
    // Set pin modes explicitly - using schematic pin assignments
    pinMode(LORA_CS_PIN, OUTPUT);
    pinMode(LORA_RST_PIN, OUTPUT);
    pinMode(LORA_BUSY_PIN, INPUT); // BUSY pin (confirmed working)
    pinMode(LORA_DIO1_PIN, INPUT); // DIO1 pin (confirmed working)
    
    digitalWrite(LORA_CS_PIN, HIGH); // Deselect chip
    
    // Reset the module before initializing (full reset sequence for SX1262)
    digitalWrite(LORA_RST_PIN, LOW);
    delay(10);
    digitalWrite(LORA_RST_PIN, HIGH);
    delay(100);
    
    int state = lora.begin();
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print(F("Failed! Error code: "));
        Serial.println(state);
        return false;
    }
    
    // Configure the module
    lora.setFrequency(LORA_FREQUENCY);
    lora.setBandwidth(LORA_BANDWIDTH);
    lora.setSpreadingFactor(LORA_SPREADING_FACTOR);
    lora.setCodingRate(LORA_CODING_RATE);
    lora.setSyncWord(LORA_SYNC_WORD);
    lora.setOutputPower(LORA_POWER);
    lora.setPreambleLength(LORA_PREAMBLE_LENGTH);
    
    if (LORA_ENABLE_CRC) {
        lora.setCRC(true);
    }
    
    Serial.println(F("LoRa module initialized!"));
    isInitialized = true;
    return true;
}

bool LoRaCommunication::sendMessage(const char* type, JsonDocument& payload, int* rssi, float* snr) {
    if (!isInitialized) {
        Serial.println(F("LoRa module not initialized"));
        return false;
    }
    
    // Create the message document
    StaticJsonDocument<MAX_PACKET_SIZE> doc;
    
    // Build the message
    buildMessage(doc, type, payload);
    
    // Serialize the JSON document to a string
    char buffer[MAX_PACKET_SIZE];
    size_t bytes = serializeJson(doc, buffer, MAX_PACKET_SIZE);
    
    // Send the message with retries
    for (int attempt = 0; attempt < MAX_RETRIES; attempt++) {
        // Print debug info
        Serial.print(F("Sending message (attempt "));
        Serial.print(attempt + 1);
        Serial.print(F("): "));
        Serial.println(buffer);
        
        // Transmit the packet
        int state = lora.transmit(buffer, bytes);
        if (state != RADIOLIB_ERR_NONE) {
            Serial.print(F("Transmission failed! Error code: "));
            Serial.println(state);
            delay(100 * (attempt + 1));  // Exponential backoff
            continue;
        }
        
        // For the base station, no need to wait for acknowledgment unless it's a ping
        if (strcmp(type, MSG_TYPE_PING) == 0) {
            // TODO: Implement ping response handling if needed
            // For now, just consider it successful
        }
        
        return true;
    }
    
    Serial.println(F("Failed to send message after max retries"));
    return false;
}

bool LoRaCommunication::receiveMessage(JsonDocument& doc, int* rssi, float* snr) {
    if (!isInitialized) {
        Serial.println(F("LoRa module not initialized"));
        return false;
    }
    
    // Check if a packet is available
    if (!lora.available()) {
        return false;
    }
    
    // Receive the packet
    String message = "";
    int state = lora.readData(message);
    
    // Get RSSI and SNR
    if (rssi != nullptr) {
        *rssi = lora.getRSSI();
    }
    if (snr != nullptr) {
        *snr = lora.getSNR();
    }
    
    // Check for errors
    if (state != RADIOLIB_ERR_NONE) {
        Serial.print(F("Reception failed! Error code: "));
        Serial.println(state);
        return false;
    }
    
    // Print debug info
    Serial.print(F("Received: "));
    Serial.println(message);
    
    // Parse the JSON document
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print(F("JSON parsing failed: "));
        Serial.println(error.c_str());
        return false;
    }
    
    // Send acknowledgment for most message types
    if (doc.containsKey("id") && 
        doc.containsKey("type") && 
        strcmp(doc["type"], MSG_TYPE_PONG) != 0) {  // Don't ack an ack
        sendAcknowledgment(doc["id"]);
    }
    
    return true;
}

void LoRaCommunication::checkForIncomingMessages(void (*messageHandler)(const char* type, JsonDocument& doc, int rssi, float snr)) {
    if (!isInitialized || messageHandler == nullptr) {
        return;
    }
    
    // Check if a packet is available
    if (!lora.available()) {
        return;
    }
    
    // Receive the message
    StaticJsonDocument<MAX_PACKET_SIZE> doc;
    int rssi = 0;
    float snr = 0.0;
    
    if (receiveMessage(doc, &rssi, &snr)) {
        // Extract the message type
        if (doc.containsKey("type")) {
            const char* type = doc["type"];
            
            // Call the message handler
            messageHandler(type, doc, rssi, snr);
        }
    }
}

bool LoRaCommunication::sendAcknowledgment(uint32_t messageId) {
    // Create a pong response with the same ID
    StaticJsonDocument<200> response;
    response["type"] = MSG_TYPE_PONG;
    response["id"] = messageId;
    response["timestamp"] = millis() / 1000;
    
    // Send the acknowledgment
    return sendMessage(MSG_TYPE_PONG, response);
}

uint32_t LoRaCommunication::getNextMessageId() {
    return nextMessageId++;
}

void LoRaCommunication::sleep() {
    if (isInitialized) {
        lora.sleep();
        Serial.println(F("LoRa module in sleep mode"));
    }
}

void LoRaCommunication::wakeup() {
    if (isInitialized) {
        lora.standby();
        Serial.println(F("LoRa module woken up"));
    }
}

SX1262* LoRaCommunication::getModule() {
    return &lora;
}

void LoRaCommunication::buildMessage(JsonDocument& doc, const char* type, const JsonDocument& payload) {
    // Set message type
    doc["type"] = type;
    
    // Set message ID
    doc["id"] = getNextMessageId();
    
    // Set timestamp (seconds since boot)
    doc["timestamp"] = millis() / 1000;
    
    // Copy metrics if present
    if (payload.containsKey("metrics")) {
        doc["metrics"] = payload["metrics"];
    }
    
    // Copy payload if present
    if (payload.containsKey("payload")) {
        doc["payload"] = payload["payload"];
    }
}
