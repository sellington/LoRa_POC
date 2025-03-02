#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// OLED display dimensions
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// OLED display pins for Heltec WiFi LoRa 32 V3
#define OLED_SCL_PIN 18  // I2C SCL pin for Heltec WiFi LoRa 32 V3
#define OLED_SDA_PIN 17  // I2C SDA pin for Heltec WiFi LoRa 32 V3
#define OLED_RESET_PIN -1  // No reset pin
#define OLED_ADDR 0x3C    // I2C address (typically 0x3C or 0x3D)

// Display update frequency
#define DISPLAY_UPDATE_INTERVAL 1000  // Update interval in ms

// Screen pages
enum ScreenPage {
    PAGE_STATUS,          // Current status, last message
    PAGE_REMOTE_STATUS,   // Remote device status
    PAGE_SIGNAL_METRICS,  // Signal quality metrics
    PAGE_SYSTEM_METRICS,  // System metrics
    PAGE_COUNT            // Total number of pages
};

class DisplayManager {
public:
    DisplayManager();
    
    // Initialize the display
    bool begin();
    
    // Clear the display
    void clear();
    
    // Update the display with fresh data
    void update();
    
    // Show a specific screen page
    void showPage(ScreenPage page);
    
    // Cycle to the next page
    void nextPage();
    
    // Display a status message
    void showStatus(const char* status);
    
    // Update remote device status
    void updateRemoteStatus(float batteryVoltage, uint8_t batteryPercentage, bool isCharging, unsigned long lastSeenSeconds);
    
    // Update signal metrics
    void updateSignalMetrics(int rssi, float snr, float packetLoss, float avgLatency);
    
    // Update system metrics
    void updateSystemMetrics(unsigned long uptime, unsigned long packets, unsigned long errors);
    
    // Turn the display on or off
    void setPower(bool on);
    
    // Get the current page
    ScreenPage getCurrentPage() const;
    
    // Get a reference to the display object
    Adafruit_SSD1306* getDisplay();
    
private:
    Adafruit_SSD1306 display;
    ScreenPage currentPage;
    unsigned long lastUpdateTime;
    bool displayOn;
    
    // Status data
    char statusMessage[64];
    unsigned long statusUpdateTime;
    
    // Remote device data
    float remoteBatteryVoltage;
    uint8_t remoteBatteryPercentage;
    bool remoteIsCharging;
    unsigned long remoteLastSeen;
    
    // Signal metrics
    int rssi;
    float snr;
    float packetLoss;
    float avgLatency;
    
    // System metrics
    unsigned long uptime;
    unsigned long totalPackets;
    unsigned long errorPackets;
    
    // Helper methods to draw specific pages
    void drawStatusPage();
    void drawRemoteStatusPage();
    void drawSignalMetricsPage();
    void drawSystemMetricsPage();
    
    // Draw battery icon
    void drawBatteryIcon(int x, int y, uint8_t percentage, bool charging);
    
    // Draw signal strength icon
    void drawSignalIcon(int x, int y, int rssi);
    
    // Format time string (HH:MM:SS)
    void formatTime(unsigned long seconds, char* buffer, size_t bufferSize);
};

extern DisplayManager displayManager;

#endif // DISPLAY_MANAGER_H
