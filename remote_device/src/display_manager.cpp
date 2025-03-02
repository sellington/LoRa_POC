#include "display_manager.h"

// Global instance
DisplayManager displayManager;

DisplayManager::DisplayManager() : 
    display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN),
    currentPage(PAGE_STATUS),
    lastUpdateTime(0),
    displayOn(true),
    rssi(-120),
    snr(0.0),
    latency(0.0),
    batteryVoltage(0.0),
    batteryPercentage(0),
    isCharging(false) {
    
    // Initialize status and debug messages
    strcpy(statusMessage, "Initializing...");
    strcpy(debugMessage, "");
}

bool DisplayManager::begin() {
    // Set up I2C pins
    Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN);
    
    // Initialize the OLED display
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("SSD1306 allocation failed"));
        return false;
    }
    
    // Initial display setup
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(F("LoRa Remote Device"));
    display.println(F("Initializing..."));
    display.display();
    
    Serial.println(F("Display manager initialized"));
    return true;
}

void DisplayManager::clear() {
    display.clearDisplay();
    display.display();
}

void DisplayManager::update() {
    // Only update if enough time has passed
    if (millis() - lastUpdateTime < DISPLAY_UPDATE_INTERVAL) {
        return;
    }
    
    // Only update if display is on
    if (!displayOn) {
        return;
    }
    
    // Update the display based on current page
    display.clearDisplay();
    
    switch (currentPage) {
        case PAGE_STATUS:
            drawStatusPage();
            break;
        case PAGE_METRICS:
            drawMetricsPage();
            break;
        case PAGE_DEBUG:
            drawDebugPage();
            break;
        default:
            // Invalid page, show status
            currentPage = PAGE_STATUS;
            drawStatusPage();
            break;
    }
    
    display.display();
    lastUpdateTime = millis();
}

void DisplayManager::showPage(ScreenPage page) {
    if (page < PAGE_COUNT) {
        currentPage = page;
        update();  // Force immediate update
    }
}

void DisplayManager::nextPage() {
    currentPage = (ScreenPage)((currentPage + 1) % PAGE_COUNT);
    update();  // Force immediate update
}

void DisplayManager::showStatus(const char* status) {
    strncpy(statusMessage, status, sizeof(statusMessage) - 1);
    statusMessage[sizeof(statusMessage) - 1] = '\0';  // Ensure null termination
    
    // If on status page, update immediately
    if (currentPage == PAGE_STATUS) {
        update();
    }
}

void DisplayManager::updateSignalMetrics(int rssi, float snr, float latency) {
    this->rssi = rssi;
    this->snr = snr;
    this->latency = latency;
    
    // If on metrics page, update immediately
    if (currentPage == PAGE_METRICS) {
        update();
    }
}

void DisplayManager::updatePowerMetrics(float batteryVoltage, uint8_t batteryPercentage, bool isCharging) {
    this->batteryVoltage = batteryVoltage;
    this->batteryPercentage = batteryPercentage;
    this->isCharging = isCharging;
    
    // If on metrics page, update immediately
    if (currentPage == PAGE_METRICS) {
        update();
    }
}

void DisplayManager::showDebugInfo(const char* message) {
    strncpy(debugMessage, message, sizeof(debugMessage) - 1);
    debugMessage[sizeof(debugMessage) - 1] = '\0';  // Ensure null termination
    
    // If on debug page, update immediately
    if (currentPage == PAGE_DEBUG) {
        update();
    }
}

void DisplayManager::setPower(bool on) {
    displayOn = on;
    
    if (on) {
        // Power on sequence
        display.ssd1306_command(SSD1306_DISPLAYON);
        update();  // Force update
    } else {
        // Power off sequence
        display.ssd1306_command(SSD1306_DISPLAYOFF);
    }
}

void DisplayManager::setBrightness(uint8_t brightness) {
    // Note: SSD1306 doesn't really support brightness adjustment
    // But we could potentially implement PWM control if hardware supports it
    // For now, just an on/off toggle
    setPower(brightness > 0);
}

ScreenPage DisplayManager::getCurrentPage() const {
    return currentPage;
}

Adafruit_SSD1306* DisplayManager::getDisplay() {
    return &display;
}

void DisplayManager::drawStatusPage() {
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(F("LoRa Remote Device"));
    display.drawLine(0, 8, SCREEN_WIDTH - 1, 8, SSD1306_WHITE);
    
    // Status message
    display.setCursor(0, 12);
    display.println(statusMessage);
    
    // Draw battery
    drawBatteryIcon(110, 0, batteryPercentage, isCharging);
    
    // Draw signal
    drawSignalIcon(90, 0, rssi);
    
    // Power info
    display.setCursor(0, 24);
    display.print(F("Battery: "));
    display.print(batteryVoltage, 1);
    display.print(F("V ("));
    display.print(batteryPercentage);
    display.println(F("%)"));
    
    // Signal info
    display.setCursor(0, 36);
    display.print(F("Signal: "));
    display.print(rssi);
    display.print(F("dBm SNR:"));
    display.println(snr, 1);
    
    // Connection info
    display.setCursor(0, 48);
    display.print(F("Latency: "));
    display.print(latency, 0);
    display.println(F("ms"));
    
    // Page indicator
    display.setCursor(0, 56);
    display.print(F("Page 1/"));
    display.print(PAGE_COUNT);
}

void DisplayManager::drawMetricsPage() {
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("Metrics "));
    display.drawLine(0, 8, SCREEN_WIDTH - 1, 8, SSD1306_WHITE);
    
    // Draw battery
    drawBatteryIcon(110, 0, batteryPercentage, isCharging);
    
    // Signal metrics
    display.setCursor(0, 10);
    display.println(F("Signal Quality:"));
    display.print(F("  RSSI: "));
    display.print(rssi);
    display.println(F(" dBm"));
    display.print(F("  SNR: "));
    display.print(snr, 1);
    display.println(F(" dB"));
    
    // Power metrics
    display.setCursor(0, 34);
    display.println(F("Power:"));
    display.print(F("  Batt: "));
    display.print(batteryVoltage, 2);
    display.print(F("V "));
    display.print(batteryPercentage);
    display.println(F("%"));
    display.print(F("  Charging: "));
    display.println(isCharging ? F("Yes") : F("No"));
    
    // Page indicator
    display.setCursor(0, 56);
    display.print(F("Page 2/"));
    display.print(PAGE_COUNT);
}

void DisplayManager::drawDebugPage() {
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("Debug "));
    display.drawLine(0, 8, SCREEN_WIDTH - 1, 8, SSD1306_WHITE);
    
    // Debug message - wrap at screen width
    display.setCursor(0, 10);
    int len = strlen(debugMessage);
    int lineWidth = 21;  // Characters per line
    
    for (int i = 0; i < len && i < 120; i += lineWidth) {  // Limit to 120 chars total
        char line[22];  // lineWidth + 1 for null
        int copyLen = min(lineWidth, len - i);
        strncpy(line, &debugMessage[i], copyLen);
        line[copyLen] = '\0';  // Null terminate
        display.println(line);
    }
    
    // Page indicator
    display.setCursor(0, 56);
    display.print(F("Page 3/"));
    display.print(PAGE_COUNT);
}

void DisplayManager::drawBatteryIcon(int x, int y, uint8_t percentage, bool charging) {
    // Draw battery outline
    display.drawRect(x, y, 18, 8, SSD1306_WHITE);
    display.drawRect(x + 18, y + 2, 2, 4, SSD1306_WHITE);
    
    // Calculate filled width
    uint8_t fillWidth = map(percentage, 0, 100, 0, 16);
    
    // Draw fill level
    display.fillRect(x + 1, y + 1, fillWidth, 6, SSD1306_WHITE);
    
    // Draw charging indicator if charging
    if (charging) {
        display.drawLine(x + 4, y + 1, x + 8, y + 6, SSD1306_WHITE);
        display.drawLine(x + 8, y + 1, x + 12, y + 6, SSD1306_WHITE);
    }
}

void DisplayManager::drawSignalIcon(int x, int y, int rssi) {
    // Convert RSSI to bars (0-4)
    int bars = 0;
    
    if (rssi >= -60) bars = 4;
    else if (rssi >= -70) bars = 3;
    else if (rssi >= -80) bars = 2;
    else if (rssi >= -90) bars = 1;
    
    // Draw signal bars
    for (int i = 0; i < 4; i++) {
        int barHeight = 2 + i;
        if (i < bars) {
            display.fillRect(x + i * 3, y + 8 - barHeight, 2, barHeight, SSD1306_WHITE);
        } else {
            display.drawRect(x + i * 3, y + 8 - barHeight, 2, barHeight, SSD1306_WHITE);
        }
    }
}
