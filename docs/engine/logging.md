# Racing Dynamite Engine Logging & Telemetry Subsystem

## 1. Overview & Architecture

To support reverse engineering, bug discovery, and runtime performance tracking, **Racing Dynamite** features a high-performance, low-overhead centralized logging and telemetry subsystem (`include/ignition/log.h`, `src/core/log.c`).

The system guarantees that diagnostic data is captured simultaneously in the console and on disk (`racing_dynamite.log`). Every write is immediately followed by `fflush()`, ensuring that all log entries prior to an abnormal termination or crash are preserved for analysis.

```
                    ┌─────────────────────────┐
                    │ Engine Subsystems       │
                    │ (Physics, Render, etc.) │
                    └───────────┬─────────────┘
                                │ LOG_* macros
                                ▼
                    ┌─────────────────────────┐
                    │ Log_Message() Dispatch  │
                    │   - Level Filtering     │
                    │   - [MM:SS.mmm] Timing  │
                    └───────┬─────────┬───────┘
                            │         │
               Console Mode │         │ File Mode (Immediate fflush)
                            ▼         ▼
                      ┌─────────┐ ┌───────────────┐
                      │ stdout/ │ │ Disk Log File │
                      │ stderr  │ │ (.log)        │
                      └─────────┘ └───────────────┘
```

---

## 2. Record Format Specification

Every log line follows a strict, fixed-width timestamped format:

```text
[MM:SS.mmm] [LEVEL] [CHANNEL] Message...
```

### Components:
* **`[MM:SS.mmm]`**: Elapsed session time in minutes, seconds, and milliseconds since engine startup (via monotonic high-resolution clock `GetTickCount64` on Windows or `clock_gettime(CLOCK_MONOTONIC)` on POSIX).
* **`[LEVEL]`**: 5-character padded level indicator:
  * `DEBUG`: Detailed in-race telemetry, surface raycast misses, per-wheel status.
  * `INFO `: Major state transitions, turbo activation, renderer toggles, startup/shutdown.
  * `WARN `: Non-critical anomalies, missing optional assets.
  * `ERROR`: Critical failures, missing required track/car geometry, SDL2 failure.
* **`[CHANNEL]`**: Subsystem domain identifier:
  * `[STATE]`: Game state machine transitions (Intro -> Menus -> Loading -> Race).
  * `[PHYSICS]`: Vehicle dynamics, landing impacts, gear shifts, turbo timer events, periodic telemetry.
  * `[SURFACE]`: `getsurf.c` raycasting, out-of-bounds queries, candidate polygon misses.
  * `[RENDER]`: Rasterizer, wireframe/textured modes, waypoint visualization toggles.
  * `[CAMERA]`: Camera mode shifts, chase camera alignment, orbit/pan updates.
  * `[SETTINGS]`: Options menu toggles, including the "Original Game Bugs" fixes switch.
  * `[ENGINE]`: Application lifecycle, frame pacing, shutdown sequence.
  * `[PLATFORM]`: SDL2 window creation, input event polling.

---

## 3. In-Race Detailed Telemetry

During active race simulation (`GAME_STATE_IN_RACE`), the logging subsystem produces two tiers of telemetry:

### 3.1. Periodic Telemetry (500 ms Interval)
Every 500 ms (approximately every 36 physics integration ticks), the active player car emits a comprehensive snapshot of its dynamics via `Vehicle_LogTelemetry()`:

```text
[00:00.515] [DEBUG] [PHYSICS] Tick #36 Car 0: pos=(919.1, 270.3, 2855.0) vel=(7.4, 0.0, 0.0) speed_long=7.4 yaw=0.0 deg pitch=0.0 deg roll=0.0 deg RPM=1473 gear=1 grounded=1 friction=0.30
```

#### Fields Logged:
| Field | Description |
| :--- | :--- |
| `Tick #N` | Cumulative fixed-timestep 72 Hz physics integration count. |
| `Car ID` | Archetype index (0: Coop, 1: Monster, 2: Beetle, 3: Bus, 4: Red Evader, 5: Turbo, 6: Buggy, 7: Police). |
| `pos=(X, Y, Z)` | World space coordinates in game units. |
| `vel=(Vx, Vy, Vz)` | 3D world velocity vector. |
| `speed_long` | Longitudinal forward chassis speed. |
| `yaw, pitch, roll` | Chassis Euler angles converted from radians to degrees. |
| `RPM` | Virtual engine rotational speed (1000 - 8000 RPM). |
| `gear` | Selected transmission gear (`R`, `1`, `2`). |
| `grounded` | Ground contact flag (`1` = at least one wheel on terrain, `0` = airborne). |
| `friction` | Evaluated contact friction coefficient ($0.10$ to $1.20$). |

### 3.2. Event-Driven Telemetry
Instantaneous events are logged immediately as state transitions occur:
* **Chassis Takeoff**:
  ```text
  [00:01.218] [DEBUG] [PHYSICS] Vehicle 0 TAKEOFF: speed=18.4 y=295.0 vy=-2.40
  ```
* **Chassis Touchdown / Landing Impact**:
  ```text
  [00:01.442] [DEBUG] [PHYSICS] Vehicle 0 TOUCHDOWN: speed=17.9 impact_vy=-6.20 ground_y=280.1
  ```
* **Transmission Gear Shift**:
  ```text
  [00:03.110] [DEBUG] [PHYSICS] Vehicle 0 GEAR SHIFT: 1st -> R (speed=-1.2, RPM=1100)
  ```
* **Turbo Boost Trigger & Expiry**:
  ```text
  [00:05.800] [INFO ] [PHYSICS] Vehicle 0 TURBO ACTIVATED (timer=200 ticks)
  [00:08.577] [DEBUG] [PHYSICS] Vehicle 0 TURBO EXPIRED
  ```
* **Surface Raycast Misses & Boundaries**:
  ```text
  [00:00.015] [DEBUG] [SURFACE] Raycast candidate miss, falling back to coarse SRF elevation: world=(-1154.6, -2672.4)
  [00:02.100] [DEBUG] [SURFACE] Raycast out of bounds: world=(32000.0, -15000.0) cell=(112, -2)
  ```

---

## 4. Command Line Configuration

The engine allows runtime customization of log output through CLI arguments:

```bash
# Set custom log file destination
racing_dynamite.exe --log logs/session_01.log

# Set minimum active log level (debug, info, warn, error)
racing_dynamite.exe --log-level info

# Combine with frame limit for automated testing
racing_dynamite.exe --frames 120 --log debug.log --log-level debug
```

---

## 5. C API Reference

```c
#include "ignition/log.h"

// Lifecycle
bool     Log_Init(const char *filepath, LogLevel min_level);
void     Log_Shutdown(void);

// Filtering & Output Destinations
void     Log_SetLevel(LogLevel level);
LogLevel Log_GetLevel(void);
void     Log_SetFileLogging(bool enable);
void     Log_SetConsoleLogging(bool enable);

// Formatted Output Macros
LOG_DEBUG(channel, format, ...);
LOG_INFO(channel, format, ...);
LOG_WARN(channel, format, ...);
LOG_ERROR(channel, format, ...);
```
