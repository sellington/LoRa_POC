#include "serial_manager.h"

// Global instance
SerialManager serialManager;

SerialManager::SerialManager() : 
    debugEnabled(true),
    bufferIndex(0) {
    
    // Initialize buffer
    inputBuffer[0] = '\0';
}

void SerialManager::begin() {
    // Serial is already initialized in main.cpp
    Serial.println(F("Serial manager initialized"));
}

void SerialManager::processCommands() {
    // Check if serial data is available
    while (Serial.available()) {
        char c = Serial.read();
        
        // Add to buffer if not a newline
        if (c != '\n' && c != '\r') {
            // Prevent buffer overflow
            if (bufferIndex < SERIAL_BUFFER_SIZE - 1) {
                inputBuffer[bufferIndex++] = c;
                inputBuffer[bufferIndex] = '\0';
            }
        } else if (bufferIndex > 0) {
            // Process the command when a newline is received
            String command = String(inputBuffer);
            
            // Check if it's a command
            if (command.startsWith(CMD_PREFIX)) {
                command.remove(0, strlen(CMD_PREFIX));  // Remove the prefix
                parseCommand(command);
            }
            
            // Reset buffer
            bufferIndex = 0;
            inputBuffer[0] = '\0';
        }
    }
}

void SerialManager::sendMetrics(const JsonDocument& metrics) {
    // Create a response
    StaticJsonDocument<512> response;
    response["type"] = "metrics";
    response["data"] = metrics;
    
    // Send the response
    sendJsonResponse(response);
}

void SerialManager::sendStatus(const char* status) {
    // Create a response
    StaticJsonDocument<256> response;
    response["type"] = "status";
    response["message"] = status;
    response["timestamp"] = millis() / 1000;
    
    // Send the response
    sendJsonResponse(response);
}

void SerialManager::sendRemoteData(const JsonDocument& data) {
    // Create a response
    StaticJsonDocument<512> response;
    response["type"] = "remote_data";
    response["data"] = data;
    
    // Send the response
    sendJsonResponse(response);
}

void SerialManager::sendSignalMetrics(int rssi, float snr, float packetLoss, float avgLatency) {
    // Create a response
    StaticJsonDocument<256> response;
    response["type"] = "signal_metrics";
    
    JsonObject metrics = response.createNestedObject("metrics");
    metrics["rssi"] = rssi;
    metrics["snr"] = snr;
    metrics["packet_loss"] = packetLoss;
    metrics["latency"] = avgLatency;
    
    // Send the response
    sendJsonResponse(response);
}

void SerialManager::sendSystemMetrics(unsigned long uptime, unsigned long packets, unsigned long errors) {
    // Create a response
    StaticJsonDocument<256> response;
    response["type"] = "system_metrics";
    
    JsonObject metrics = response.createNestedObject("metrics");
    metrics["uptime"] = uptime;
    metrics["packets_received"] = packets;
    metrics["errors"] = errors;
    
    // Send the response
    sendJsonResponse(response);
}

void SerialManager::sendError(const char* errorMessage) {
    // Create a response
    StaticJsonDocument<256> response;
    response["type"] = "error";
    response["message"] = errorMessage;
    response["timestamp"] = millis() / 1000;
    
    // Send the response
    sendJsonResponse(response);
}

void SerialManager::log(const char* message) {
    // Create a response
    StaticJsonDocument<256> response;
    response["type"] = "log";
    response["message"] = message;
    response["timestamp"] = millis() / 1000;
    
    // Send the response
    sendJsonResponse(response);
}

void SerialManager::debug(const char* message) {
    // Only send if debug mode is enabled
    if (debugEnabled) {
        // Create a response
        StaticJsonDocument<256> response;
        response["type"] = "debug";
        response["message"] = message;
        response["timestamp"] = millis() / 1000;
        
        // Send the response
        sendJsonResponse(response);
    }
}

void SerialManager::setDebugMode(bool enabled) {
    debugEnabled = enabled;
    
    // Log the change
    char message[64];
    snprintf(message, sizeof(message), "Debug mode %s", enabled ? "enabled" : "disabled");
    log(message);
}

bool SerialManager::isCommandAvailable() {
    // Not implemented in this simplified version
    // In a full implementation, would maintain a command queue
    return false;
}

String SerialManager::getNextCommand() {
    // Not implemented in this simplified version
    // In a full implementation, would dequeue from a command queue
    return String("");
}

void SerialManager::parseCommand(const String& command) {
    // Split the command into command and parameters
    int spaceIndex = command.indexOf(' ');
    String cmd;
    String params;
    
    if (spaceIndex == -1) {
        // No space found, entire string is the command
        cmd = command;
        params = "";
    } else {
        // Split at the space
        cmd = command.substring(0, spaceIndex);
        params = command.substring(spaceIndex + 1);
    }
    
    // Trim whitespace
    cmd.trim();
    params.trim();
    
    // Execute the command
    executeCommand(cmd, params);
}

void SerialManager::executeCommand(const String& command, const String& params) {
    // Handle the command
    if (command.equalsIgnoreCase(PING_COMMAND)) {
        // Ping command is handled in main.cpp
        // Just log that we received it
        log("Ping command received");
    }
    else if (command.equalsIgnoreCase(STATUS_COMMAND)) {
        // Status command is handled in main.cpp
        log("Status command received");
    }
    else if (command.equalsIgnoreCase(RESET_COMMAND)) {
        // Reset command
        log("Reset command received, restarting device...");
        
        // In a real implementation, would actually reset the device
        // ESP.restart();
    }
    else if (command.equalsIgnoreCase(CONFIG_COMMAND)) {
        // Configuration command
        log("Configuration command received");
        processConfigCommand(params);
    }
    else {
        // Unknown command
        char message[64];
        snprintf(message, sizeof(message), "Unknown command: %s", command.c_str());
        sendError(message);
    }
}

void SerialManager::sendJsonResponse(const JsonDocument& response) {
    // Serialize the JSON
    serializeJson(response, Serial);
    Serial.println();  // Add a newline
}

void SerialManager::processConfigCommand(const String& params) {
    // Parse the parameters as JSON
    StaticJsonDocument<256> config;
    DeserializationError error = deserializeJson(config, params);
    
    if (error) {
        // Failed to parse
        char message[64];
        snprintf(message, sizeof(message), "JSON parsing failed: %s", error.c_str());
        sendError(message);
        return;
    }
    
    // Process the configuration
    bool configChanged = false;
    
    // Handle debug mode setting
    if (config.containsKey("debug")) {
        bool debug = config["debug"];
        setDebugMode(debug);
        configChanged = true;
    }
    
    // Add additional configuration options here
    
    // Log the result
    if (configChanged) {
        log("Configuration updated");
    } else {
        log("No configuration changes");
    }
}
