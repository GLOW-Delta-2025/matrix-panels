# Echoes of Tomorrow - Matrix Panels Firmware

This repository contains the embedded firmware for the microcontrollers that operate the matrix panels in the **Echoes of Tomorrow** GLOW 2025 project.

## Overview
- Written in C++ for Arduino framework
- Compiled and uploaded to microcontrollers (Teensy 4.0)
- Controls 5 curtains of LED matrix panels (20×26 LEDs each = 2,600 total LEDs)
- Features a star field animation with configurable parameters
- Serial command interface for real-time control

## Project Structure
```
├── src/
│   ├── main.cpp              # Main loop and initialization
│   ├── config.cpp            # Configuration defaults
│   ├── octo_wrapper.cpp      # OctoWS2811 abstraction
│   ├── renderer.cpp          # Soft buffer & fade effects
│   ├── serial_reader.cpp     # Serial command parser
│   └── stars.cpp             # Star animation logic
├── include/
│   ├── config.h              # Configuration constants
│   ├── octo_wrapper.h        # OctoWS2811 interface
│   ├── renderer.h            # Renderer interface
│   ├── serial_reader.h       # Serial interface
│   ├── stars.h               # Star structure & functions
│   └── mapping.h             # Pixel coordinate mapping
├── lib/
│   └── CmdLib.h              # Command protocol library
└── platformio.ini            # PlatformIO build configuration
```

## Hardware Configuration
- **5 Curtains**: Each curtain is a vertical strip of LEDs
- **Dimensions**: 20 columns × 26 rows per curtain (520 LEDs each)
- **Total LEDs**: 2,600 pixels
- **Pin Configuration**: Customizable via `pinList` in `config.cpp`

## Setup Instructions
1. Install [PlatformIO](https://platformio.org/) or use Arduino IDE with Teensy support
2. Open the project in your IDE
3. Select the target board in `platformio.ini` (default: `teensy40` or `teensy41`)
4. Configure hardware settings in `src/config.cpp` if needed:
   - Adjust `invertCurtain[]` for physical mounting orientation
   - Modify `pinList[]` for custom pin assignments
5. Build and upload to the connected board

### PlatformIO Build
```bash
pio run -e teensy40       # Build for Teensy 4.0
pio run -e teensy41 -t upload   # Upload to Teensy 4.1
```

## Serial Commands
Connect via serial at 9600 baud to send runtime commands:

| Command | Arguments | Description | Example |
|---------|-----------|-------------|---------|
| `COUNT` | `n` | Set number of active stars (1-500) | `COUNT 30` |
| `SPEED` | `min max` | Set speed range (cols/sec) | `SPEED 15 80` |
| `FADE` | `factor` | Set fade factor (0.0-1.0) | `FADE 0.90` |
| `WRAP` | `0/1` | Enable/disable star wrapping | `WRAP 1` |
| `RANDOM_ROWS` | `0/1` | Enable/disable random row placement | `RANDOM_ROWS 1` |
| `ROW` | `r` | Set fixed row for all stars | `ROW 13` |

## Configuration Parameters
Default values in `src/config.cpp`:
- `MAX_STARS`: 500 (maximum allocation)
- `activeStarCount`: 15 (initial active stars)
- `minSpeedColsPerSec`: 10.0 (minimum star speed)
- `maxSpeedColsPerSec`: 100.0 (maximum star speed)
- `fadeFactor`: 0.86 (trail fade per frame)
- `frameTargetMs`: 20 (~50 FPS)
- `STAR_R/G/B`: 255, 191, 0 (default amber color)

## Dependencies
- [OctoWS2811](https://github.com/PaulStoffregen/OctoWS2811) (v1.5+) - High-performance LED control
- [FastLED](https://github.com/FastLED/FastLED) (v3.10.3+) - Optional, for additional effects

## Testing
Diagnostic tests and example sketches will be added to a `tests/` directory as needed.

## Documentation
For more details on architecture, electrical schematics, and integration with the master controller, see the [project Wiki](https://github.com/GLOW-Delta-2025/master/wiki).

## Branches
- `main`: Production-ready code for live installation
- `develop`: Active development and testing
- `feature/<name>`, `bugfix/<name>`, `hotfix/<name>`: Follow Git Flow conventions

## Commit Convention
```text
<type>: <short description>

Types: feat, fix, docs, style, refactor, test, chore
```
**Examples:**
- `feat: add serial command for star color`
- `fix: LED flickering on curtain 3`
- `refactor: optimize star rendering loop`
