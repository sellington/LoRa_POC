#include <Arduino.h>

#ifdef IS_REMOTE_DEVICE
  // This is a simple redirection file to the actual source code
  // Empty on purpose - actual implementation is in remote_device/src/main.cpp
#endif

#ifdef IS_BASE_STATION
  // This is a simple redirection file to the actual source code
  // Empty on purpose - actual implementation is in base_station/src/main.cpp
#endif

// Fallback if no environment is defined
#if !defined(IS_REMOTE_DEVICE) && !defined(IS_BASE_STATION)
void setup() {
  Serial.begin(115200);
  Serial.println(F("ERROR: Neither IS_REMOTE_DEVICE nor IS_BASE_STATION is defined."));
  Serial.println(F("Please check your platformio.ini configuration."));
}

void loop() {
  // Blink LED to indicate error
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
#endif
