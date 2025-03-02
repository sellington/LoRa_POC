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
#define DISPLAY_UPDATE_INTERVAL 5000  // Update interval in ms

// Screen pages
enum ScreenPage {
    PAGE_STATUS,
    PAGE_METRICS,
    PAGE_DEBUG,
    PAGE_COUNT  // Total number of pages
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
    
    // Update signal metrics on the display
    void updateSignalMetrics(int rssi, float snr, float latency);
    
    // Update power metrics on the display
    void updatePowerMetrics(float batteryVoltage, uint8_t batteryPercentage, bool isCharging);
    
    // Display debug information
    void showDebugInfo(const char* message);
    
    // Turn the display on or off
    void setPower(bool on);
    
    // Set display brightness (0-255)
    void setBrightness(uint8_t brightness);
    
    // Get the current page
    ScreenPage getCurrentPage() const;
    
    // Get a reference to the display object
    Adafruit_SSD1306* getDisplay();
    
private:
    Adafruit_SSD1306 display;
    ScreenPage currentPage;
    unsigned long lastUpdateTime;
    bool displayOn;
    
    // Metrics data
    int rssi;
    float snr;
    float latency;
    float batteryVoltage;
    uint8_t batteryPercentage;
    bool isCharging;
    char statusMessage[32];
    char debugMessage[64];
    
    // Helper methods to draw specific pages
    void drawStatusPage();
    void drawMetricsPage();
    void drawDebugPage();
    
    // Draw battery icon
    void drawBatteryIcon(int x, int y, uint8_t percentage, bool charging);
    
    // Draw signal strength icon
    void drawSignalIcon(int x, int y, int rssi);
};

extern DisplayManager displayManager;

#endif // DISPLAY_MANAGER_H
