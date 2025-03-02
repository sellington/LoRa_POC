#ifndef POWER_MANAGEMENT_H
#define POWER_MANAGEMENT_H

#include <Arduino.h>

// Battery ADC pin (adjusted for ESP32-S3)
// ESP32-S3 ADC1 pins: 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
// Using pins 1 and 2 for battery and solar monitoring
#define BATTERY_ADC_PIN     1  // GPIO1 on ESP32-S3 for ADC
#define SOLAR_ADC_PIN       2  // GPIO2 on ESP32-S3 for ADC

// Battery voltage thresholds
#define BATTERY_NORMAL      3.7   // Battery voltage considered normal (V)
#define BATTERY_LOW         3.5   // Battery voltage considered low (V)
#define BATTERY_CRITICAL    3.3   // Battery voltage considered critical (V)
#define BATTERY_MAX         4.2   // Maximum battery voltage (V)
#define BATTERY_MIN         3.0   // Minimum battery voltage (V)

// Sleep durations
#define SLEEP_DURATION_NORMAL   60    // Normal sleep duration in seconds
#define SLEEP_DURATION_LOW      300   // Low battery sleep duration in seconds
#define SLEEP_DURATION_CRITICAL 1800  // Critical battery sleep duration in seconds

// Battery status enum
enum BatteryStatus {
    BATTERY_STATUS_NORMAL,
    BATTERY_STATUS_LOW,
    BATTERY_STATUS_CRITICAL
};

// Solar charging status enum
enum ChargingStatus {
    NOT_CHARGING,
    CHARGING,
    CHARGING_UNKNOWN
};

class PowerManagement {
public:
    PowerManagement();

    // Initialize power management
    void begin();

    // Read battery voltage
    float getBatteryVoltage();

    // Read solar panel voltage
    float getSolarVoltage();
    
    // Get battery status
    BatteryStatus getBatteryStatus();
    
    // Get solar charging status
    ChargingStatus getChargingStatus();
    
    // Get current sleep duration based on battery status
    uint32_t getSleepDuration();
    
    // Enter light sleep mode for a specific duration
    void lightSleep(uint32_t seconds);
    
    // Enter deep sleep mode for a specific duration
    void deepSleep(uint32_t seconds);
    
    // Smart sleep function that decides which sleep mode to use
    void smartSleep();
    
    // Calibrate the ADC reading for battery voltage
    void calibrateBatteryADC(float knownVoltage);
    
    // Get estimated remaining battery percentage
    uint8_t getBatteryPercentage();
    
private:
    float batteryVoltage;
    float lastBatteryVoltage;
    float solarVoltage;
    BatteryStatus batteryStatus;
    ChargingStatus chargingStatus;
    float adcCalibration;
    unsigned long lastBatteryReadTime;
    
    // Convert ADC reading to voltage
    float adcToVoltage(uint16_t adcValue);
    
    // Read and update battery status
    void updateBatteryStatus();
    
    // Update charging status
    void updateChargingStatus();
};

extern PowerManagement powerManagement;

#endif // POWER_MANAGEMENT_H
