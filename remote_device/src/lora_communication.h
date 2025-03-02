#ifndef LORA_COMMUNICATION_H
#define LORA_COMMUNICATION_H

#include <Arduino.h>
#include <RadioLib.h>
#include <ArduinoJson.h>

// LoRa module pins for Heltec WiFi LoRa 32 V3 (ESP32-S3)
// Using pins confirmed by pin discovery testing
// NOTE: Our systematic testing revealed the original pins ARE working correctly
#define LORA_CS_PIN    8   // SPI CS (NSS)
#define LORA_SCK_PIN   9   // SPI SCK
#define LORA_MISO_PIN  11  // SPI MISO
#define LORA_MOSI_PIN  10  // SPI MOSI
#define LORA_RST_PIN   12  // Reset
#define LORA_BUSY_PIN  13  // BUSY - original pin confirmed working
#define LORA_DIO1_PIN  14  // DIO1 - original pin confirmed working

// LoRa parameters - using the minimal working configuration from our tests
#define LORA_FREQUENCY       915.0   // MHz (US frequency, use 868.0 for EU)
#define LORA_BANDWIDTH       500.0   // kHz - maximum bandwidth for reliable communications
#define LORA_SPREADING_FACTOR 6      // Minimum spreading factor (fastest data rate)
#define LORA_CODING_RATE     5       // 4/5 coding rate
#define LORA_SYNC_WORD       0x34    // Sync word that worked in our tests
#define LORA_POWER           2       // dBm - low power for testing
#define LORA_PREAMBLE_LENGTH 8       // symbols - minimal preamble length
#define LORA_ENABLE_CRC      true    // Enable CRC checking

// Message types
#define MSG_TYPE_PING    "ping"
#define MSG_TYPE_PONG    "pong"
#define MSG_TYPE_DATA    "data"
#define MSG_TYPE_STATUS  "status"

// Communication parameters
#define MAX_PACKET_SIZE    256   // Maximum size of packet to send
#define MAX_RETRIES        3     // Maximum number of transmission retries
#define ACK_TIMEOUT        1000  // Timeout for acknowledgment in ms

// Message IDs
extern uint32_t nextMessageId;

class LoRaCommunication {
public:
    LoRaCommunication();
    
    // Initialize the LoRa module
    bool begin();
    
    // Send JSON message and wait for acknowledgment
    bool sendMessage(const char* type, JsonDocument& payload, int* rssi = nullptr, float* snr = nullptr);
    
    // Check if a message is available and receive it
    bool receiveMessage(JsonDocument& doc, int* rssi = nullptr, float* snr = nullptr);
    
    // Ping the base station and measure round-trip time
    int ping(int* rssi = nullptr, float* snr = nullptr);
    
    // Send a data message with metrics
    bool sendMetrics(JsonDocument& metrics);
    
    // Send a status update
    bool sendStatus(const char* status, JsonDocument& metrics);
    
    // Get the next message ID
    uint32_t getNextMessageId();
    
    // Put the LoRa module to sleep
    void sleep();
    
    // Wake up the LoRa module
    void wakeup();
    
    // Return the LoRa module instance for direct access if needed
    SX1262* getModule();
    
private:
    SX1262 lora;
    bool isInitialized;
    
    // Helper method to build a standard message
    void buildMessage(JsonDocument& doc, const char* type, const JsonDocument& payload);
    
    // Try to receive an acknowledgment
    bool waitForAck(uint32_t messageId, int timeout, int* rssi = nullptr, float* snr = nullptr);
};

extern LoRaCommunication loraCommunication;

#endif // LORA_COMMUNICATION_H
