# LoRa POC Architecture

## System Overview

This project implements a long-range communication system using LoRa technology with ESP32 boards. The system consists of two main components:

1. **Remote Device**: Solar-powered ESP32 LoRa board that collects metrics and transmits them to the base station.
2. **Base Station**: ESP32 LoRa board that receives metrics from the remote device, displays them, and sends them to a computer via serial connection.

## Hardware Architecture

### Remote Device Hardware

- **Microcontroller**: MakerFocus ESP32 LoRa board
- **Power System**:
  - Solar panels (2x in parallel)
  - LiPo battery
  - Battery management system (built into the ESP32 board)
  - Voltage regulation circuit:
    - Diode (to prevent reverse current)
    - Capacitor (for stabilization)
    - Potentially a buck converter
- **Communication**: SX1262 LoRa module (built into the ESP32 board)
- **Display**: OLED display (SSD1306 or similar)
- **Sensors**: On-board temperature sensor, voltage monitoring

### Base Station Hardware

- **Microcontroller**: MakerFocus ESP32 LoRa board
- **Power**: USB power supply
- **Communication**: 
  - SX1262 LoRa module (built into the ESP32 board)
  - USB Serial connection to computer
- **Display**: OLED display (SSD1306 or similar)

### Wiring Diagram (Remote Device Power System)

```
Solar Panels (2x in parallel) → Diode → Capacitor → ESP32 LoRa Board → LiPo Battery
```

## Software Architecture

### Project Structure

```
LoRa_POC/
├── PROJECT_STATUS.md       # Current status and progress
├── ARCHITECTURE.md         # System architecture documentation
├── platformio.ini          # PlatformIO configuration
├── docs/                   # Detailed documentation
│   ├── hardware_setup.md   # Hardware wiring and setup
│   ├── metrics.md          # Metrics definitions and usage
│   └── protocol.md         # Communication protocol specification
├── remote_device/          # Remote device code
│   ├── src/                # Source files
│   └── include/            # Header files
├── base_station/           # Base station code
│   ├── src/                # Source files
│   └── include/            # Header files
└── tests/                  # Test cases
```

### Key Software Components

#### Remote Device

- **Power Management**: 
  - Battery voltage monitoring
  - Sleep mode control
  - Solar charging status
- **LoRa Communication**:
  - Packet formation
  - Transmission handling
  - Error management
- **Metrics Collection**:
  - System health metrics
  - Performance metrics
  - Transmission statistics
- **Display Management**:
  - Status display
  - Metrics visualization

#### Base Station

- **LoRa Communication**:
  - Packet reception
  - Response handling
  - Error management
- **Serial Communication**:
  - Data forwarding to computer
  - Command receiving
- **Metrics Processing**:
  - Data analysis
  - Statistics calculation
- **Display Management**:
  - Status display
  - Metrics visualization

### Communication Protocol

JSON-based protocol for data exchange:

```json
{
  "type": "ping|pong|data|status",
  "id": 12345,
  "timestamp": 1646245245,
  "metrics": {
    "rssi": -75,
    "snr": 9.5,
    "battery": 3.8,
    "temp": 28.5,
    "uptime": 3600
  },
  "payload": "Optional additional data"
}
```

#### Message Types

- **ping/pong**: Connection testing
- **data**: Regular metrics transmission
- **status**: System status updates

### Power Management Strategy

- **Sleep Modes**:
  - Light sleep between transmissions
  - Deep sleep for longer periods if battery is low
- **Adaptive Behavior**:
  - Adjust transmission frequency based on battery level
  - Reduce OLED usage when battery is low
- **Battery Monitoring**:
  - Regular ADC readings of battery voltage
  - Threshold-based behavior changes

## Metrics

### Signal Quality
- RSSI (Received Signal Strength Indicator)
- SNR (Signal-to-Noise Ratio)

### Reliability
- Packet loss rate
- Transmission success rate
- Retry count

### Performance
- Round-trip latency
- Throughput (bytes/second)

### System Health
- Battery voltage (remote device)
- Solar charging status
- CPU temperature
- Free memory
