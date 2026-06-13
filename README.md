# RoboMaz

A robotics project featuring an ESP32-S3 based 4-wheel mecanum robot with mobile and desktop control applications.

## Overview

RoboMaz is a multi-phase robotics project combining embedded systems, mobile development, and IoT connectivity. Control the robot via mobile app (Android) or desktop application, with distributed sensor nodes connected via CAN bus.

## Key Components

- **Robot Controller**: ESP32-S3 with motor control (4-wheel mecanum)
- **Mobile App**: Qt 6 Android application with BLE connectivity
- **Desktop App**: Qt 6 desktop application for advanced control
- **Sensor Nodes**: Distributed ESP32-S3 CAN bus nodes (humidity, relay, gateway)

## Technology Stack

- **C++** (68.4%) - Core logic and motor control
- **QML** (21.7%) - UI components
- **CMake** (2.8%) - Build configuration

## Project Structure
