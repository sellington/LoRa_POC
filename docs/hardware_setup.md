# Hardware Setup Guide

This document describes the hardware setup for the LoRa POC project, including connections, components, and configuration.

## Components List

### Remote Device
- MakerFocus ESP32 LoRa Board
- OLED Display (SSD1306, 128x64)
- 2x Small Solar Panels (5V, connected in parallel)
- 3.7V LiPo Battery (compatible with the board's battery connector)
- Diode (1N4007 or similar)
- Capacitor (470μF-1000μF)
- Jumper wires
- Optional: Buck converter module (if solar panel voltage needs regulation)

### Base Station
- MakerFocus ESP32 LoRa Board
- OLED Display (SSD1306, 128x64)
- USB cable for power and serial communication
- Jumper wires

## Pin Connections

### OLED Display Connection (Both Devices)
| OLED Pin | ESP32 Pin  |
|----------|------------|
| VCC      | 3.3V       |
| GND      | GND        |
| SCL      | GPIO 22    |
| SDA      | GPIO 21    |

### LoRa Module (Built into MakerFocus ESP32 Board)
The LoRa module is integrated into the MakerFocus ESP32 LoRa board. The default connections are:

| Function | ESP32 Pin  |
|----------|------------|
| NSS      | GPIO 18    |
| RESET    | GPIO 14    |
| DIO0     | GPIO 26    |
| SCK      | GPIO 5     |
| MISO     | GPIO 19    |
| MOSI     | GPIO 27    |

### Power Circuit (Remote Device)

```
Solar Panels (+ in parallel) → Diode → Capacitor → ESP32 LoRa Board → LiPo Battery
```

#### Details:
1. Connect the positive terminals of both solar panels together
2. Connect the negative terminals of both solar panels together
3. Connect the combined positive terminal through a diode (cathode toward the board)
4. Add a capacitor between the diode output and ground for stabilization
5. Connect the diode output to the 5V input of the ESP32 LoRa board
6. Connect the combined negative terminal to the GND of the ESP32 LoRa board
7. Connect the LiPo battery to the battery connector on the ESP32 LoRa board

## Power Management

### Battery Charging
The MakerFocus ESP32 LoRa board has a built-in battery management system that handles charging the LiPo battery when external power is available. The charging circuit typically includes:

- TP4056 or similar lithium battery charging IC
- Input voltage protection
- Over-current protection
- Battery level monitoring

### Solar Power Considerations
- The solar panels should provide at least 5V and enough current to charge the battery effectively
- Direct sunlight will provide the best charging performance
- Position the panels to maximize sun exposure throughout the day
- The diode prevents battery discharge through the solar panels during low-light conditions

## Device Configuration

### Remote Device
- The battery will power the device when solar power is insufficient
- The ESP32 will use sleep modes to conserve power between transmissions
- Battery voltage will be monitored through an ADC pin connected to the battery

### Base Station
- Always powered via USB
- Serial communication available through USB connection
- No power management needed for this device

## Initial Setup Procedure

1. Assemble both devices according to the pin connections described above
2. For the remote device, connect the solar charging circuit
3. For the base station, connect the USB cable
4. Upload the appropriate firmware to each device using PlatformIO
5. Position the remote device in a location with good sun exposure
6. Position the base station within LoRa range of the remote device
7. Monitor the initial communication and adjust positions if needed

## Troubleshooting

### Solar Charging Issues
- Verify diode polarity (cathode toward the ESP32)
- Check for sufficient sunlight exposure
- Measure solar panel output voltage (should be at least 5V in sunlight)
- Ensure battery connections are secure

### Communication Issues
- Verify LoRa module connections
- Check that both devices are powered
- Ensure devices are within LoRa range (can be up to several km with clear line of sight)
- Verify antenna connection and orientation

### Display Issues
- Check I2C connections (SDA, SCL)
- Verify display is receiving power (VCC, GND)
- Confirm I2C address in the software matches the display (typically 0x3C or 0x3D)
