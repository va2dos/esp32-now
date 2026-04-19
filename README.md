# ESP32-Now

ESP32-Now broadcast using RFID reader

## Project Overview

This project implements an interactive audio system where RFID cards trigger synchronized audio scenarios across multiple ESP32 devices communicating via ESP-NOW. The system consists of a controller device with an RFID reader and multiple speaker devices that play audio tracks in coordination.

## Features

- RFID card detection to trigger audio scenarios
- Multi-device synchronization using ESP-NOW protocol
- Modular architecture with separate controller and speaker roles
- State management for system operation
- Lightning effects using FastLED
- Support for different scenarios: random playback, distributed playback, and sequential bouncing playback
- Configurable track management

## Hardware Requirements

- ESP32 development boards (one for controller, multiple for speakers)
- RFID reader module (Adafruit PN532)
- DFPlayer Mini for audio playback
- FastLED-compatible LED strips for lighting effects
- Push button for manual control (optional)
- MicroSD card for audio files

## Architecture

The project is organized into modules and services:

### Modules
- **Card Module**: Handles RFID card detection and reading
- **Sound Module**: Manages audio playback using DFPlayer Mini
- **Lightning Module**: Controls LED lighting effects

### Services
- **ESP-NOW Service**: Handles wireless communication between devices
- **State Controller**: Manages system states (Idle, Playing, etc.)
- **Scenario Service**: Maps RFID UIDs to scenarios and coordinates playback
- **Track Service**: Manages audio track information
- **Button Service**: Handles button input

### Device Roles
- **Controller**: Reads RFID cards, maps to scenarios, controls local playback and broadcasts commands
- **Speaker**: Receives commands via ESP-NOW and plays audio tracks

## Software Setup

1. Install PlatformIO IDE or VS Code with PlatformIO extension
2. Clone this repository
3. Open the project in PlatformIO
4. Build and upload firmware for each device role:
   - `controller` environment for the main controller device
   - `speaker` environment for speaker devices
   - `mac` environment to read device MAC addresses for configuration

## Usage

1. Configure ESP-NOW peer addresses in the code (update MAC addresses in constants)
2. Load audio files onto MicroSD cards in the appropriate folder structure
3. Flash controller firmware to the main device
4. Flash speaker firmware to additional devices
5. Power on all devices
6. Scan RFID cards to trigger scenarios

### Scenarios

- **Scenario 1**: Random sound from folder 1 on a random speaker
- **Scenario 2**: One sound per speaker from folder 2
- **Scenario 3**: Sequential bouncing playback from folder 3 across all speakers

The system supports up to multiple speakers and can be expanded by adding more devices.
