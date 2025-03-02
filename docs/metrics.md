# Metrics Documentation

This document details the metrics collected and analyzed in the LoRa POC project, explaining their purpose, collection methods, and significance.

## Overview

Metrics are collected to evaluate:
1. RF communication quality
2. System reliability
3. Performance characteristics
4. Power management efficiency
5. Overall system health

## Metrics Categories

### Signal Quality Metrics

| Metric | Description | Units | Target Range |
|--------|-------------|-------|--------------|
| RSSI | Received Signal Strength Indicator | dBm | -120 to -40 |
| SNR | Signal-to-Noise Ratio | dB | >5 for reliable communication |

#### Collection Method:
- RSSI and SNR are provided by the RadioLib library after each reception
- These values are captured for every packet received
- Trends over time are analyzed to detect potential issues

#### Significance:
- RSSI: Indicates signal power at the receiver
  - Higher (less negative) values indicate stronger signal
  - Values below -100 dBm may lead to unreliable communication
- SNR: Indicates how much the signal stands out from noise
  - Higher values indicate cleaner signal
  - Typically need SNR > 5 dB for reliable LoRa communication

### Reliability Metrics

| Metric | Description | Units | Target Value |
|--------|-------------|-------|--------------|
| Packet Loss Rate | Percentage of packets lost | % | <5% |
| Transmission Success Rate | Percentage of successful transmissions | % | >95% |
| Retry Count | Number of retransmission attempts needed | Count | <2 |

#### Collection Method:
- Each packet has a unique ID
- Base station tracks received IDs to identify missing packets
- Remote device tracks acknowledgments to measure delivery success
- Both devices count retry attempts

#### Significance:
- Packet Loss Rate: Critical for assessing link reliability
  - High loss may indicate interference or range issues
  - Spikes in loss can reveal temporal interference patterns
- Transmission Success Rate: Direct measure of communication reliability
- Retry Count: Indicates difficulty in establishing successful transmission
  - High retry counts may indicate borderline signal conditions
  - Can impact power consumption significantly

### Performance Metrics

| Metric | Description | Units | Target Value |
|--------|-------------|-------|--------------|
| Round-trip Latency | Time from transmission to acknowledgment | ms | <1000 ms |
| Throughput | Effective data transfer rate | bytes/s | Depends on distance |

#### Collection Method:
- Timestamps added to each message
- Round-trip time calculated from ping/pong sequence
- Throughput calculated by dividing payload size by transfer time

#### Significance:
- Latency: Important for applications requiring timely data delivery
  - Affected by LoRa parameters (spreading factor, bandwidth)
  - Higher latency may indicate processing delays or interference
- Throughput: Measures effective transmission capacity
  - Trade-off between range and throughput in LoRa
  - Can be optimized by adjusting spreading factor and coding rate

### System Health Metrics

| Metric | Description | Units | Target Range |
|--------|-------------|-------|--------------|
| Battery Voltage | Current battery level | V | 3.3-4.2V |
| Solar Charging Status | Indicator of charging activity | Boolean | - |
| CPU Temperature | Internal temperature of ESP32 | °C | <80°C |
| Free Memory | Available RAM | bytes | >10KB |
| Uptime | Time since last boot | seconds | - |

#### Collection Method:
- Battery voltage measured via ADC pin
- Solar charging detected by comparing voltage trends
- Temperature read from internal temperature sensor
- Memory and uptime from ESP32 system functions

#### Significance:
- Battery Voltage: Critical for power management
  - Below 3.3V indicates need for power conservation
  - Trend analysis helps predict runtime on battery
- Solar Charging Status: Helps evaluate power sustainability
- CPU Temperature: Can indicate overheating issues
  - High temperatures may require reduced duty cycle
- Free Memory: Low memory can cause stability issues
- Uptime: Used to track stability and reboot frequency

## Advanced Analysis

### Metric Correlations
- Battery voltage vs. transmission frequency
- RSSI vs. packet loss rate
- Temperature vs. battery charging efficiency
- SNR vs. retry count

### Long-term Trends
- Daily patterns in solar charging
- Weekly patterns in communication quality
- Battery discharge rate during low-light periods
- Signal quality variation with weather conditions

## Visualization and Reporting

### On OLED Display
- Current battery voltage
- Last RSSI/SNR values
- Packet success rate (last 10 packets)
- System status indicator

### Via Serial (Base Station)
- Complete metrics table
- Historical trends (last 24 hours)
- Alert conditions with timestamps
- Performance recommendations

## Thresholds and Adaptive Behavior

| Metric | Threshold | Adaptive Action |
|--------|-----------|-----------------|
| Battery Voltage | <3.5V | Reduce transmission frequency |
| Battery Voltage | <3.3V | Enter deep sleep mode |
| RSSI | <-100 dBm | Increase transmit power |
| Packet Loss | >10% | Reduce spreading factor |
| Temperature | >70°C | Reduce CPU speed |
| Free Memory | <5KB | Restart device |

These thresholds trigger automatic adjustments to optimize reliability and power consumption based on current conditions.
