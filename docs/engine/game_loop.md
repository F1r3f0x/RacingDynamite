# Ignition Game Loop & State Machine

## 1. Top-Level Entry & Execution Flow

The Windows 95 executable initializes CRT startup at `entry` (`0x00469950`), acquires the module instance, and calls `WinMain` (`0x004120a0`).

```
+-------------------------------------------------------------+
|               entry (CRT Startup: 0x00469950)               |
+-------------------------------------------------------------+
                               |
                               v
+-------------------------------------------------------------+
|                     WinMain (0x004120a0)                    |
| - Registers "Ignition" window class                         |
| - Queries performance timer (QueryPerformanceFrequency)     |
| - App_Init (0x00412500): Creates window, DirectDraw/Input   |
+-------------------------------------------------------------+
                               |
                               v
               +-------------------------------+
               |      Message / Tick Loop      |
               | PeekMessage / GetMessage      |
               +-------------------------------+
                               | (No OS messages)
                               v
+-------------------------------------------------------------+
|                App_FrameTick (0x00412230)                   |
| - State 0: Game Init (0x00417270) -> switches to State 1    |
| - State 1: Active Game Dispatcher (0x004172b0)              |
| - State 2: Shutdown & Cleanup (0x00417e90)                  |
+-------------------------------------------------------------+
```

---

## 2. Main Game Dispatcher (`FUN_004172b0`)

The main game state machine is governed by global control flags in `FUN_004172b0`:

### Primary State Flags

| Variable | Address | Description |
| :--- | :--- | :--- |
| `g_GameStage` | `0x00493734` | Master game stage (`0` = Init, `1` = Active, `2` = Exit/Shutdown). |
| `g_IntroState` | `0x00639394` | Intro logos sequence (`2` = Virgin/UDS logos, `1` = Initialize menus, `0` = Done). |
| `g_MenuState` | `0x00563c3c` | Menu active flag (`1` = Rendering & processing menu inputs via `FUN_00402c00`). |
| `g_MenuSelected` | `0x00563d9c` | Menu item selected (`1` = Process menu selection via `FUN_004038f0`). |
| `g_RaceLoadStage`| `0x00553290` | Race loading sequence (`1` $\rightarrow$ `2` $\rightarrow$ `3` $\rightarrow$ `0`). |
| `g_InRace` | `0x00525e5c` | Active race simulation (`1` = In-race vehicle simulation & 3D rendering). |

---

## 3. State Machine Transitions

```
[Start Engine]
      |
      v
g_GameStage = 0  --> FUN_00417270() initializes defaults, sets g_IntroState = 2
      |
      v
g_IntroState = 2 --> FUN_004182d0() displays Virgin & UDS company logos
      |
      v
g_IntroState = 1 --> FUN_004029a0() initializes menu data, sets g_MenuState = 1
      |
      v
g_MenuState = 1  --> FUN_00402c00() runs interactive menus (single race, championship, options)
      |
      |-- (User selects Race)
      v
g_RaceLoadStage = 1 --> FUN_00418b70() loads car models & sound pools
      |
      v
g_RaceLoadStage = 2 --> FUN_00418be0() loads track meshes (.MSH, .TRI, .TEX, .TAB)
      |
      v
g_RaceLoadStage = 3 --> FUN_00418c40() loads surface physics (.SRF), AI waypoints (.POS)
      |
      v
g_InRace = 1        --> In-race simulation loop:
                        1. FUN_00420c00(): Timestep / physics delta
                        2. Vehicle dynamics & wheel collision via getsurf
                        3. AI opponent logic
                        4. 3D Camera update
                        5. Lisa3D software rasterization to framebuffer
```
