# RoboNet — Unified Embedded & IoT Learning Project

## Project Overview

**RoboNet** is a personal learning project that integrates mobile app development, embedded systems, wireless communication protocols, and industrial networking into a single unified platform. The goal is to progressively build a system where a 4-wheel robot is controlled via a smartphone and a PC, while a separate CAN-based sensor network runs in parallel — all communicating through Bluetooth and WiFi.

---

## Learning Objectives

| Technology | What You Learn |
|---|---|
| **Qt 6 Mobile (Android)** | QML, Qt Sensors, Qt Bluetooth, UI design |
| **Qt 6 Desktop (PC)** | Widget/QML app, network communication, controls |
| **Bluetooth Low Energy (BLE)** | GATT profiles, custom services, Android BLE client |
| **ESP32-S3 Firmware** | ESP-IDF, FreeRTOS tasks, BLE GATT server, WiFi |
| **CAN Bus** | Multi-node network, message framing, arbitration |
| **I2C** | Sensor reading (MPU6050, Magnetometer, Humidity) |
| **WiFi / TCP** | ESP32-S3 to PC data bridge |

---

## System Architecture

```
┌─────────────────────────────────────────────────────────┐
│                     PHASE 1 & 2                         │
│                                                         │
│  ┌──────────────┐   BLE    ┌──────────────────────┐    │
│  │  Xiaomi      │ ───────► │  4-Wheel Robot        │    │
│  │  Qt Mobile   │          │  ESP32-S3             │    │
│  │  (Roll/Pitch)│          │  - BLE Server         │    │
│  └──────────────┘          │  - Motor Control      │    │
│                            │  - I2C (MPU6050)      │    │
│  ┌──────────────┐   WiFi   └──────────────────────┘    │
│  │  PC Qt App   │ ───────►                              │
│  │  (Buttons)   │                                       │
│  └──────────────┘                                       │
│                                                         │
├─────────────────────────────────────────────────────────┤
│                     PHASE 3                             │
│                                                         │
│  ┌────────────┐   CAN Bus  ┌────────────┐              │
│  │ ESP32-S3 A │ ─────────► │ ESP32-S3 B │              │
│  │ (Humidity) │            │ (Relays)   │              │
│  └────────────┘            └────────────┘              │
│         │                        │                      │
│         └──────────┬─────────────┘                     │
│                    ▼                                     │
│             ┌────────────┐   WiFi                       │
│             │ ESP32-S3 C │ ─────────► PC Qt App         │
│             │ (Gateway)  │           (Dashboard)        │
│             └────────────┘                              │
└─────────────────────────────────────────────────────────┘
```

---

## Hardware Inventory

| # | Device | Role |
|---|---|---|
| 1 | 4-Wheel Robot (Amazon B0CCNJKM17) | Mobile platform |
| 1 | ESP32-S3 (on robot) | BLE server + motor controller |
| 3 | ESP32-S3 (standalone) | CAN network nodes |
| 1 | Xiaomi Redmi Note 9 Pro | Qt mobile controller (BLE client) |
| 1 | PC (Windows) | Qt desktop controller + CAN dashboard |
| — | MPU6050 | IMU — already wired on robot |
| — | Magnetometer | Already wired on robot |
| — | Humidity Sensors | CAN network nodes |
| — | Relay Boards | CAN network nodes |
| — | Character LCD | Already working on robot |

---

## Project Phases

### Phase 1 — Mobile App: Tilt-to-Drive via BLE

Control the 4-wheel robot by tilting the phone. Roll controls left/right. Pitch controls forward/backward. Roll and Pitch angles are displayed live on the phone screen.

**Components:**
- Qt 6 Android app with Qt Sensors (accelerometer → Roll/Pitch)
- Qt Bluetooth BLE client (GATT write)
- ESP32-S3 BLE GATT server (ESP-IDF + NimBLE)
- Motor control logic on ESP32-S3

---

### Phase 2 — PC App: Button-Based Control via WiFi

A desktop Qt app with directional buttons (Forward / Backward / Left / Right / Stop) that sends commands to the robot over WiFi (TCP or UDP).

**Components:**
- Qt 6 Desktop app (QML or Widgets)
- Qt Network module (QTcpSocket or QUdpSocket)
- ESP32-S3 WiFi station mode + TCP/UDP server
- Shared motor control logic with Phase 1

---

### Phase 3 — CAN Sensor Network

Three ESP32-S3 boards connected via CAN bus. One acts as a WiFi gateway and forwards sensor data and relay states to the PC Qt dashboard.

**Components:**
- CAN bus wiring between 3 ESP32-S3 boards
- Node A: reads humidity sensor, publishes on CAN
- Node B: controls relays, reads switches, publishes on CAN
- Node C: CAN gateway — aggregates data, sends to PC over WiFi
- PC Qt app: real-time dashboard showing humidity, relay states, switch inputs

---

## BLE Communication Design (Phase 1)

```
Service UUID:        12345678-1234-1234-1234-123456789ABC
Characteristic UUID: 12345678-1234-1234-1234-123456789DEF

Payload format:  "R:-12.5,P:8.3"
Update rate:     20 Hz (every 50 ms)
Direction:       Phone (Write) → ESP32-S3
```

### Tilt-to-Motor Mapping

```
Pitch > +10°   →   Forward   (speed ∝ angle)
Pitch < -10°   →   Backward  (speed ∝ angle)
Roll  > +10°   →   Turn Right
Roll  < -10°   →   Turn Left
|angle| < 5°   →   Stop (dead zone)
```

---

## WiFi Communication Design (Phase 2)

```
Protocol:   TCP (reliable, ordered)
Port:       8080
Commands:   "FWD", "BWD", "LEFT", "RIGHT", "STOP"
Direction:  PC Qt App → ESP32-S3
```

---

## CAN Network Design (Phase 3)

```
Bitrate:     500 kbps
Node A ID:   0x100  (Humidity data)
Node B ID:   0x200  (Relay states + switch inputs)
Node C ID:   0x300  (Gateway — aggregates + sends WiFi)

Frame format (8 bytes):
  Byte 0:    Node ID
  Byte 1:    Message type
  Bytes 2-7: Payload
```

---

## Qt Project Structure

### Mobile App (Phase 1)
```
QtRobotMobile/
├── CMakeLists.txt
├── main.cpp
├── SensorManager.h/.cpp       ← Accelerometer → Roll/Pitch
├── BleController.h/.cpp       ← BLE scan, connect, write
└── qml/
    ├── main.qml
    ├── Dashboard.qml           ← Live Roll/Pitch display
    └── ConnectionPanel.qml    ← BLE device list + connect
```

### Desktop App (Phase 2 + 3)
```
QtRobotDesktop/
├── CMakeLists.txt
├── main.cpp
├── RobotController.h/.cpp     ← TCP/UDP command sender
├── CanDashboard.h/.cpp        ← CAN data display
└── qml/
    ├── main.qml
    ├── ControlPanel.qml       ← D-pad buttons
    └── SensorDashboard.qml   ← Humidity, relays, switches
```

---

## ESP32-S3 Firmware Structure (Robot)

```
robot_firmware/
├── main/
│   ├── main.c
│   ├── ble_server.c/.h        ← NimBLE GATT server
│   ├── wifi_server.c/.h       ← TCP server for PC app
│   ├── motor_control.c/.h     ← PWM motor driver
│   └── command_parser.c/.h    ← Parse BLE/WiFi commands
├── CMakeLists.txt
└── sdkconfig
```

---

## Development Roadmap

| Step | Description | Phase |
|---|---|---|
| 1 | Qt Mobile: Read accelerometer, show Roll & Pitch | 1 |
| 2 | Qt Mobile: BLE scan & connect UI | 1 |
| 3 | Qt Mobile: Send Roll/Pitch over BLE | 1 |
| 4 | ESP32-S3: BLE server, print received data to serial | 1 |
| 5 | ESP32-S3: Parse Roll/Pitch, drive motors | 1 |
| 6 | Tune dead zones and speed mapping | 1 |
| 7 | Qt Desktop: D-pad control UI | 2 |
| 8 | Qt Desktop: Send commands via TCP | 2 |
| 9 | ESP32-S3: Add WiFi TCP server, share motor logic | 2 |
| 10 | Test PC + Mobile simultaneously | 2 |
| 11 | CAN Node A: Humidity sensor reads + CAN publish | 3 |
| 12 | CAN Node B: Relay control + switch read + CAN publish | 3 |
| 13 | CAN Node C: Gateway — receive CAN, forward via WiFi | 3 |
| 14 | Qt Desktop: Add CAN dashboard tab | 3 |
| 15 | End-to-end integration test of all 3 phases | 3 |

---

## Notes & Conventions

- All ESP32-S3 firmware uses **ESP-IDF** (not Arduino)
- BLE stack: **NimBLE** (lighter than Bluedroid)
- Qt version: **Qt 6.x**
- Android target ABI: **arm64-v8a** (Xiaomi Redmi Note 9 Pro)
- CAN transceiver required: **SN65HVD230** or equivalent for each node
- Motors use PWM via **MCPWM peripheral** on ESP32-S3

---

*Project started: 2026 | Author: Morteza*
