#ifndef METRICS_H
#define METRICS_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Maximum number of packets to track for statistics
#define MAX_PACKET_HISTORY  20

// Metrics update interval
#define METRICS_UPDATE_INTERVAL  5000  // ms

// Struct to hold a packet record for statistics
struct PacketRecord {
    uint32_t id;
    uint32_t timestamp;
    bool success;
    int rssi;
    float snr;
    uint32_t retries;
    uint32_t latency;  // ms
};

class Metrics {
public:
    Metrics();
    
    // Initialize the metrics system
    void begin();
    
    // Update metrics (call periodically)
    void update();
    
    // Record a transmitted packet
    void recordTransmission(uint32_t packetId, bool success, int rssi = 0, float snr = 0.0, uint32_t retries = 0, uint32_t latency = 0);
    
    // Get packet success rate (0.0-1.0)
    float getPacketSuccessRate();
    
    // Get average RSSI
    int getAverageRSSI();
    
    // Get average SNR
    float getAverageSNR();
    
    // Get average retry count
    float getAverageRetries();
    
    // Get average latency
    uint32_t getAverageLatency();
    
    // Get current system metrics as JSON
    void getSystemMetrics(JsonDocument& doc);
    
    // Get signal metrics as JSON
    void getSignalMetrics(JsonDocument& doc);
    
    // Get performance metrics as JSON
    void getPerformanceMetrics(JsonDocument& doc);
    
    // Get all metrics as JSON
    void getAllMetrics(JsonDocument& doc);
    
    // Reset all metrics
    void reset();
    
private:
    // Packet history for statistics
    PacketRecord packetHistory[MAX_PACKET_HISTORY];
    uint8_t packetHistoryIndex;
    uint8_t packetHistoryCount;
    
    // Cumulative statistics
    uint32_t totalPackets;
    uint32_t successfulPackets;
    
    // System metrics
    uint32_t uptime;
    uint32_t freeMemory;
    float cpuTemperature;
    
    // Last update time
    unsigned long lastUpdateTime;
    
    // Update system metrics
    void updateSystemMetrics();
    
    // Helper method to get CPU temperature
    float getCpuTemperature();
    
    // Helper method to get free memory
    uint32_t getFreeMemory();
};

extern Metrics metrics;

#endif // METRICS_H
