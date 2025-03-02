#include "display_manager.h"

// Global instance
DisplayManager displayManager;

DisplayManager::DisplayManager() :
    display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET_PIN),
    currentPage(PAGE_STATUS),
    lastUpdateTime(0),
    displayOn(true),
    statusUpdateTime(0) {
    
    // Initialize status message
    strcpy(statusMessage, "Initializing...");
    
    // Initialize metrics
    remoteBatteryVoltage = 0.0;
    remoteBatteryPercentage = 0;
    remoteIsCharging = false;
    remoteLastSeen = 0;
    
    rssi = -120;
    snr = 0.0;
    packetLoss = 0.0;
    avgLatency = 0.0;
    
    uptime = 0;
    totalPackets = 0;
    errorPackets = 0;
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
    display.println(F("LoRa Base Station"));
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
        case PAGE_REMOTE_STATUS:
            drawRemoteStatusPage();
            break;
        case PAGE_SIGNAL_METRICS:
            drawSignalMetricsPage();
            break;
        case PAGE_SYSTEM_METRICS:
            drawSystemMetricsPage();
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
    statusUpdateTime = millis();
    
    // If on status page, update immediately
    if (currentPage == PAGE_STATUS) {
        update();
    }
}

void DisplayManager::updateRemoteStatus(float batteryVoltage, uint8_t batteryPercentage, bool isCharging, unsigned long lastSeenSeconds) {
    remoteBatteryVoltage = batteryVoltage;
    remoteBatteryPercentage = batteryPercentage;
    remoteIsCharging = isCharging;
    remoteLastSeen = lastSeenSeconds;
    
    // If on remote status page, update immediately
    if (currentPage == PAGE_REMOTE_STATUS) {
        update();
    }
}

void DisplayManager::updateSignalMetrics(int rssi, float snr, float packetLoss, float avgLatency) {
    this->rssi = rssi;
    this->snr = snr;
    this->packetLoss = packetLoss;
    this->avgLatency = avgLatency;
    
    // If on signal metrics page, update immediately
    if (currentPage == PAGE_SIGNAL_METRICS) {
        update();
    }
}

void DisplayManager::updateSystemMetrics(unsigned long uptime, unsigned long packets, unsigned long errors) {
    this->uptime = uptime;
    this->totalPackets = packets;
    this->errorPackets = errors;
    
    // If on system metrics page, update immediately
    if (currentPage == PAGE_SYSTEM_METRICS) {
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
    display.println(F("LoRa Base Station"));
    display.drawLine(0, 8, SCREEN_WIDTH - 1, 8, SSD1306_WHITE);
    
    // Status message
    display.setCursor(0, 12);
    display.println(statusMessage);
    
    // Time since status update
    if (statusUpdateTime > 0) {
        unsigned long timeSince = (millis() - statusUpdateTime) / 1000;
        display.setCursor(0, 24);
        display.print(F("Updated: "));
        
        char timeStr[12];
        formatTime(timeSince, timeStr, sizeof(timeStr));
        display.print(timeStr);
        display.println(F(" ago"));
    }
    
    // Signal strength
    drawSignalIcon(110, 0, rssi);
    
    // Remote device info
    display.setCursor(0, 36);
    display.print(F("Remote: "));
    if (remoteLastSeen > 0) {
        char timeStr[12];
        formatTime(remoteLastSeen, timeStr, sizeof(timeStr));
        display.print(timeStr);
        display.println(F(" ago"));
        
        display.setCursor(0, 46);
        display.print(F("Batt: "));
        display.print(remoteBatteryVoltage, 1);
        display.print(F("V ("));
        display.print(remoteBatteryPercentage);
        display.print(F("%)"));
    } else {
        display.println(F("No data"));
    }
    
    // Page indicator
    display.setCursor(0, 56);
    display.print(F("Page 1/"));
    display.print(PAGE_COUNT);
}

void DisplayManager::drawRemoteStatusPage() {
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("Remote Device"));
    display.drawLine(0, 8, SCREEN_WIDTH - 1, 8, SSD1306_WHITE);
    
    // Remote device status
    if (remoteLastSeen > 0) {
        // Last seen
        display.setCursor(0, 10);
        display.print(F("Last seen: "));
        char timeStr[12];
        formatTime(remoteLastSeen, timeStr, sizeof(timeStr));
        display.print(timeStr);
        display.println(F(" ago"));
        
        // Battery info
        display.setCursor(0, 20);
        display.print(F("Battery: "));
        display.print(remoteBatteryVoltage, 1);
        display.print(F("V ("));
        display.print(remoteBatteryPercentage);
        display.println(F("%)"));
        
        // Charging status
        display.setCursor(0, 30);
        display.print(F("Charging: "));
        display.println(remoteIsCharging ? F("Yes") : F("No"));
        
        // Draw battery icon
        drawBatteryIcon(100, 20, remoteBatteryPercentage, remoteIsCharging);
    } else {
        display.setCursor(0, 20);
        display.println(F("No data from remote device"));
    }
    
    // Page indicator
    display.setCursor(0, 56);
    display.print(F("Page 2/"));
    display.print(PAGE_COUNT);
}

void DisplayManager::drawSignalMetricsPage() {
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("Signal Metrics"));
    display.drawLine(0, 8, SCREEN_WIDTH - 1, 8, SSD1306_WHITE);
    
    // Draw signal strength icon
    drawSignalIcon(110, 0, rssi);
    
    // Signal metrics
    display.setCursor(0, 10);
    display.print(F("RSSI: "));
    display.print(rssi);
    display.println(F(" dBm"));
    
    display.setCursor(0, 20);
    display.print(F("SNR: "));
    display.print(snr, 1);
    display.println(F(" dB"));
    
    display.setCursor(0, 30);
    display.print(F("Packet Loss: "));
    display.print(packetLoss * 100, 1);
    display.println(F("%"));
    
    display.setCursor(0, 40);
    display.print(F("Latency: "));
    display.print(avgLatency, 0);
    display.println(F(" ms"));
    
    // Page indicator
    display.setCursor(0, 56);
    display.print(F("Page 3/"));
    display.print(PAGE_COUNT);
}

void DisplayManager::drawSystemMetricsPage() {
    // Header
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("System Metrics"));
    display.drawLine(0, 8, SCREEN_WIDTH - 1, 8, SSD1306_WHITE);
    
    // Uptime
    display.setCursor(0, 10);
    display.print(F("Uptime: "));
    char timeStr[12];
    formatTime(uptime, timeStr, sizeof(timeStr));
    display.println(timeStr);
    
    // Packet statistics
    display.setCursor(0, 20);
    display.print(F("Packets: "));
    display.println(totalPackets);
    
    display.setCursor(0, 30);
    display.print(F("Errors: "));
    display.println(errorPackets);
    
    display.setCursor(0, 40);
    if (totalPackets > 0) {
        float errorRate = (float)errorPackets / totalPackets * 100.0;
        display.print(F("Error Rate: "));
        display.print(errorRate, 1);
        display.println(F("%"));
    }
    
    // Page indicator
    display.setCursor(0, 56);
    display.print(F("Page 4/"));
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

void DisplayManager::formatTime(unsigned long seconds, char* buffer, size_t bufferSize) {
    // Format time as HH:MM:SS or MM:SS or SS depending on magnitude
    if (seconds < 60) {
        snprintf(buffer, bufferSize, "%lus", seconds);
    } else if (seconds < 3600) {
        unsigned long minutes = seconds / 60;
        seconds %= 60;
        snprintf(buffer, bufferSize, "%lum %lus", minutes, seconds);
    } else {
        unsigned long hours = seconds / 3600;
        seconds %= 3600;
        unsigned long minutes = seconds / 60;
        seconds %= 60;
        snprintf(buffer, bufferSize, "%luh %lum", hours, minutes);
    }
}
