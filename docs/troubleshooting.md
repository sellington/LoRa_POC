# LoRa POC Troubleshooting Guide

Last Updated: 2025-03-01

## Current Issues and Solutions

### LoRa Communication Issues

#### Remote Device Sends But No Acknowledgments

**Issue:** The remote device successfully initializes and sends messages, but doesn't receive acknowledgments from the base station, resulting in timeout errors.

**Log Output:**
```
LoRa Remote Device Starting...
...
Initializing LoRa module with correct pin configuration... LoRa module initialized!
...
Sending message (attempt 1): {"type":"data","id":1,"timestamp":34,"metrics":{...}}
Acknowledgment timeout
...
Failed to send message after max retries
```

**Possible Causes and Solutions:**

1. **Base Station Not Running**
   - Ensure the base station is powered on and the code is running
   - Verify the base station's serial output for any errors during initialization

2. **Configuration Mismatch**
   - Verify that both devices use identical LoRa parameters:
     - Frequency: 915.0 MHz
     - Bandwidth: 500.0 kHz
     - Spreading Factor: 6
     - Coding Rate: 5
     - Sync Word: 0x34
   - Double-check these values in both `remote_device/src/lora_communication.h` and `base_station/src/lora_communication.h`

3. **Base Station Not Processing Messages**
   - Check if the base station is correctly implementing the message handler
   - Ensure it's listening and parsing the incoming messages
   - Verify the base station properly sends acknowledgment (PONG) responses

4. **Hardware Signal Issues**
   - Try testing with devices physically close to each other
   - Increase the output power (modify LORA_POWER value) for testing
   - Check antenna connections on both devices

5. **Timing and Retry Issues**
   - The ACK_TIMEOUT (1000ms) might be too short - could try increasing it
   - Look at the MAX_RETRIES (3) value and potentially increase it for testing

## Debug Strategies

### Add Debug Logging on Base Station

Add more detailed debug prints to the base station code to trace what's happening when messages arrive:

```cpp
// In base_station/src/lora_communication.cpp
bool LoRaCommunication::receiveMessage(JsonDocument& doc, int* rssi, float* snr) {
    // ...existing code...
    
    // Print debug info
    Serial.print(F("Received: "));
    Serial.println(message);
    
    // Add detailed debug for message parsing
    DeserializationError error = deserializeJson(doc, message);
    if (error) {
        Serial.print(F("JSON parsing failed: "));
        Serial.println(error.c_str());
        return false;
    }
    
    // Add debug for message type
    if (doc.containsKey("type")) {
        Serial.print(F("Message type: "));
        Serial.println(doc["type"].as<const char*>());
    } else {
        Serial.println(F("Message has no type field"));
    }
    
    // Add debug for acknowledgment
    if (doc.containsKey("id") && 
        doc.containsKey("type") && 
        strcmp(doc["type"], MSG_TYPE_PONG) != 0) {  // Don't ack an ack
        Serial.print(F("Sending acknowledgment for message ID: "));
        Serial.println(doc["id"].as<uint32_t>());
        sendAcknowledgment(doc["id"]);
    }
    
    return true;
}
```

### Simplify for Initial Testing

Consider temporarily removing the JSON structure for initial testing and using simple strings to verify basic LoRa communication is working:

```cpp
// In test code
const char* testMessage = "PING";
radio.transmit(testMessage, strlen(testMessage));
```

### Run Individual Component Tests

Create a simple test program that only tests the bidirectional communication between two devices, without the complexity of metrics, power management, etc.

## Next Steps When Resuming Development

1. Upload a stripped-down test to both devices focusing only on message exchange
2. Add debug prints to both devices to trace message flow
3. Test with devices close together to rule out signal issues
<<<<<<< HEAD
4. Once bidirectional communication works, re-enable the full functionality
=======
4. Once bidirectional communication works, re-enable the full functionality
>>>>>>> origin/main
