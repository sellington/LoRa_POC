#include "power_management.h"
#include <esp_sleep.h>
#include <esp_adc_cal.h>

// Global instance
PowerManagement powerManagement;

PowerManagement::PowerManagement() :
    batteryVoltage(0.0),
    lastBatteryVoltage(0.0),
    solarVoltage(0.0),
    batteryStatus(BATTERY_STATUS_NORMAL),
    chargingStatus(CHARGING_UNKNOWN),
    adcCalibration(1.0),
    lastBatteryReadTime(0) {
}

void PowerManagement::begin() {
    // Configure ADC pins
    pinMode(BATTERY_ADC_PIN, INPUT);
    pinMode(SOLAR_ADC_PIN, INPUT);
    
    // Configure ADC
    analogReadResolution(12);  // 12-bit resolution for ESP32
    
    // Set ADC attenuation
    analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);  // For wider voltage range
    analogSetPinAttenuation(SOLAR_ADC_PIN, ADC_11db);
    
    // Initial readings
    updateBatteryStatus();
    updateChargingStatus();
    
    Serial.println(F("Power management system initialized"));
}

float PowerManagement::getBatteryVoltage() {
    // Read and update if it's been a while since the last reading
    if (millis() - lastBatteryReadTime > 5000) {
        updateBatteryStatus();
    }
    return batteryVoltage;
}

float PowerManagement::getSolarVoltage() {
    // Read the solar panel voltage
    uint16_t adcValue = analogRead(SOLAR_ADC_PIN);
    solarVoltage = adcToVoltage(adcValue);
    return solarVoltage;
}

BatteryStatus PowerManagement::getBatteryStatus() {
    // Update status if needed
    if (millis() - lastBatteryReadTime > 5000) {
        updateBatteryStatus();
    }
    return batteryStatus;
}

ChargingStatus PowerManagement::getChargingStatus() {
    updateChargingStatus();
    return chargingStatus;
}

uint32_t PowerManagement::getSleepDuration() {
    // Return the appropriate sleep duration based on battery status
    switch (batteryStatus) {
        case BATTERY_STATUS_LOW:
            return SLEEP_DURATION_LOW;
        case BATTERY_STATUS_CRITICAL:
            return SLEEP_DURATION_CRITICAL;
        case BATTERY_STATUS_NORMAL:
        default:
            return SLEEP_DURATION_NORMAL;
    }
}

void PowerManagement::lightSleep(uint32_t seconds) {
    Serial.print(F("Entering light sleep for "));
    Serial.print(seconds);
    Serial.println(F(" seconds"));
    
    // Calculate microseconds
    uint64_t sleepTime = seconds * 1000000ULL;
    
    // Configure timer wakeup
    esp_sleep_enable_timer_wakeup(sleepTime);
    
    // Enter light sleep
    Serial.flush();
    esp_light_sleep_start();
    
    // Code continues here after wakeup
    Serial.println(F("Woke up from light sleep"));
}

void PowerManagement::deepSleep(uint32_t seconds) {
    Serial.print(F("Entering deep sleep for "));
    Serial.print(seconds);
    Serial.println(F(" seconds"));
    
    // Calculate microseconds
    uint64_t sleepTime = seconds * 1000000ULL;
    
    // Configure timer wakeup
    esp_sleep_enable_timer_wakeup(sleepTime);
    
    // Enter deep sleep (device will reset after waking up)
    Serial.flush();
    esp_deep_sleep_start();
    
    // Code will not reach here as deep sleep causes a reset
}

void PowerManagement::smartSleep() {
    // Update battery and charging status
    updateBatteryStatus();
    updateChargingStatus();
    
    // Get sleep duration based on battery status
    uint32_t duration = getSleepDuration();
    
    // Choose sleep mode based on battery status and charging state
    if (batteryStatus == BATTERY_STATUS_CRITICAL && chargingStatus != CHARGING) {
        // Critical battery and not charging - use deep sleep
        deepSleep(duration);
    } else {
        // Normal operation or charging - use light sleep
        lightSleep(duration);
    }
}

void PowerManagement::calibrateBatteryADC(float knownVoltage) {
    // Read raw ADC value
    uint16_t adcValue = analogRead(BATTERY_ADC_PIN);
    float measuredVoltage = adcToVoltage(adcValue);
    
    // Calculate calibration factor
    if (measuredVoltage > 0) {
        adcCalibration = knownVoltage / measuredVoltage;
        Serial.print(F("ADC calibration factor: "));
        Serial.println(adcCalibration);
    }
}

uint8_t PowerManagement::getBatteryPercentage() {
    // Make sure we have the latest battery reading
    if (millis() - lastBatteryReadTime > 5000) {
        updateBatteryStatus();
    }
    
    // Calculate percentage based on min/max voltage
    float percentage = (batteryVoltage - BATTERY_MIN) / (BATTERY_MAX - BATTERY_MIN) * 100.0;
    
    // Constrain to valid range
    if (percentage > 100.0) percentage = 100.0;
    if (percentage < 0.0) percentage = 0.0;
    
    return (uint8_t)percentage;
}

float PowerManagement::adcToVoltage(uint16_t adcValue) {
    // Convert ADC reading to voltage
    // For ESP32 with default ADC attenuation (11dB), the full range is 0-3.9V
    // But we use a voltage divider to measure higher voltages
    
    // This is a simplified conversion - should be calibrated for your specific hardware
    float voltage = (adcValue / 4095.0) * 3.9 * adcCalibration;
    
    // If you have a voltage divider, apply the division factor here
    // For example, if you have a 100K/100K divider, multiply by 2
    // voltage *= 2.0;
    
    return voltage;
}

void PowerManagement::updateBatteryStatus() {
    // Read the battery voltage
    uint16_t adcValue = analogRead(BATTERY_ADC_PIN);
    lastBatteryVoltage = batteryVoltage;
    batteryVoltage = adcToVoltage(adcValue);
    
    // Update the last read time
    lastBatteryReadTime = millis();
    
    // Update the battery status
    if (batteryVoltage <= BATTERY_CRITICAL) {
        batteryStatus = BATTERY_STATUS_CRITICAL;
    } else if (batteryVoltage <= BATTERY_LOW) {
        batteryStatus = BATTERY_STATUS_LOW;
    } else {
        batteryStatus = BATTERY_STATUS_NORMAL;
    }
    
    // Debug print
    Serial.print(F("Battery voltage: "));
    Serial.print(batteryVoltage);
    Serial.print(F("V, Status: "));
    Serial.println(batteryStatus);
}

void PowerManagement::updateChargingStatus() {
    // Read the solar panel voltage
    float solar = getSolarVoltage();
    
    // Read the current battery voltage
    float battery = getBatteryVoltage();
    
    // Determine charging status based on solar panel voltage and battery trend
    if (solar > battery + 0.5) {  // Solar voltage significantly higher than battery
        // Check if battery voltage is trending upward
        if (battery > lastBatteryVoltage && solar > 4.5) {
            chargingStatus = CHARGING;
        } else {
            chargingStatus = NOT_CHARGING;
        }
    } else {
        chargingStatus = NOT_CHARGING;
    }
    
    // Debug print
    Serial.print(F("Solar voltage: "));
    Serial.print(solar);
    Serial.print(F("V, Charging status: "));
    Serial.println(chargingStatus);
}
