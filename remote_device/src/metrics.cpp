#include "metrics.h"
#include <esp_system.h>
// ESP32-specific includes
#ifdef ESP_PLATFORM
// No specific temperature sensor header needed
#endif

// Global instance
Metrics metrics;

Metrics::Metrics() : 
    packetHistoryIndex(0),
    packetHistoryCount(0),
    totalPackets(0),
    successfulPackets(0),
    uptime(0),
    freeMemory(0),
    cpuTemperature(0.0),
    lastUpdateTime(0) {
}

void Metrics::begin() {
    // Initialize metrics
    reset();
    updateSystemMetrics();
    
    Serial.println(F("Metrics system initialized"));
}

void Metrics::update() {
    // Only update if enough time has passed
    if (millis() - lastUpdateTime < METRICS_UPDATE_INTERVAL) {
        return;
    }
    
    // Update system metrics
    updateSystemMetrics();
    
    lastUpdateTime = millis();
}

void Metrics::recordTransmission(uint32_t packetId, bool success, int rssi, float snr, uint32_t retries, uint32_t latency) {
    // Increment total packets counter
    totalPackets++;
    
    // Increment successful packets counter if successful
    if (success) {
        successfulPackets++;
    }
    
    // Add to packet history
    PacketRecord& record = packetHistory[packetHistoryIndex];
    record.id = packetId;
    record.timestamp = millis() / 1000;  // seconds since boot
    record.success = success;
    record.rssi = rssi;
    record.snr = snr;
    record.retries = retries;
    record.latency = latency;
    
    // Update index and count
    packetHistoryIndex = (packetHistoryIndex + 1) % MAX_PACKET_HISTORY;
    if (packetHistoryCount < MAX_PACKET_HISTORY) {
        packetHistoryCount++;
    }
    
    // Debug print
    Serial.print(F("Packet recorded - ID: "));
    Serial.print(packetId);
    Serial.print(F(", Success: "));
    Serial.println(success ? F("Yes") : F("No"));
}

float Metrics::getPacketSuccessRate() {
    if (totalPackets == 0) {
        return 0.0;
    }
    return (float)successfulPackets / totalPackets;
}

int Metrics::getAverageRSSI() {
    if (packetHistoryCount == 0) {
        return -120;  // Default weak signal
    }
    
    int total = 0;
    int count = 0;
    
    for (uint8_t i = 0; i < packetHistoryCount; i++) {
        if (packetHistory[i].success) {
            total += packetHistory[i].rssi;
            count++;
        }
    }
    
    if (count == 0) {
        return -120;
    }
    
    return total / count;
}

float Metrics::getAverageSNR() {
    if (packetHistoryCount == 0) {
        return 0.0;
    }
    
    float total = 0;
    int count = 0;
    
    for (uint8_t i = 0; i < packetHistoryCount; i++) {
        if (packetHistory[i].success) {
            total += packetHistory[i].snr;
            count++;
        }
    }
    
    if (count == 0) {
        return 0.0;
    }
    
    return total / count;
}

float Metrics::getAverageRetries() {
    if (packetHistoryCount == 0) {
        return 0.0;
    }
    
    uint32_t total = 0;
    
    for (uint8_t i = 0; i < packetHistoryCount; i++) {
        total += packetHistory[i].retries;
    }
    
    return (float)total / packetHistoryCount;
}

uint32_t Metrics::getAverageLatency() {
    if (packetHistoryCount == 0) {
        return 0;
    }
    
    uint32_t total = 0;
    int count = 0;
    
    for (uint8_t i = 0; i < packetHistoryCount; i++) {
        if (packetHistory[i].success && packetHistory[i].latency > 0) {
            total += packetHistory[i].latency;
            count++;
        }
    }
    
    if (count == 0) {
        return 0;
    }
    
    return total / count;
}

void Metrics::getSystemMetrics(JsonDocument& doc) {
    // Update system metrics first
    updateSystemMetrics();
    
    // Add system metrics to JSON document
    doc["uptime"] = uptime;
    doc["free_memory"] = freeMemory;
    doc["temperature"] = cpuTemperature;
}

void Metrics::getSignalMetrics(JsonDocument& doc) {
    // Add signal metrics to JSON document
    doc["rssi"] = getAverageRSSI();
    doc["snr"] = getAverageSNR();
}

void Metrics::getPerformanceMetrics(JsonDocument& doc) {
    // Add performance metrics to JSON document
    doc["success_rate"] = getPacketSuccessRate();
    doc["avg_retries"] = getAverageRetries();
    doc["avg_latency"] = getAverageLatency();
    doc["total_packets"] = totalPackets;
}

void Metrics::getAllMetrics(JsonDocument& doc) {
    // Add all metrics to JSON document
    getSystemMetrics(doc);
    getSignalMetrics(doc);
    getPerformanceMetrics(doc);
}

void Metrics::reset() {
    // Reset packet history
    packetHistoryIndex = 0;
    packetHistoryCount = 0;
    
    // Reset counters
    totalPackets = 0;
    successfulPackets = 0;
    
    // Initialize system metrics
    updateSystemMetrics();
    
    Serial.println(F("Metrics reset"));
}

void Metrics::updateSystemMetrics() {
    // Update system metrics
    uptime = millis() / 1000;  // seconds since boot
    freeMemory = getFreeMemory();
    cpuTemperature = getCpuTemperature();
}

float Metrics::getCpuTemperature() {
    #ifdef ESP_PLATFORM
        // For ESP32/ESP32-S3, use a simulated temperature value
        // ESP32-S3 doesn't have the hall sensor that regular ESP32 has
        
        // Simple simulation based on millis
        float temp = 25.0 + ((millis() / 1000) % 10) / 10.0; // Range of 25.0-25.9 degrees
        return temp;
    #else
        // For other platforms or simulators
        return 25.0;  // Return a default value
    #endif
}

uint32_t Metrics::getFreeMemory() {
    // Get free heap memory on ESP32
    #ifdef ESP_PLATFORM
        return esp_get_free_heap_size();
    #else
        // For other platforms or simulators
        return 50000;  // Return a default value
    #endif
}
