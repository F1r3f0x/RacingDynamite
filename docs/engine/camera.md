# Dynamic Chase Camera & Viewport Subsystem (`camera.c`)

This document specifies the reverse-engineered dynamic chase camera, projection transforms, orientation lag filters, and multi-mode viewports of **Ignition** (1997, Unique Development Studios / Virgin Interactive), reconstructed from `MAINDOS.EXE` (`0x0043c910`, `0x00436990`) and `MAINDOS.EXE` (`main.c`, `lisa3d.c`).

---

## 1. Subsystem Overview & Architectural Timing

Ignition's signature visual presentation is characterized by its high-angle isometric chase camera. Unlike static cameras that lock rigidly behind a vehicle, Ignition's camera behaves like an airborne boom-mounted tracking camera that:
1. Projects its focal center ahead along the vehicle's instantaneous velocity vector (**Dynamic Lookahead**).
2. Damps azimuth yaw rotations using an authentic exponential spring-damper filter (**Azimuth Lag**).
3. Adapts pitch elevation to hill gradients and expands altitude during airborne jumps (**Terrain Slope Adaptation**).
4. Provides 5 distinct viewpoint modes tailored for gameplay, tactical track inspection, and debugging.

```
+------------------------------------------------------------------------------------+
|                                 Physics / Vehicle State                            |
|                     Position (X,Y,Z), Velocity, Heading, Pitch, Roll               |
+-----------------------------------------+------------------------------------------+
                                          |
                                          v
+------------------------------------------------------------------------------------+
|               Camera_UpdateFollowChase (src/renderer/camera.c)                     |
|                                                                                    |
|  1. Velocity Lookahead:                                                            |
|     target = car_pos + clamp(speed * 3.5, -60, +180) * (sin(yaw), 0, cos(yaw))     |
|                                                                                    |
|  2. Vertical Focal Offset:                                                         |
|     target.y = car_pos.y + (is_bumper ? 12.0 : 40.0)                               |
|                                                                                    |
|  3. Azimuth Shortest-Path Lag:                                                     |
|     yaw_diff = wrap_180(target_yaw - cam.yaw)                                      |
|     cam.yaw += yaw_diff * lag_factor (0.125 authentic)                             |
|                                                                                    |
|  4. Terrain Pitch Adaptation:                                                      |
|     cam.pitch = base_pitch + terrain_pitch * 0.35                                  |
|                                                                                    |
|  5. Spherical to Cartesian Conversion (DEV-004):                                   |
|     pos = target + distance * (cos(pitch)*sin(yaw), sin(pitch), cos(pitch)*cos(yaw))|
+-----------------------------------------+------------------------------------------+
                                          |
                                          v
+------------------------------------------------------------------------------------+
|                      Lisa3D Software Rasterizer Viewport                           |
|             World-to-View Transform -> Backface Culling -> Projection              |
+------------------------------------------------------------------------------------+
```

---

## 2. Camera View Modes (`CameraMode`)

Racing Dynamite provides 5 selectable camera viewing modes:

| Mode | Distance | Pitch Angle | FOV | Lag ($\alpha$) | Description |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **`CAMERA_MODE_CLASSIC`** *(Default)* | **1100.0** | **$32^\circ$** | **$60^\circ$** | **0.125** | Authentic 1997 default isometric chase perspective framing the car and immediate upcoming corners. |
| **`CAMERA_MODE_CLOSE`** | **650.0** | **$24^\circ$** | **$65^\circ$** | **0.200** | Intimate, low-slung chase perspective with heightened sense of speed. |
| **`CAMERA_MODE_FAR`** | **1900.0** | **$40^\circ$** | **$55^\circ$** | **0.080** | High-altitude tactical overview advantageous for spotting sharp hairpin turns. |
| **`CAMERA_MODE_BUMPER`** | **50.0** | **$5^\circ$** | **$75^\circ$** | **0.950** | Low hood / front bumper view with near-instantaneous heading response. |
| **`CAMERA_MODE_FREE_ORBIT`** | **3000.0** | **$35^\circ$** | **$60^\circ$** | **1.000** | Spectator / track inspection camera with manual WASD pan and arrow key orbit. |

### Mode Controls
* In race: Press `C` to cycle through camera modes (`Classic` $\to$ `Close` $\to$ `Far` $\to$ `Bumper` $\to$ `Classic`).
* Press `Home` or `R` to reset the camera to the track origin.

---

## 3. Dynamic Velocity Lookahead Algorithm

Rather than focusing directly on the center of the vehicle chassis, the camera projects its target focal point ahead along the car's direction of motion:

$$\vec{d}_{\text{lookahead}} = \text{clamp}(V_{\text{long}} \cdot 3.5, -60.0, +180.0)$$

$$\vec{T}_{\text{target}} = \begin{pmatrix} X_{\text{car}} + \sin(\theta_{\text{yaw}}) \cdot \vec{d}_{\text{lookahead}} \\ Y_{\text{car}} + H_{\text{offset}} \\ Z_{\text{car}} + \cos(\theta_{\text{yaw}}) \cdot \vec{d}_{\text{lookahead}} \end{pmatrix}$$

Where:
* $H_{\text{offset}} = +12.0$ units in Bumper mode, $+40.0$ units in chase modes.
* The velocity lead allows the player to see oncoming track curves and obstacles before the vehicle rotates into the turn.
* Clamping prevents the target point from separating excessively during turbo boost bursts.

---

## 4. Azimuth Lag Damping & Shortest-Path Wrapping

The camera smoothly tracks the vehicle's yaw $\theta_{\text{target}}$ using an authentic exponential spring-damper filter:

$$\Delta \theta = \text{atan2}(\sin(\theta_{\text{target}} - \theta_{\text{cam}}), \cos(\theta_{\text{target}} - \theta_{\text{cam}}))$$

$$\theta_{\text{cam}, t+1} = \theta_{\text{cam}, t} + \Delta \theta \cdot \alpha$$

### 4.1 Authentic Lag Constant ($\alpha = 0.125$)
In `MAINDOS.EXE` at global memory location `_DAT_0047a720` (`.rdata` file offset `0x79720`), the camera orientation filter constant reads as a **`double`**:

$$\text{DAT\_0047a720} = -0.125 = -\frac{1}{8}$$

The **negative** sign reflects the angular difference convention: the shortest-path $\Delta\theta = \text{atan2}(\sin(\theta_\text{target} - \theta_\text{cam}), \cos(\theta_\text{target} - \theta_\text{cam}))$ is positive when the target leads the camera, so multiplying by $-0.125$ and subtracting moves the camera toward the target. The effective damping magnitude is $|\alpha| = 0.125$.

**Nearby confirmed binary constants** (verified from `.rdata` at `0x0047a700`–`0x0047a728`):

| Address | Value | Usage |
| :--- | :--- | :--- |
| `0x0047a700` | `150.0` | Lookahead clamp maximum (see §3). |
| `0x0047a708` | `0.0333…` ($\approx 1/30$) | Secondary interpolation step (lookahead smoothing). |
| `0x0047a710` | `6.2832` ($2\pi$) | Full-circle wrap constant. |
| `0x0047a718` | `-3.1416` ($-\pi$) | Half-circle boundary for shortest-path wrap. |
| `0x0047a720` | **`-0.125`** ($-1/8$) | **Azimuth lag damping factor** ✅ |

This produces the characteristic delayed tail-slide effect when drifting around tight corners, as the vehicle body turns inward before the camera swings around.


### 4.2 Shortest-Path Angle Wrapping
Direct difference subtraction $(\theta_{\text{target}} - \theta_{\text{cam}})$ can result in $359^\circ$ spinning when the car crosses the North azimuth threshold ($0^\circ \leftrightarrow 360^\circ$). The $\text{atan2}$ wrapping formulation guarantees that the camera always interpolates along the shortest angular arc across $[-\pi, +\pi]$.

---

## 5. Terrain Slope & Jump Elevation Adaptation

### 5.1 Hill Inclination Pitch Adaptation
When driving up steep mountains (e.g. Moose Jaw Falls, Austria) or down canyon drops (Caldera Peak, Iceland), keeping a constant pitch angle causes the road surface to obscure the horizon or drop out of view:

$$\beta_{\text{terrain}} = \theta_{\text{car pitch}} \cdot \frac{180}{\pi}$$

$$\theta_{\text{cam pitch}} = \theta_{\text{base pitch}} + \beta_{\text{terrain}} \cdot 0.35$$

$$\text{clamp}(\theta_{\text{cam pitch}}, 10.0^\circ, 85.0^\circ)$$

This tilts the camera upward on ascents and downward on descents, keeping the car and upcoming roadway centered.

### 5.2 Airborne Jump Altitude Expansion
When the car becomes airborne ($Y_{\text{car}} - Y_{\text{ground}} > 100$), the camera focal height smoothly rises, preventing ground clipping and framing both the airborne car and the landing zone below.

---

## 6. Mathematical Coordinate Basis Alignment (`DEV-004`)

### Original Behavior (`MAINDOS.EXE` @ `0x00436990`)
The original 1997 engine used an ad-hoc software projection matrix tailored to DirectDraw's top-left origin with inverted Z depth conventions. When pitching downward past vertical ($-90^\circ$), the viewport flipped upside down due to unhandled gimbal singularities.

### Source Port Implementation
Racing Dynamite constructs an orthonormal right-handed camera coordinate frame:

$$\vec{F} = \frac{\vec{T} - \vec{P}}{\|\vec{T} - \vec{P}\|}$$

$$\vec{R} = \frac{\vec{F} \times \vec{U}_{\text{world}}}{\|\vec{F} \times \vec{U}_{\text{world}}\|}$$

$$\vec{U}_{\text{cam}} = \vec{R} \times \vec{F}$$

This eliminates gimbal flips and guarantees correct winding order across all viewing angles. Toggleable via `GameFixOptions.fix_camera` (`DEV-004`).
