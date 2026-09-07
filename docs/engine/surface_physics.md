# Authentic Vehicle Physics & Surface Raycasting Subsystem (`getsurf.c`)

This document details the reverse-engineered vehicle dynamics, 4-wheel independent raycast suspension, terrain collision, and surface query subsystems from **Ignition** (1997, Unique Development Studios / Virgin Interactive), reconstructed from `IGN_WIN.EXE` (`0x00412fc0`, `0x00424570`, `0x0040e6b0`, `0x00423aa0`) and `MAINDOS.EXE` (`getsurf.c`, `lisa3d.c`).

---

## 1. Subsystem Overview & Architectural Timing

Ignition decouples the physics simulation from the graphical presentation rate. While the Lisa3D software rasterizer runs at variable frame rates (30–60 FPS), the vehicle dynamics engine ticks at a strict, deterministic **72 Hz** fixed timestep:

$$\Delta t = \frac{1}{72}\text{ seconds} \approx 0.013888889\text{ s}$$

This tick rate is anchored to the global constant at `0x004792f0`. The coordinate integration uses an authentic scaling factor of `21.76` (`0x00479af0`):

$$\vec{P}_{t+1} = \vec{P}_t + \vec{V} \cdot \Delta t \cdot 21.76$$

```
+---------------------------------------------------------------------------------+
|                                 Game_Update                                     |
|                       (Accumulates real frame delta)                            |
+---------------------------------------+-----------------------------------------+
                                        |
                   physics_accumulator += delta_time
                                        |
            +---------------------------v---------------------------+
            |  While (physics_accumulator >= 1.0 / 72.0):          |
            |                                                       |
            |  1. Rotate 4 wheel offsets by chassis yaw             |
            |  2. Surface_Raycast() 4 wheels against .SRF terrain   |
            |  3. Compute axle heights -> Pitch & Roll tilt         |
            |  4. Vertical Dynamics: gravity -0.2 / tick, landing   |
            |  5. Bicycle Model: steering attenuation & yaw rate    |
            |  6. Powertrain: engine torque, drag, reverse gear     |
            |  7. World velocity & position integration             |
            +---------------------------+---------------------------+
                                        |
            +---------------------------v---------------------------+
            |  Vehicle_GetChaseCamera() -> Smooth 3rd-person cam    |
            |  Renderer_DrawTrackMesh() + Renderer_DrawCar()        |
            +-------------------------------------------------------+
```

---

## 2. Authentic `.SRF` Spatial Raycasting (`Surface_Raycast`, `getsurf.c`)

Surface elevation and normal querying is performed on the track's binary `.SRF` (Surface) data. The algorithm matches `FUN_00412fc0` in `IGN_WIN.EXE` (`getsurf.c`).

### 2.1 World Coordinate Offset & Grid Hashing

Ignition track models are centered at $(0, 0)$ and span $\pm 25,600$ units in $X$ and $Z$. The spatial grid uses 512-unit cell dimensions with an origin offset of 50 cells (`0x00479c48`):

$$X_{\text{raw}} = X_{\text{world}} + 25,600$$

$$Z_{\text{raw}} = Z_{\text{world}} + 25,600$$

$$c_x = \left\lfloor \frac{X_{\text{raw}}}{512} \right\rfloor = \left\lfloor \frac{X_{\text{world}}}{512} \right\rfloor + 50$$

$$c_z = \left\lfloor \frac{Z_{\text{raw}}}{512} \right\rfloor = \left\lfloor \frac{Z_{\text{world}}}{512} \right\rfloor + 50$$

Cells outside $c_x \in [0, \text{stride}_x)$ or $c_z \in [0, \text{stride}_z)$ immediately return off-track water/kill plane status (`elevation = ref_y`, `is_kill_plane = true`).

### 2.2 Candidate Traversal: `table2` and `table1`

Each cell references two index arrays, read from the `SrfCell` struct (confirmed by `FUN_00412fc0` decompilation: `*(ushort*)(cell + 8)` and `*(ushort*)(cell + 10)`):
1. `table2` (`cell->table2_offset`, count `cell->table2_count` at offset `+0x0A`): Triangles with positive height spans ($dz \ge 0$). Tested via `FUN_00446578`.
2. `table1` (`cell->table1_offset`, count `cell->table1_count` at offset `+0x08`): Triangles with negative height spans ($dz < 0$). Tested via `FUN_004465e1`.

Each 32-bit entry in these tables is a byte offset into the 24-byte `SrfTriangle` array:

$$\text{tri\_idx} = \frac{\text{offset}}{24}$$

### 2.3 2D Trapezoid & Slope Span Test

`SrfTriangle` encodes 2D trapezoidal bounds via 16.16 fixed-point slopes:

```c
typedef struct {
    int32_t x_base;         // Base apex X
    int32_t z_base;         // Base apex Z
    int32_t slope1;         // 16.16 fixed-point dx1/dz
    int32_t slope2;         // 16.16 fixed-point dx2/dz
    int32_t flags_and_dz;   // Low 16 bits = dz (int16_t), High 16 bits = polygon dword offset
    int32_t v_ptr;          // Byte offset of placed object in .PLC array
} SrfTriangle;
```

A point $(q_x, q_z)$ lies within the candidate triangle if:
- For $dz \ge 0$: $q_z \in [z_{\text{base}}, z_{\text{base}} + dz]$
- For $dz < 0$: $q_z \in [z_{\text{base}} + dz, z_{\text{base}}]$

And along the horizontal span:

$$x_1 = x_{\text{base}} + \frac{(q_z - z_{\text{base}}) \cdot \text{slope}_1}{65536}$$

$$x_2 = x_{\text{base}} + \frac{(q_z - z_{\text{base}}) \cdot \text{slope}_2}{65536}$$

$$q_x \in [\min(x_1, x_2), \max(x_1, x_2)]$$

### 2.4 Placed Object Resolution & Geometry Filtering

In `.SRF`, the placed object reference `normal_z` stores the byte offset into Lisa3D's internal registered scene object table (`DAT_0063c5dc`):

$$\text{obj\_index} = \frac{\text{normal\_z}}{42}$$

The track placement table (`.PLC`) contains both physical 3D mesh instances (road chunks, terrain, buildings) and non-mesh gameplay markers. During scene registration in `IGN_WIN.EXE` (`0x00418c40` / `0x0041b6af`), non-mesh marker archetypes are excluded from the 42-byte object pool while preserving original `.PLC` sequential order:
* **Checkpoints & split-time triggers**: Archetypes 150..199 (`model_type & 0xff == 0x9a`).
* **Camera trigger zones**: Archetypes 200..299 (`model_type & 0xff == 0xc8`).
* **Dynamic obstacle volumes & triggers**: Archetypes 300..399 (`model_type & 0xff in {0x62, 0x2c}`).
* **Starting grid spawn positions**: Archetypes 400..499 (`model_type & 0xff in 0x90..0x98`).

When filtered via `IsGeometryMeshObject()`, **100.00% of all collision triangles across all 7 circuits** resolve to valid 3D polygon vertices:
* **Austria**: 12,462 / 12,462 triangles (100.00% valid)
* **Brazil**: 11,074 / 11,074 triangles (100.00% valid)
* **Canada**: 8,752 / 8,752 triangles (100.00% valid)
* **Carib**: 15,126 / 15,126 triangles (100.00% valid)
* **Iceland**: 9,266 / 9,266 triangles (100.00% valid)
* **Japan**: 12,427 / 12,427 triangles (100.00% valid)
* **USA**: 9,685 / 9,685 triangles (100.00% valid)

### 2.5 Vertex Transformation & Normal Calculation

When a 2D hit is found, the 3D world vertices $V_0, V_1, V_2$ are resolved from the placed object in `.PLC` and submesh in `.MSH`:

$$\vec{V}_{i,\text{world}} = \begin{pmatrix} \text{obj}_x + \text{mesh\_vertex}_{i,x} \\ \text{obj}_y - \text{mesh\_vertex}_{i,y} \\ \text{obj}_z + \text{mesh\_vertex}_{i,z} \end{pmatrix}$$

The triangle face normal is computed via the cross product and normalized to unit length:

$$\vec{N} = (V_0 - V_1) \times (V_2 - V_1)$$

$$\hat{N} = \frac{\vec{N}}{\|\vec{N}\|}$$

To ensure consistent upward orientation for terrain collision, if $\hat{N}_y < 0$, the normal vector is inverted ($\hat{N} \leftarrow -\hat{N}$) so that $\hat{N}_y \ge 0$.

### 2.6 Authentic 3D Plane Height Equation & Near-Vertical Division Protection

The exact vertical surface elevation $Y$ directly below $(X, Z)$ is computed using the point-normal plane formula from `0x00412fc0`:

$$Y = V_{0,y} + \frac{(V_{1,y} - V_{0,y})\hat{N}_y + \hat{N}_x(V_{1,x} - X) + \hat{N}_z(V_{1,z} - Z)}{\hat{N}_y}$$

* **Vertical Wall Division Protection ($\hat{N}_y < 0.20$)**: When $\hat{N}_y \to 0$ (near-vertical cliff faces or walls where $\text{slope} > 78^\circ$), dividing by $\hat{N}_y$ causes numerical explosion ($Y \to \infty$). In this regime, the engine falls back to the average triangle vertex height:
  $$Y_{\text{fallback}} = \frac{V_{0,y} + V_{1,y} + V_{2,y}}{3}$$
* **Candidate Selection & Road Surface Prioritization**: If multiple overlapping candidate triangles exist at the coordinate (e.g. overpasses, tunnels, or track edges adjacent to vertical mountain walls), candidates with steep normal ($\hat{N}_y < 0.25$, $\text{slope} > 75.5^\circ$) or barrier materials ($\text{material\_id} \ge 80$) receive a $+2,000.0$ candidate distance penalty. This guarantees that drivable road surfaces are always prioritized over steep mountain walls.

---

### 2.7 Mountain Wall Collision & Step Climb Rejection (`IGN_WIN.EXE 0x00424570`)

In original assembly at `0x00424570` line 590:
```c
dVar4 = veh->speed_long * c1 + c2; // Maximum climbable step height
if ((dVar4 < (double)step_height) || (0x4f < (int)material_id)) {
    wheel_y = veh->y - veh->ride_height;
    wall_hit = 1;
}
```

1. **Barrier & Wall Identification**:
   * **Boundary Barriers**: Any polygon with `material_id > 79` (`material_id >= 80`) is classified as an impassable barrier.
   * **Steep Mountain Cliffs**: Any surface with $\hat{N}_y < 0.25$ ($\text{slope} > 75.5^\circ$) is classified as an unclimbable rock wall.
   * **Step Climb Threshold**: For grounded wheels, the height jump in a single tick is bounded by:
     $$\Delta Y_{\text{max}} = 0.5 \cdot |V_{\text{long}}| + 25.0\text{ units}$$
     If $\Delta Y > \Delta Y_{\text{max}}$, the step is too steep to climb.
2. **Collision Response**:
   * Wheel contact elevation is clamped to vehicle elevation: $y_{\text{contact}} = Y_{\text{car}} - h_{\text{ride}}$, preventing the suspension from riding up the cliff into the sky.
   * Wheel grounding is disabled (`grounded = false`).
   * **Propulsion Halt**: If the front wheels hit a wall while moving forward ($V_{\text{long}} > 0$), forward propulsion is halted ($V_{\text{long}} = 0, V_x = 0, V_z = 0$). If the rear wheels hit an obstacle while reversing ($V_{\text{long}} < 0$), reverse propulsion is halted ($V_{\text{long}} = 0$). This prevents vehicles from scaling vertical mountain walls.

## 3. 4-Wheel Independent Raycast Suspension

Each vehicle archetype defines local wheel contact offsets $(\pm \frac{1}{2} w_{\text{track}}, +a, -b)$ relative to the center of mass.

### 3.1 World Coordinate Wheel Transformation

At every physics tick, the 4 wheel contact coordinates are rotated by the chassis heading $\psi$:

$$\begin{pmatrix} X_{\text{wheel}} \\ Z_{\text{wheel}} \end{pmatrix} = \begin{pmatrix} X_{\text{car}} \\ Z_{\text{car}} \end{pmatrix} + \begin{pmatrix} \cos\psi & \sin\psi \\ -\sin\psi & \cos\psi \end{pmatrix} \begin{pmatrix} \Delta x_{\text{local}} \\ \Delta z_{\text{local}} \end{pmatrix} \cdot S_{\text{chassis}} \cdot 21.76$$

Each wheel then performs an independent `Surface_Raycast` to discover the exact terrain elevation and friction coefficient directly beneath that tire.

### 3.2 Suspension Contact & Ride Height Equilibrium

A wheel is considered grounded when:

$$Y_{\text{wheel}} \in [Y_{\text{contact}} - 2.0, Y_{\text{contact}} + 25.0]$$

The equilibrium chassis ride height is $+5.0$ units above average wheel ground contact (`0x00479b58`):

$$Y_{\text{target}} = \bar{Y}_{\text{ground}} + 5.0$$

---

## 4. Chassis Pitch & Roll Orientation Angles

The vehicle chassis tilts dynamically to match the local grade and bank of the track.

### 4.1 Axle Elevation Deltas

$$\bar{Y}_{\text{front}} = \frac{Y_{\text{FL}} + Y_{\text{FR}}}{2}, \quad \bar{Y}_{\text{rear}} = \frac{Y_{\text{RL}} + Y_{\text{RR}}}{2}$$

$$\bar{Y}_{\text{left}} = \frac{Y_{\text{FL}} + Y_{\text{RL}}}{2}, \quad \bar{Y}_{\text{right}} = \frac{Y_{\text{FR}} + Y_{\text{RR}}}{2}$$

### 4.2 Arcsine Clamping & Rate Limiting

The target angles are computed from axle height deltas divided by wheelbase and track width, clamped to $\pm 0.95$ (`0x00479ca0`):

$$\theta_{\text{target}} = \arcsin\left(\text{clamp}\left(\frac{\bar{Y}_{\text{front}} - \bar{Y}_{\text{rear}}}{L_{\text{wheelbase}} \cdot 21.76}, -0.95, 0.95\right)\right)$$

$$\phi_{\text{target}} = \arcsin\left(\text{clamp}\left(\frac{\bar{Y}_{\text{left}} - \bar{Y}_{\text{right}}}{W_{\text{track}} \cdot 21.76}, -0.95, 0.95\right)\right)$$

Angular rates are strictly limited to **$\pm 0.1\text{ rad/tick}$** (`0x00479cb0`) to prevent visual jitter over rough road geometry:

$$\Delta\theta = \text{clamp}(\theta_{\text{target}} - \theta, -0.1, 0.1), \quad \theta_{t+1} = \theta_t + \Delta\theta$$

$$\Delta\phi = \text{clamp}(\phi_{\text{target}} - \phi, -0.1, 0.1), \quad \phi_{t+1} = \phi_t + \Delta\phi$$

---

## 5. Bicycle Handling Model & Steering Dynamics

Vehicle steering combines a bicycle kinematic yaw rate model with high-speed steering attenuation.

### 5.1 Speed-Attenuated Steering Lock

At higher speeds, front wheel lock is automatically attenuated to maintain vehicle stability:

$$A_{\text{steer}} = \text{clamp}\left(1.0 - \frac{|V_{\text{long}}|}{240.0}, 0.35, 1.0\right)$$

$$\delta_{\text{target}} = u_{\text{steer}} \cdot \delta_{\text{max}} \cdot A_{\text{steer}}$$

$$\delta_{t+1} = \delta_t + (\delta_{\text{target}} - \delta_t) \cdot 12.0 \cdot \Delta t$$

### 5.2 Kinematic Yaw Rate & Angular Acceleration

$$\dot{\psi}_{\text{target}} = \frac{\tan\delta}{a + b} \cdot V_{\text{long}}$$

$$\ddot{\psi}_{\text{max}} = 6.0 \cdot \mu_{\text{friction}}$$

$$\Delta\dot{\psi} = \text{clamp}\left(\dot{\psi}_{\text{target}} - \dot{\psi}, -\ddot{\psi}_{\text{max}} \cdot 60 \Delta t, \ddot{\psi}_{\text{max}} \cdot 60 \Delta t\right)$$

### 5.3 Steering Polarity & Lisa3D Coordinate Parity
In the Lisa3D camera coordinate system (`right = forward x world_up`), world $+X$ projects to Screen Left and world $-X$ projects to Screen Right.
To ensure intuitive driving controls:
* **Steering Left (`input->key_left`)**: Maps to `steer = +1.0`, producing positive $\delta_{\text{target}}$ and increasing heading yaw $\psi$. This directs longitudinal velocity into world $+X$ ($\Delta X > 0$), steering the car towards the left of the screen.
* **Steering Right (`input->key_right`)**: Maps to `steer = -1.0`, producing negative $\delta_{\text{target}}$ and decreasing heading yaw $\psi$. This directs longitudinal velocity into world $-X$ ($\Delta X < 0$), steering the car towards the right of the screen.

---

## 6. Powertrain, Drag & Transmission

### 6.1 Propulsion & Gear Selection
- **Forward Gear (`gear == 1`)**: Throttle applies forward thrust proportional to engine power and surface friction. If the vehicle is stopped ($|V| < 1.5$) and brake is held ($u_{\text{brake}} > 0.4$), the transmission automatically shifts to **Reverse Gear (`gear == 0`)**.
- **Reverse Gear (`gear == 0`)**: Holding brake applies reverse thrust ($45\%$ of engine power). Pressing throttle shifts back to 1st gear.

### 6.2 Aerodynamic & Rolling Drag
Longitudinal motion is resisted by quadratic aerodynamic drag and linear rolling resistance:

$$F_{\text{drag}} = 0.40 \cdot V_{\text{long}} |V_{\text{long}}| + 25.0 \cdot V_{\text{long}}$$

$$F_{\text{slope}} = m \cdot g \cdot \sin\theta$$

$$a_{\text{long}} = \frac{F_{\text{thrust}} - F_{\text{brake}} - F_{\text{drag}} - F_{\text{slope}}}{m}$$

### 6.3 Turbo Boost Mechanics
- Manual activation (`key_turbo` / Spacebar) or driving over a yellow accelerator boost pad (`material_id == 3`) activates turbo boost for **200 ticks** (~2.8 seconds).
- Forward thrust is boosted by a **$1.75\times$** multiplier (`boost_mult = 1.75`).

---

## 7. Chase Camera Tracking Algorithm

In-race third-person view tracks smoothly behind the car:
- **Target**: Follows vehicle center of mass: $\vec{T} = (X_{\text{car}}, Y_{\text{car}} + 5.0, Z_{\text{car}})$.
- **Yaw Smoothing**: Camera yaw exponentially tracks vehicle heading with a $15\%$ per-frame lag ($0.15\text{ factor}$).
- **Terrain Incline Tilt**: Pitch tilts with the vehicle's pitch plus a $24^\circ$ base chase elevation:

$$\text{Pitch}_{\text{cam}} = 24^\circ + 0.35 \cdot \theta_{\text{car}}$$

- **Chase Distance**: Fixed at $220.0$ units behind the center of mass (proportional to the authentic $66\text{-unit}$ chassis length).
- **Controls**: Pressing `C` resets the camera to track origin for free inspection (WASD/Orbit); touching any driving control (Gas, Brake, Steer, Turbo) automatically snaps back to chase camera.

---

## 8. Master Binary Constants & References

| Constant | Value | Original Binary Address | Description |
| :--- | :--- | :--- | :--- |
| `PHYSICS_DT_SEC` | $1/72\text{ s}$ (`0.013888889`) | `0x004792f0` | Fixed simulation tick rate (72 Hz). |
| `PHYSICS_SCALE_FACTOR` | `21.76` | `0x00479af0` | World velocity integration multiplier. |
| `PHYSICS_GRAVITY` | `9.81` | `0x004792c8` | Gravitational constant ($m/s^2$). |
| `PHYSICS_GRAVITY_TICK` | `0.2` | `0x00479b50` | Vertical downward velocity delta per tick. |
| `PHYSICS_RIDE_HEIGHT` | `5.0` | `0x00479b58` | Ground clearance equilibrium offset. |
| `PHYSICS_MAX_SLOPE_SIN` | `0.95` | `0x00479ca0` | Arcsin clamping limit for pitch/roll angles. |
| `PHYSICS_RATE_LIMIT_RAD` | `0.1` | `0x00479cb0` | Max pitch/roll rate of change per tick. |
| `PHYSICS_WORLD_OFFSET` | `25600.0` | `0x00479c48` | World origin centering offset ($50 \times 512$). |
