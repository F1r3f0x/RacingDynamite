# Lisa3D Software Rasterizer Architecture

## 1. Overview

Ignition (1997) utilizes an in-house proprietary software rasterization engine named **"Lisa 2 Development System"** (referenced in source strings and decompiled symbols as `Lisa_Init` at `0x0043e2a0` and `Lisa_PrintVersion` at `0x0045b4f0`).

The rasterizer operates exclusively on **8-bit indexed color framebuffers** (`640x480` and `320x200`), pairing a 16/32-bit depth buffer (Z-buffer) with Gouraud shading and affine/perspective texture mapping using a 64KB lighting lookup table (`.TAB`).

---

## 2. Pipeline Overview

```
 [3D Mesh Geometry (.MSH, .SRF, .PLC)]
                   |
                   v
       [World Transformation]
 (Translation by .PLC position, Car Physics position)
                   |
                   v
    [Camera View Matrix Transform]
 (Forward, Right, Up vectors from Orbit/Chase camera)
                   |
                   v
     [Near/Far Plane Z-Clipping]
                   |
                   v
    [Perspective Projection to 2D]
 x_s = (x_c * scale) / z_c + (width / 2)
 y_s = -(y_c * scale) / z_c + (height / 2)
                   |
                   v
   [2D Backface & Degenerate Culling]
  Signed 2D area (Y down) >= -0.1 -> Cull
  (Discards backfaces & prevents Z-fighting on double-sided signs/billboards)
                    |
                    v
  [Barycentric Triangle Span Rasterizer]
   - Sub-pixel conservative rasterization
   - Z-buffer depth test (z < zbuffer[pixel])
   - UV texture mapping from 1024x1024 atlas (.TEX)
   - Gouraud lighting table lookup (.TAB)
                    |
                    v
   [8bpp Virtual Framebuffer Presentation]
  (Blitted via SDL2 streaming ARGB32 hardware texture)
```

## 2.1 Backface Culling & Double-Sided Mesh Architecture

In original Ignition (`MAINDOS.EXE` / Lisa3D rasterizer at `Lisa_DrawTriangle_OpcodeHelper` `0x0044cb20`), every triangle undergoes 2D screen-space backface culling:

$$\text{area} = (x_1 - x_0)(y_2 - y_0) - (y_1 - y_0)(x_2 - x_0)$$

Because screen space has $+Y$ pointing downward, front-facing triangles have a **negative signed area** ($\text{area} < -0.1$). Triangles with $\text{area} \ge -0.1$ are back-facing or degenerate and are discarded immediately before rasterization.

### Double-Sided Signs & Billboards:
Across all circuits (`AUSTRIA`, `BRAZIL`, `CANADA`, etc.), roadside signs, track billboards, banners, and fences are constructed as thin coplanar polygon pairs sharing identical 3D vertex positions:
- **Front Polygon**: Front-facing from the approaching track direction (e.g. Moose sign on page 10 in Canada, speed arrows, sponsor boards), with winding producing $\text{area} < -0.1$.
- **Back Polygon**: Co-planar with reversed vertex winding, referencing a dark structural backing texture (e.g. vertical wood planks on page 8 in Canada), producing $\text{area} > 0$.

If backface culling is absent or improperly swaps backfaces, both the front and back triangles are rendered into the same pixels at the exact same depth ($1/Z$). Sub-pixel interpolation floating-point variations cause the dark backing texture to fight with the front texture, resulting in visible dark/black stipple dots across the sign. Strict culling of $\text{area} \ge -0.1$ eliminates this Z-fighting completely and renders all signs crisp and clean.

---

## 3. Mathematical Foundations

### 3.1 View Matrix & Coordinate Space
* **Coordinate System**: Right-handed world coordinates.
  * $X$: East / West
  * $Y$: Elevation (positive is UP, negative is DOWN into valleys/canyons)
  * $Z$: North / South
* **Camera Vectors**:
  $$\vec{F} = \text{normalize}(\vec{T} - \vec{C})$$
  $$\vec{R} = \text{normalize}(\vec{F} \times \vec{U}_{\text{world}})$$
  $$\vec{U} = \vec{R} \times \vec{F}$$
* **Camera-Space Transform**:
  $$x_c = (\vec{P} - \vec{C}) \cdot \vec{R}$$
  $$y_c = (\vec{P} - \vec{C}) \cdot \vec{U}$$
  $$z_c = (\vec{P} - \vec{C}) \cdot \vec{F}$$

### 3.2 Perspective Projection
Given vertical Field of View $\theta$:
$$\text{scale} = \frac{\text{height} / 2}{\tan(\theta / 2)}$$
$$x_{\text{screen}} = \frac{x_c \cdot \text{scale}}{z_c} + \frac{\text{width}}{2}$$
$$y_{\text{screen}} = -\frac{y_c \cdot \text{scale}}{z_c} + \frac{\text{height}}{2}$$

### 3.3 Triangle Rasterization & Perspective-Correct Interpolation

In screen space, linear interpolation of world quantities ($Z, U, V, L$) directly across pixels is mathematically invalid under perspective projection and leads to severe distortion and tearing. Instead, the reciprocal depth $1/Z$ and perspective-divided attributes ($U/Z, V/Z, L/Z$) are affine in screen space:

For each candidate pixel $(x, y)$ inside the triangle's screen-space bounding box, barycentric coordinates $(w_0, w_1, w_2)$ are evaluated:
$$w_0 = \frac{(x_1 - x)(y_2 - y) - (y_1 - y)(x_2 - x)}{\text{area}}$$
$$w_1 = \frac{(x_2 - x)(y_0 - y) - (y_2 - y)(x_0 - x)}{\text{area}}$$
$$w_2 = 1.0 - w_0 - w_1$$

If $w_0 \ge 0 \land w_1 \ge 0 \land w_2 \ge 0$:
1. **Depth Test**: Interpolate reciprocal depth:
   $$\frac{1}{Z} = w_0 \frac{1}{z_0} + w_1 \frac{1}{z_1} + w_2 \frac{1}{z_2}$$
   Closer geometry has larger reciprocal depth values. If $\frac{1}{Z} > \text{zbuffer}[y \cdot w + x]$, the pixel passes the depth test.
2. **Perspective Reconstruction & Authentic Texture Sampling**:
   $$Z_{\text{real}} = \frac{1}{1/Z}$$
   $$u = \left(w_0 \frac{u_0}{z_0} + w_1 \frac{u_1}{z_1} + w_2 \frac{u_2}{z_2}\right) \times Z_{\text{real}}$$
   $$v = \left(w_0 \frac{v_0}{z_0} + w_1 \frac{v_1}{z_1} + w_2 \frac{v_2}{z_2}\right) \times Z_{\text{real}}$$
   $$\text{tu} = \lfloor u \times 256.0 \rfloor \& 255$$
   $$\text{tv} = \lfloor v \times 256.0 \rfloor \& 255$$
   $$\text{texel} = \text{pixels}[\text{page\_offset} + \text{tv} \times 256 + \text{tu}]$$
3. **Alpha Test (Color Key 0 for Cutout Geometry - Opcodes 0x12, 0x16, and Non-Shadow 0x13/0x17)**:
   If the polygon opcode specifies cutout transparency and $\text{texel} == 0$, the pixel is discarded immediately without writing to the framebuffer or the Z-buffer. When $\text{texel} \ne 0$, it writes both the texel color and reciprocal depth to the Z-buffer in Pass 1.
4. **Shadow & Translucency Blending via `.SHD` (Opcodes 0x13, 0x17 with Shadow Model Archetype)**:
   Polygons configured for shadow projection or alpha remapping (e.g. cloud shadows, vehicle ground shadows, steam puffs, tree canopy shadows) look up their final blended color in the active circuit's 64KB `.SHD` matrix:
   $$\text{bg\_pixel} = \text{framebuffer}[y \cdot w + x]$$
   $$\text{color} = \text{shd\_table}[(\text{texel} \ll 8) \mid \text{bg\_pixel}]$$
   If $\text{color} == \text{bg\_pixel}$ or $\text{texel} == 0$, the texel is completely transparent relative to the background and is discarded without writing to the framebuffer.
   Crucially, **shadow and translucent blend polygons NEVER write to the depth buffer** ($\text{zbuffer}$), and are evaluated with a slight depth bias ($\epsilon = -0.00005$) against the Z-buffer so they do not Z-fight against coplanar road surfaces.
5. **Lighting & Shading Table Lookup**:
   $$\text{light} = \left(w_0 \frac{l_0}{z_0} + w_1 \frac{l_1}{z_1} + w_2 \frac{l_2}{z_2}\right) \times Z_{\text{real}}$$
   In Ignition's 64KB `.TAB` matrix, Column 0 represents full brightness (1.0), and Column 63 represents maximum darkness (0.0). For shaded polygons (opcodes 0x12, 0x15):
   $$\text{light\_level} = \text{clamp}(\lfloor (1.0 - \text{light}) \times 63.0 \rfloor, 0, 63)$$
   The `.TAB` file is indexed in row-major order: **Row** = original texel palette color index ($0 \dots 255$), **Column** = lighting level tier ($0 \dots 63$):
   $$\text{color} = \text{shading\_table}[(\text{texel} \ll 8) \mid \text{light\_level}]$$
   For unshaded scenery and road triangles (opcode 0x11) and shadow blends (`is_shadow_blend`), `.TAB` lookup is bypassed.
6. **Framebuffer & Z-Buffer Write**:
   $$\text{framebuffer}[y \cdot w + x] = \text{color}$$
   $$\text{zbuffer}[y \cdot w + x] = \frac{1}{Z} \quad (\text{only when } \neg \text{is\_shadow\_blend})$$

---

## 3.4 Two-Pass Ordering Table & Model-Type Opcode Routing

Reverse engineering of `MAINDOS.EXE` (`Track_PreprocessPlacements` at `FUN_0041b360`, `Lisa_DrawTriangle_OpcodeHelper` at `0x0044cb20`, `Lisa_RenderSubmeshes` at `0x0044c1f0`, and `Lisa_ExecuteRasterizerCommands` at `0x0044f0e9`) reveals that Lisa3D implements a dual-pass ordering table (OT) architecture with conditional opcode dispatch based on placed object archetype IDs:

### Authentic 12-Bit Archetype Masking (`FUN_0041b360`)
During level loading in `Track_PreprocessPlacements`, the engine loads `.PLC` object headers and applies a **12-bit bitmask**:
```c
// Decompiled from MAINDOS.EXE FUN_0041b360:
*puVar1 = *puVar1 & 0xfff;
```
This masks the upper flags from `obj->model_type`, extracting the genuine archetype index:
$$\text{raw\_type} = \text{model\_type} \& \text{0xFFF}$$

### Model Archetype Opcode Routing
When a polygon in `.MSH` specifies opcode `0x13` or `0x17`, it does **not** unconditionally act as a shadow blend. Instead, `Lisa_DrawTriangle_OpcodeHelper` inspects the 12-bit archetype:
* **Shadow Archetype (`raw_type < 100 || raw_type == 200 || (raw_type >= 300 && raw_type <= 302)`)**:
  * Routed to internal opcode `0x15` in the Ordering Table (`*puVar20 = 0x15`).
  * Processed as a shadow/translucency blend through `track.SHD`.
  * Rendered in **Pass 2** over existing framebuffer pixels.
  * **No Z-buffer writing** (`r->zbuffer[pixel] = inv_z` bypassed).
  * Evaluated with depth bias ($1/Z \ge \text{zbuffer}[pixel] - 0.00005$).
  * **USA Water Tank (`model_type = 0x4022002 -> raw_type = 2`)**: The water tank trestle beams are modeled with opcode `0x13`/`0x17`. Because `raw_type = 2 < 100`, they blend through `USA.SHD` into authentic **black silhouettes**.
  * **Austria Mountain Clouds (`model_type = 0x6002004 -> raw_type = 4`)**: Modeled with opcode `0x13`/`0x17`. In `AUSTRIA.SHD`, the cloud border color index 144 is the identity row ($\text{shd}[(144 \ll 8) \mid \text{bg}] == \text{bg}$), causing the border texels to be discarded cleanly as transparent air without drawing red/salmon rectangular borders.
  * **Iceland Steam Puffs (`model_type = 1`)**: Soft translucent mist over the canyon river.
* **General Scenery Archetype (All other model types $\ge 100$)**:
  * Routed to internal opcode `0x12` (`*puVar20 = 0x12`).
  * Processed as a **1-bit transparency cutout**: Color key 0 is discarded, and all non-zero texels are drawn as solid un-shaded texture pixels.
  * Rendered in **Pass 1** alongside standard opaque geometry with full depth testing and **Z-buffer writing**.

### Two-Pass Execution Sequence:
1. **Pass 1 (Opaque & Cutout Geometry)**:
   Renders all standard scenery (`0x11`), 1-bit cutouts (`0x12`, `0x16`), and non-shadow `0x13`/`0x17` geometry. All visible pixels write reciprocal depth to `r->zbuffer`, establishing complete occlusion for all solid terrain, road surfaces, buildings, and structures.
2. **Pass 2 (Translucent & Shadow Blend Geometry)**:
   Renders cloud shadows, vehicle ground shadows, water tank trestles, and gorge steam puffs (`0x13`/`0x17` on shadow archetypes). These blend directly with the underlying pixels already present in the framebuffer using `track.SHD`. Because depth writing is disabled, they never cut holes into the world behind them or prevent terrain from rendering.

---

## 3.5 Foreground Clipping & Game Fixes Menu

Reverse engineering of `Lisa_TransformVertices` (`FUN_00449e70` in `MAINDOS.EXE`) reveals how Lisa3D handles near-plane camera projection:
* **Authentic Lisa3D Vertex Clamping**: The original engine does **not** discard vertices or drop triangles when individual vertices fall close to or behind the camera plane. Instead, transformed camera $Z$ is clamped to a minimum positive integer (`DAT_0050ddfc`):
  ```c
  if (iVar15 < DAT_0050ddfc) {
      iVar15 = DAT_0050ddfc;
  }
  ```
  Only the average depth sum $(z_0 + z_1 + z_2) > 600$ and screen area $> 0$ are tested in `Lisa_DrawTriangle_OpcodeHelper` (`0x0044cb20`).
* **Source Port Correction**: Initial port builds erroneously discarded whole triangles if even a single vertex had $z_{\text{cam}} < 200.0$, causing entire road chunks to drop in close-up or low-angle views. Configuring `near_z = 20.0f` and area threshold to $-0.001f$ resolves this natively, restoring solid foreground rendering.
* **Game Fixes Menu Infrastructure**: The engine features a dedicated menu under **`Options > Gameplay > Game Fixes`**, designed to allow players to toggle workarounds and fixes specifically for genuine bugs present in the original 1997 game code and asset files.

---

## 4. Interactive 3D Camera Controls & Panning

In the in-race 3D view (`GAME_STATE_IN_RACE`), full free-camera navigation and visualization inspection tools are provided:

### 4.1 Navigation Controls
* **`W` / `S`**: Translate camera forward / backward along horizontal look direction ($-\sin(\text{yaw}), -\cos(\text{yaw})$).
* **`A` / `D`**: Strafe camera left / right perpendicular to look direction ($\cos(\text{yaw}), -\sin(\text{yaw})$).
* **`Q` / `E`** (or **`R` / `F`**, **`PageUp` / `PageDown`**): Elevate camera up / down along the world $Y$-axis.
* **Arrow Keys**: Orbit camera around current focal target (Yaw $\pm 1.8^\circ$, Pitch $\pm 1.2^\circ$).
* **`Z` / `X`**: Zoom camera distance in / out (Clamped range: $1,000 \dots 25,000$ units).
* **`SPACE`**: Toggle automatic 360-degree orbital flyby rotation.
* **`C` / `Home`**: Reset camera focal target to track origin $(0, -600, 0)$, yaw $45^\circ$, pitch $35^\circ$, distance $7500$, and pause auto-orbit.
* **`T`**: Toggle texture mapping ON / OFF (switches between authentic 1024x1024 `.TEX` mapping and 3D facet-shaded polygon visualization).
* **`ESC`**: Return cleanly to the Main Menu with authentic palette restoration.

### 4.2 Camera Pan Formulation (`Camera_Pan`)
The horizontal forward and right unit vectors in the $XZ$ plane are derived directly from the camera's azimuth yaw angle $\theta_{\text{yaw}}$:
$$\vec{F}_{\text{horiz}} = \begin{pmatrix} -\sin(\theta_{\text{yaw}}) \\ 0 \\ -\cos(\theta_{\text{yaw}}) \end{pmatrix}, \quad \vec{R}_{\text{horiz}} = \begin{pmatrix} \cos(\theta_{\text{yaw}}) \\ 0 \\ -\sin(\theta_{\text{yaw}}) \end{pmatrix}$$

Target translation preserves the look vector while shifting the orbit pivot:
$$\vec{T}_{\text{new}} = \vec{T}_{\text{old}} + \Delta_{\text{fwd}} \vec{F}_{\text{horiz}} + \Delta_{\text{right}} \vec{R}_{\text{horiz}} + \begin{pmatrix} 0 \\ \Delta_{\text{up}} \\ 0 \end{pmatrix}$$

---

## 5. Untextured Facet Debug Visualization

When texture mapping is toggled off (`T` key), the rasterizer enters an untextured debug mode designed for examining raw 3D mesh geometry, submesh boundaries, and track topography:

1. **Face Normal Calculation**:
   $$\vec{E}_1 = \vec{P}_1 - \vec{P}_0, \quad \vec{E}_2 = \vec{P}_2 - \vec{P}_0$$
   $$\vec{N} = \vec{E}_1 \times \vec{E}_2, \quad \hat{n} = \frac{\vec{N}}{\|\vec{N}\|}$$
2. **Directional Diffuse Lighting**:
   With a key light vector $\hat{L} = \text{normalize}(1, 2, 1) \approx (0.408, 0.816, 0.408)$:
   $$\text{diffuse} = \text{clamp}(\hat{n} \cdot \hat{L}, 0.25, 1.0)$$
3. **Submesh Distinct Color Allocation & Shading**:
   Each submesh receives a distinct base palette color index:
   $$\text{color}_{\text{base}} = 32 + ((\text{type} \times 37 + i \times 13) \pmod{200})$$
   The diffuse intensity is passed to the vertex shader and mapped through the 64KB `.TAB` shading matrix, producing lit, topographic 3D relief without textures.

---

## 6. Vehicle 3D Mesh Rendering & Chase Camera Architecture

### 6.1 `CARS.MSH` Model Space Conventions & Physics Scaling
In Ignition's vehicle geometry archive (`CARS.MSH`), 3D car body meshes are authored in a distinct local coordinate system with raw integer vertex coordinates:
* **$v_x$ (Longitudinal Length)**:
  * $+v_x$: Front bumper / hood (vehicle heading forward)
  * $-v_x$: Rear bumper / trunk / exhaust
  * Extent across archetype meshes: $[-30, +36]$ (raw length $\approx 66$ units).
* **$v_y$ (Vertical Height)**:
  * $-v_y$: Roof / canopy / upward
  * $+v_y$: Chassis floor / wheel wells / downward
  * Extent: $[-17, +12]$ (raw height $\approx 29$ units).
* **$v_z$ (Lateral Width)**:
  * $+v_z$: Left side
  * $-v_z$: Right side
  * Extent: $[-19, +18]$ (raw width $\approx 37$ units).

#### Physical Scaling vs Mesh Coordinates
In Ignition, vehicle mesh vertices in `CARS.MSH` are already authored in world coordinates ($66 \times 37 \times 29$ units). On track roads of width $\sim 550$ units, this provides authentic proportion (roughly 8 car widths per road ribbon). The internal multiplier `PHYSICS_SCALE_FACTOR` ($21.76$ at `0x00479af0`) converts vehicle dynamic velocities ($v \cdot \Delta t \cdot 21.76$) and SI physical parameters (wheelbase $2.3\text{ m} \times 21.76 \approx 50\text{ units}$, track width $1.15\text{ m} \times 21.76 \approx 25\text{ units}$), and must NOT be applied to mesh vertices.

### 6.2 Authentic Vehicle Dimensions & CARS.MSH Coordinate Space
In `CARS.MSH`, vertex coordinates are authored directly in world units ($1\text{ unit} \approx 4\text{ cm}$):
* **Cooper Body Extents**:
  * Length ($X$ axis): $\Delta X = 36 - (-30) = 66\text{ units}$ ($\sim 2.6\text{ meters}$).
  * Width ($Z$ axis): $\Delta Z = 18 - (-19) = 37\text{ units}$ ($\sim 1.5\text{ meters}$).
  * Height ($Y$ axis): $\Delta Y = 12 - (-17) = 29\text{ units}$ ($\sim 1.15\text{ meters}$).
* **Track Proportion**: Standard track road width is $\sim 550\text{ units}$, accommodating 6–8 vehicles side by side.
* **Physics Scale vs Model Scale**: The constant `PHYSICS_SCALE_FACTOR` ($21.76$) is used exclusively for time-step velocity integration ($v \cdot \Delta t \cdot 21.76$) and suspension trackwidth calculations ($1.15\text{ m} \times 21.76 \approx 25\text{ units}$). The 3D vehicle mesh is rendered at native scale $1.0$ (`veh->params.chassis_scale`).

### 6.3 Chassis Coordinate Space Transformation & Grounding
To integrate with the vehicle dynamics simulation (`Vehicle_Update`) and the Lisa3D world coordinate system ($+Y$ up, $+X$ east, $+Z$ north), local `CARS.MSH` vertices are mapped to chassis space:
$$\begin{pmatrix} x_{\text{chassis}} \\ y_{\text{chassis}} \\ z_{\text{chassis}} \end{pmatrix} = \begin{pmatrix} -v_z \times s \\ -v_y \times s + 7.0 \times s \\ +v_x \times s \end{pmatrix}$$

Where:
* **$l_z = +v_x \times s$**: Maps $+X_{\text{msh}}$ (front grille and headlights) to forward heading $+Z_{\text{chassis}}$.
* **$l_x = -v_z \times s$**: In `CARS.MSH`, $+Z$ is left and $-Z$ is right. Negating $v_z$ ensures that chassis $+X$ corresponds to vehicle right, preserving authentic polygon winding and exterior surface normals.
* **$l_y = -v_y \times s + 7.0 \times s$**: Inverts vertical axis ($-Y$ roof to $+Y$ ground) and offsets vertices so tire bottoms ($v_y = 12$) sit at $y_{\text{chassis}} = -5.0$, perfectly flush with the road elevation ($Y_{\text{car}} - 5.0 = \text{ground\_y}$).

### 6.4 3D Chassis Orientation (Roll $\to$ Pitch $\to$ Yaw)
Chassis orientation is evaluated via Tait-Bryan rotations about the chassis axes:
1. **Roll ($\phi$)** around longitudinal axis ($Z_{\text{chassis}}$):
   $$x_1 = x_{\text{chassis}} \cos\phi - y_{\text{chassis}} \sin\phi$$
   $$y_1 = x_{\text{chassis}} \sin\phi + y_{\text{chassis}} \cos\phi$$
   $$z_1 = z_{\text{chassis}}$$
2. **Pitch ($\theta$)** around lateral axis ($X_{\text{chassis}}$):
   $$x_2 = x_1$$
   $$y_2 = y_1 \cos\theta - z_1 \sin\theta$$
   $$z_2 = y_1 \sin\theta + z_1 \cos\theta$$
3. **Yaw ($\psi$)** around world vertical axis ($Y$):
   $$r_x = x_2 \cos\psi + z_2 \sin\psi$$
   $$r_y = y_2$$
   $$r_z = -x_2 \sin\psi + z_2 \cos\psi$$

The final world coordinates for vertex $i$ are:
$$\vec{P}_{i,\text{world}} = \begin{pmatrix} X_{\text{car}} + r_x \\ Y_{\text{car}} + r_y \\ Z_{\text{car}} + r_z \end{pmatrix}$$

### 6.5 Wheel Suspension Struts & Contact Points
Each of the 4 wheels tracks its simulated world ground contact position $(x_c, y_c, z_c)$ computed via `Surface_Raycast`. Suspension struts are drawn as 3D lines from the ground contact point upwards into the chassis wheel hub:
$$\vec{P}_{\text{contact}} = \begin{pmatrix} x_c \\ y_c \\ z_c \end{pmatrix}, \quad \vec{P}_{\text{top}} = \begin{pmatrix} x_c \\ y_c + 12.0 \\ z_c \end{pmatrix}$$

### 6.6 Lisa3D Camera View Basis & Chase Camera Geometry
#### Camera Basis Construction
The Lisa3D rasterizer employs the canonical camera basis:
$$\vec{F} = \text{normalize}(\vec{T}_{\text{cam}} - \vec{P}_{\text{cam}})$$
$$\vec{R} = \text{normalize}(\vec{F} \times \vec{U}_{\text{world}})$$
$$\vec{U} = \vec{R} \times \vec{F}$$

* In Lisa3D camera coordinates, world $+X$ projects to Screen Left ($x_{\text{screen}} < \text{width}/2$) and world $-X$ projects to Screen Right ($x_{\text{screen}} > \text{width}/2$).
* Crucially, this basis ensures front-facing terrain and object triangles preserve **negative signed screen area** ($\text{area} < -0.001$), matching the backface culling filter in `Renderer_DrawTriangle`.
* Steering input polarity is resolved at the input dispatch level in `game_state.c` (`key_left` $\to \text{steer} = +1.0$ to steer left into world $+X$).

#### Chase Camera Geometry
The chase camera tracks smoothly behind the vehicle looking forward along the car's heading direction:
* **Target Position**: Centers on the vehicle's center of mass: $\vec{T} = (X_{\text{car}}, Y_{\text{car}} + 5.0, Z_{\text{car}})$.
* **Heading Alignment**: $\theta_{\text{cam\_yaw}} = \psi_{\text{car}} \times \frac{180^\circ}{\pi} + 180.0^\circ$ (places camera behind rear bumper looking forward).
* **Yaw Smoothing**: Exponential lag tracking with factor $\alpha = 0.15$:
  $$\text{yaw}_{\text{cam}} \leftarrow \text{yaw}_{\text{cam}} + \text{wrap}(\theta_{\text{cam\_yaw}} - \text{yaw}_{\text{cam}}) \times 0.15$$
* **Terrain Tilt & Pitch**: Pitch tilts with the vehicle's terrain slope:
  $$\text{Pitch}_{\text{cam}} = 24^\circ + 0.35 \cdot \theta_{\text{pitch}}$$
* **Chase Distance**: Fixed at $220.0\text{ world units}$ behind the center of mass, elevating the observer above and behind the $66\text{-unit}$ chassis with clear road sightlines.


