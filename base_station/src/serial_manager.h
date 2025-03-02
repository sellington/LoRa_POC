#ifndef SERIAL_MANAGER_H
#define SERIAL_MANAGER_H

#include <Arduino.h>
#include <ArduinoJson.h>

// Serial parameters
#define SERIAL_BAUD_RATE    115200
#define SERIAL_BUFFER_SIZE  512

// Serial command prefixes
#define CMD_PREFIX          "CMD:"
#define PING_COMMAND        "PING"
#define STATUS_COMMAND      "STATUS"
#define RESET_COMMAND       "RESET"
#define CONFIG_COMMAND      "CONFIG"

class SerialManager {
public:
    SerialManager();
    
    // Initialize serial communication
    void begin();
    
    // Process any incoming serial commands
    void processCommands();
    
    // Send metrics data to serial
    void sendMetrics(const JsonDocument& metrics);
    
    // Send a status message to serial
    void sendStatus(const char* status);
    
    // Send remote device data to serial
    void sendRemoteData(const JsonDocument& data);
    
    // Send signal metrics to serial
    void sendSignalMetrics(int rssi, float snr, float packetLoss, float avgLatency);
    
    // Send system metrics to serial
    void sendSystemMetrics(unsigned long uptime, unsigned long packets, unsigned long errors);
    
    // Send an error message to serial
    void sendError(const char* errorMessage);
    
    // Send a log message to serial
    void log(const char* message);
    
    // Send a debug message to serial (only if debug is enabled)
    void debug(const char* message);
    
    // Set debug mode
    void setDebugMode(bool enabled);
    
    // Check if a command is available
    bool isCommandAvailable();
    
    // Get the next command from the buffer
    String getNextCommand();
    
private:
    bool debugEnabled;
    char inputBuffer[SERIAL_BUFFER_SIZE];
    int bufferIndex;
    
    // Parse a command string
    void parseCommand(const String& command);
    
    // Execute a command
    void executeCommand(const String& command, const String& params);
    
    // Send a JSON response
    void sendJsonResponse(const JsonDocument& response);
    
    // Process configuration command
    void processConfigCommand(const String& params);
};

extern SerialManager serialManager;

#endif // SERIAL_MANAGER_H
