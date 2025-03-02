# LoRa Communication Protocol

This document defines the communication protocol used between the remote device and base station in the LoRa POC project.

## Overview

The protocol uses JSON-formatted messages to exchange data between devices. Each message has a specific type and contains metadata along with payload information.

## Message Format

```json
{
  "type": "[message_type]",
  "id": [message_id],
  "timestamp": [unix_timestamp],
  "metrics": {
    "rssi": [signal_strength],
    "snr": [signal_to_noise_ratio],
    "battery": [battery_voltage],
    "temp": [temperature],
    "uptime": [seconds_since_boot]
  },
  "payload": "Optional additional data"
}
```

### Fields

| Field | Type | Description |
|-------|------|-------------|
| `type` | String | Message type: "ping", "pong", "data", "status" |
| `id` | Integer | Unique message identifier |
| `timestamp` | Integer | Unix timestamp when message was created |
| `metrics` | Object | Contains various metrics data |
| `payload` | String | Optional additional data |

### Metrics Fields

| Field | Type | Description | Units |
|-------|------|-------------|-------|
| `rssi` | Integer | Received Signal Strength Indicator | dBm |
| `snr` | Float | Signal-to-Noise Ratio | dB |
| `battery` | Float | Battery voltage | V |
| `temp` | Float | CPU temperature | °C |
| `uptime` | Integer | Time since last boot | seconds |

## Message Types

### Ping

Sent by either device to test connectivity.

```json
{
  "type": "ping",
  "id": 12345,
  "timestamp": 1646245245,
  "metrics": {
    "rssi": -75,
    "snr": 9.5,
    "battery": 3.8,
    "temp": 28.5,
    "uptime": 3600
  }
}
```

### Pong

Response to a ping message.

```json
{
  "type": "pong",
  "id": 12345,
  "timestamp": 1646245246,
  "metrics": {
    "rssi": -73,
    "snr": 10.0,
    "battery": 5.0,
    "temp": 29.0,
    "uptime": 7200
  }
}
```

### Data

Regular data transmission from the remote device.

```json
{
  "type": "data",
  "id": 12346,
  "timestamp": 1646245300,
  "metrics": {
    "rssi": -75,
    "snr": 9.5,
    "battery": 3.8,
    "temp": 28.5,
    "uptime": 3600,
    "solar_voltage": 5.2,
    "packet_loss": 0.02
  },
  "payload": "Additional sensor data could go here"
}
```

### Status

System status updates.

```json
{
  "type": "status",
  "id": 12347,
  "timestamp": 1646245400,
  "metrics": {
    "rssi": -75,
    "snr": 9.5,
    "battery": 3.8,
    "temp": 28.5,
    "uptime": 3600,
    "free_memory": 153624,
    "cpu_usage": 12
  },
  "payload": "System status: normal"
}
```

## Protocol Flow

1. Remote device wakes up from sleep
2. Remote device collects metrics data
3. Remote device sends "data" message to base station
4. Base station acknowledges with a brief response
5. Remote device goes back to sleep

## Error Handling

- If a message is not acknowledged, the sender should retry up to 3 times
- Retries should use exponential backoff (e.g., wait times of 1s, 2s, 4s)
- After multiple failures, the remote device should log the error and may go into a power-saving mode

## Future Extensions

The protocol is designed to be extensible. Additional fields can be added to the metrics object or payload as needed for future functionality.
