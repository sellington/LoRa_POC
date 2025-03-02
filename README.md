# LoRa Proof of Concept (LoRa_POC)

This repository contains a proof of concept implementation for a solar-powered long-range communication system using LoRa technology with ESP32 microcontrollers.

## Project Overview

The LoRa_POC project demonstrates a two-node communication system:

1. **Remote Device**: A solar-powered device that collects data and transmits it using LoRa
2. **Base Station**: A device that receives data from the remote node and interfaces with user applications

The system is designed to work over long distances with minimal power consumption, making it suitable for remote monitoring applications where traditional connectivity options are limited.

## Hardware Requirements

- 2x Heltec WiFi LoRa 32 V3 (ESP32-S3) development boards
- Solar panels for the remote device
- LiPo batteries
- OLED displays (built into the Heltec boards)

## Software Architecture

The project is organized into two main components:

- **Remote Device**: Manages power from solar panels, collects metrics, and transmits data
- **Base Station**: Receives data, processes it, and provides user interface via display and serial output

## Repository Structure

```
LoRa_POC/
├── platformio.ini           # PlatformIO configuration
├── remote_device/           # Remote device source code
│   └── src/                 # Implementation files
├── base_station/            # Base station source code
│   └── src/                 # Implementation files
├── src/                     # Test and utility programs
├── docs/                    # Documentation
└── tests/                   # Test cases
```

## Key Features

- Solar power management with battery charging
- Low-power operation for extended battery life
- LoRa communication with optimized parameters
- Comprehensive system health metrics
- OLED display for status visualization

## Getting Started

### Prerequisites

- [PlatformIO](https://platformio.org/) development environment
- ESP32 development setup

### Building and Flashing

1. Clone this repository
2. Open the project in PlatformIO
3. Build and upload to both devices:

```bash
# For the remote device
platformio run -e remote_device -t upload

# For the base station
platformio run -e base_station -t upload
```

## Development Status

See the [PROJECT_STATUS.md](PROJECT_STATUS.md) file for current development status, progress, and known issues.

## Troubleshooting

For common issues and solutions, refer to the [troubleshooting guide](docs/troubleshooting.md).

## License

This project is licensed under the MIT License - see the LICENSE file for details.