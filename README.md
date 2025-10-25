# LED Curtain Animation System

A Teensy-based LED animation controller for driving OctoWS2811 addressable LED curtains with dynamic star and climax effects.

## Overview

This project controls multiple LED curtain strips in a synchronized animation system. It features a command-based architecture that allows remote control of star particles and climactic buildup/release effects via serial communication.

**Key Features:**
- Multi-curtain LED control with per-strip row inversion support
- Real-time particle system with dynamic star animations
- Serial command interface for remote animation triggering
- Climax effect modes (buildup and spiral animations)
- Customizable star properties (color, speed, brightness, size)
- Soft pixel blending and fade effects

## Hardware Requirements

- **Microcontroller:** Teensy 4.1 (or compatible with OctoWS2811 support)
- **LED Driver:** OctoWS2811 breakout board
- **LEDs:** WS2811/WS2812 addressable LED strips (5x strips of 20×26 pixels by default)
- **Power Supply:** Appropriate capacity for LED count (500 LEDs default)
- **Serial Interface:** USB or Serial1 for command communication

## Configuration

Edit `src/config.cpp` to customize hardware parameters:

```cpp
#define CURTAINS 5                // Number of LED strips
#define CURTAIN_WIDTH 20          // LEDs per strip width
#define CURTAIN_HEIGHT 26         // LEDs per strip height
const int MAX_STARS = 500;        // Maximum simultaneous stars
```

Runtime tunable parameters (modifiable via serial commands):
- `minSpeedColsPerSec` / `maxSpeedColsPerSec` — Star horizontal speed range
- `fadeFactor` — Per-frame LED fade (0.0–1.0)
- `frameTargetMs` — Target frame time (1ms ≈ 50 FPS)
- `randomRows` — Spawn stars at random vertical positions
- `wrapStars` — Loop stars or randomize when exiting
- `STAR_R`, `STAR_G`, `STAR_B` — Default star color

## Serial Command Protocol

Commands use the CmdLib format: `!!source:msgKind:command{param1=value1,param2=value2}##`

### ADD_STAR_CENTER

Spawn animated stars across the curtains.

**Parameters:**
- `count` — Number of stars to add (default: 1)
- `speed` — Horizontal speed 0–100 (default: 50)
- `color` — Hex color `0xRRGGBB` (default: `0xffc003`)
- `brightness` — Brightness 0–255 (default: 255)
- `size` — Trail size (default: 1)

**Example:**
```
!!MASTER:REQUEST:ADD_STAR_CENTER{count=5,speed=75,color=0xff0000,brightness=200,size=2}##
```

### BUILDUP_CLIMAX_CENTER

Gradually accelerate existing stars toward a climax moment.

**Parameters:**
- `duration` — Buildup duration in seconds (default: 10.0, max: 120)
- `speedMultiplier` — Speed acceleration factor (default: 5.0, range: 1.0–20.0)

**Example:**
```
!!MASTER:REQUEST:BUILDUP_CLIMAX_CENTER{duration=8.0,speedMultiplier=6.0}##
```

### START_CLIMAX_CENTER

Trigger the climax effect: stars spiral upward with fading brightness.

**Parameters:**
- `duration` — Climax duration in seconds (default: 15.0, max: 120)
- `spiralSpeed` — Vertical wobble speed in rows/sec (default: 0.5, range: 0.0–5.0)
- `speedMultiplier` — Horizontal speed boost (default: 5.0, range: 1.0–10.0)
- `verticalBias` — Vertical motion emphasis for wide displays (default: 1.2, min: 1.0)

**Example:**
```
!!MASTER:REQUEST:START_CLIMAX_CENTER{duration=12.0,spiralSpeed=0.8,speedMultiplier=7.0,verticalBias=1.5}##
```

### PING

Health check to keep connection alive (auto-responded).

## Project Structure

```
├── include/
│   ├── command_handler.h          # Command routing and dispatch
│   ├── config.h                   # Configuration constants
│   ├── octo_wrapper.h             # OctoWS2811 abstraction layer
│   ├── renderer.h                 # Pixel buffer & rendering
│   ├── stars.h                    # Star particle system
│   ├── mapping.h                  # Curtain index mapping
│   └── commands/
│       ├── base_command_handler.h # Command handler base class
│       ├── star_command_handler.h # Star spawning handler
│       └── climax_command_handler.h # Climax effect handler
├── lib/
│   ├── CmdLib.h                   # Command parsing library
│   └── PingPong.h                 # Ping/pong keep-alive handler
├── src/
│   ├── main.cpp                   # Main loop & initialization
│   ├── config.cpp                 # Configuration defaults
│   ├── command_handler.cpp        # Command processing
│   ├── octo_wrapper.cpp           # LED driver setup
│   ├── renderer.cpp               # Soft pixel rendering
│   ├── stars.cpp                  # Star animation logic
│   └── commands/
│       ├── star_command_handler.cpp
│       └── climax_command_handler.cpp
```

## How It Works

### Animation Pipeline

1. **Command Parsing** — Serial input parsed via CmdLib
2. **Command Dispatch** — Routed to appropriate handler
3. **Star Updates** — Position updated by velocity + effects
4. **Rendering** — Stars drawn to soft pixel buffer with blending
5. **Fade** — Entire buffer faded by fadeFactor
6. **Output** — Buffer copied to OctoWS2811 and displayed

### Climax Effects

**Buildup Phase:** Stars accelerate gradually over the specified duration (slower acceleration initially, reaching full speed by 70% mark).

**Climax/Spiral Phase:** Stars perform a time-synchronized vertical climb to the top of the curtains while:
- Maintaining horizontal motion
- Experiencing subtle sine-wave vertical wobble
- Fading in brightness as they reach the top
- Clearing completely when duration expires

## Getting Started

1. **Clone/download** the repository
2. **Configure** curtain dimensions in `include/config.h`
3. **Set pins** for OctoWS2811 in `src/config.cpp`
4. **Upload** to Teensy via Arduino IDE (requires Teensy support + OctoWS2811 library)
5. **Send commands** via serial terminal at 9600 baud

## Dependencies

- **Arduino Framework** (Teensy)
- [OctoWS2811](https://github.com/PaulStoffregen/OctoWS2811) — LED driver library
- CmdLib (included)
- PingPong (included)

## Debugging

Serial output for diagnostics:
```cpp
Serial.println("Registered handler: StarHandler");
Serial.println("ERROR: not enough RAM for pixBuf");
```

Monitor free memory (optional):
```cpp
int freeMemory();  // Returns estimated available RAM
```

## Performance Notes

- **Frame Time:** Configurable; default 1ms for ~50 FPS
- **Memory:** 500 stars + pixel buffer requires ~8KB RAM
- **Limitations:** OctoWS2811 supports up to 8 curtain strips per Teensy

## Future Enhancements

- Preset animation sequences
- Brightness/color ramping controls
- Multi-device synchronization
- Web interface for real-time tuning
- Pattern library expansion

## License

Open source — modify and extend as needed for your installation.
