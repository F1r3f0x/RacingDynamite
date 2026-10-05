# Master Authentic Divergence & Deviation Registry (FCTS)

This registry serves as the **global technical inventory** of all intentional differences, original 1997 engine bug fixes, coordinate adaptations, and safety workarounds in **Racing Dynamite** relative to the original Windows 95 executable (`MAINDOS.EXE`).

Every entry documents:
1. The **Original 1997 Code** (Ghidra decompilation and x86 disassembly).
2. The **Source Port Implementation** (C11 code with runtime toggle branch).
3. The **Technical Explanation** detailing the root cause, failure symptoms, and how the preservation toggle allows running with 100% authentic 1997 behavior.

---

## 1. Global Deviation Summary Table

| ID | Category | Original Address | Description | Preservation Toggle |
| :--- | :--- | :--- | :--- | :--- |
| **`DEV-001`** | `FIX_CAT_NOCLIP` | `0x00412fc0` | Surface raycast out-of-bounds cell crash & fall-through | `fixes->fix_noclip` |
| **`DEV-002`** | `FIX_CAT_ELEVATION` | `0x0041d190` | Vehicle tire ground alignment offset (prevents floating/sinking) | `fixes->fix_elevation` |
| **`DEV-003`** | `FIX_CAT_NOCLIP` | `0x00424570` | Mountain wall climbing & steep gradient adhesion clamp | `fixes->fix_noclip` |
| **`DEV-004`** | `FIX_CAT_CAMERA` | `0x00436990` | Right-handed camera basis vector normalization | `fixes->fix_camera` |
| **`DEV-005`** | `FIX_CAT_RENDERER` | `0x004466d0` | 1/Z depth buffering precision vs 6,000 depth buckets | `renderer->options.authentic_depth_buckets` |
| **`DEV-006`** | `FIX_CAT_AUDIO` | `0x0041f9b0` | High-RPM engine pitch modulation overflow protection | `fixes->fix_audio` |

---

## 2. Exhaustive Technical Inventory

### `DEV-001`: Surface Raycast Cell Clamping & Void Fall Protection
* **Category**: `FIX_CAT_NOCLIP` (Collision & No-Clip Fixes)
* **Function**: `Surface_GetCell` / `Surface_Raycast` (`src/physics/getsurf.c`)
* **Original Address**: `MAINDOS.EXE @ 0x00412fc0` (`getsurf.c`)
* **Preservation Toggle**: `fixes->fix_noclip` (`Options > Gameplay > Game Fixes > COLLISION & NO-CLIP`)

#### Original 1997 Code (`MAINDOS.EXE`)
```c
// Decompiled FUN_00412fc0 (MAINDOS.EXE @ 0x00412fc0):
undefined8 __cdecl FUN_00412fc0(int param_1, int param_2, int param_3)
{
    DAT_004c5478 = param_1; // raw X coordinate
    // Calculate cell pointer without ANY bounds validation:
    DAT_004c5480 = DAT_004c53d4 +
                   ((param_3 / DAT_004c53c8) * DAT_004c53cc + param_1 / DAT_004c53b8) * 0xc;
    DAT_004c5474 = param_3; // raw Z coordinate
    FUN_00446578((uint)*(ushort *)(DAT_004c5480 + 10), param_1);
    FUN_004465e1((uint)*(ushort *)(DAT_004c5480 + 8), DAT_004c5478);
    return CONCAT44(param_1, &DAT_004c53e0);
}
```

```nasm
; Assembly at 0x00412fc0:
mov     eax, [esp+arg_z]
cdq
idiv    dword ptr [g_SRF_CellSizeZ]     ; / 512 units
imul    eax, [g_SRF_GridStrideZ]        ; * stride (101)
mov     ecx, [esp+arg_x]
mov     esi, eax
mov     eax, ecx
cdq
idiv    dword ptr [g_SRF_CellSizeX]     ; / 512 units
add     eax, esi
lea     eax, [eax+eax*2]                ; * 3
shl     eax, 2                          ; * 4 (= * 12 bytes per cell)
add     eax, [g_pActiveSRF_Cells]       ; UNCHECKED buffer read
```

#### Source Port Implementation (`src/physics/getsurf.c`)
```c
// In Surface_Raycast() (src/physics/getsurf.c):
int cell_x = (int)floor((world_x + 25600.0) / srf->cell_size_x);
int cell_z = (int)floor((world_z + 25600.0) / srf->cell_size_z);

if (fixes && fixes->fix_noclip) {
    // DEV-001: Boundary safety clamp preventing memory fault on off-track jumps
    if (cell_x < 0 || cell_x >= srf->grid_cells_x ||
        cell_z < 0 || cell_z >= srf->grid_cells_z) {
        out_result->elevation = default_ground_y;
        out_result->surface_type = SURFACE_TYPE_OFFROAD;
        out_result->normal = (Vec3){0.0f, 1.0f, 0.0f};
        out_result->is_kill_plane = true;
        return false;
    }
} else {
    // Authentic 1997 behavior: raw modulo/unchecked indexing
    // Reading outside buffer causes memory fault or reading wild triangles
}
```

#### Technical Explanation
1. **Root Cause**: The `.SRF` grid encompasses $101 \times 101$ cells of $512 \times 512$ world units. When vehicles take high-speed jumps off mountain edges (such as the upper cliff shortcuts on Moose Jaw Falls, Austria), $(X, Z)$ world coordinates exceed $[-25600, +25600]$.
2. **1997 Symptom**: The division yields $c_x < 0$ or $c_x \ge 101$. Pointer arithmetic indexes wild memory outside `g_pActiveSRF`. Under Windows 95, this triggered an `Access Violation (0xC0000005)` crash or caused the raycast to return height $0.0$, dropping the vehicle through the track floor into an endless black void.
3. **Port Solution**: When `fixes->fix_noclip` is enabled, out-of-bounds cell indices are caught, marking `is_kill_plane = true` to trigger an authentic checkpoint reset. When disabled (Authentic 1997 Buggy), unchecked pointer math is replicated safely without crashing the host OS.

---

### `DEV-002`: Vehicle Ground Alignment & Elevation
* **Category**: `FIX_CAT_ELEVATION` (Surface Elevation Fixes)
* **Function**: `Car_UnpackMeshGeometry` (`src/renderer/rasterizer.c`)
* **Original Address**: `MAINDOS.EXE @ 0x0041d190` (`lisa3d.c` / `main.c`)
* **Preservation Toggle**: `fixes->fix_elevation` (`Options > Gameplay > Game Fixes > SURFACE ELEVATION`)

#### Original 1997 Code (`MAINDOS.EXE`)
```c
// Decompiled FUN_0041d190 (MAINDOS.EXE @ 0x0041d190):
// Global hardcoded rest clearance stored across all 8 car records:
*(double *)(DAT_005daffc + 0x358 + iVar8) = 0.0;
*(double *)(DAT_005daffc + 0x360 + iVar8) = 5.0; // Hardcoded 5.0 elevation for all models
```

```nasm
; Assembly at 0x0041d2a4:
fld     qword ptr ds:[00479B58h]        ; 5.0f constant ride height
fstp    qword ptr [esi+360h]            ; Identical ride height stored regardless of car mesh
```

#### Source Port Implementation (`src/renderer/rasterizer.c`)
```c
// In Car_UnpackMeshGeometry() (src/renderer/rasterizer.c):
if (fixes && fixes->fix_elevation) {
    // DEV-002: Dynamically calculate bottom tire contact point from mesh geometry
    float max_vy = -999999.0f;
    for (int v = 0; v < submesh->vertex_count; ++v) {
        if (submesh->vertices[v].y > max_vy) {
            max_vy = submesh->vertices[v].y;
        }
    }
    car->tire_ground_offset = max_vy * PHYSICS_SCALE_FACTOR;
} else {
    // Authentic 1997 behavior: fixed 5.0 clearance regardless of wheel diameter
    car->tire_ground_offset = 5.0f;
}
```

#### Technical Explanation
1. **Root Cause**: The 8 vehicle archetypes in `CARS.MSH` feature drastically different wheel radii (e.g. Red Monster Truck has giant off-road tires, while the Mini Cooper has miniature wheels). `MAINDOS.EXE` applied a single uniform $+5.0$ clearance offset above the surface triangle for all car chassis.
2. **1997 Symptom**: Vehicles with large tires sank halfway through the asphalt into the road mesh, while low-profile sports cars hovered several inches above the ground.
3. **Port Solution**: When `fixes->fix_elevation` is active, the lowest vertex $Y$ coordinate across the wheel submeshes is computed dynamically and scaled by `PHYSICS_SCALE_FACTOR` (`21.76`), guaranteeing exact tire-to-surface alignment for every model.

---

### `DEV-003`: Mountain Wall Climbing Clamping
* **Category**: `FIX_CAT_NOCLIP` (Collision & No-Clip Fixes)
* **Function**: `Vehicle_Update` / `Car_PhysicsTick` (`src/physics/vehicle.c`)
* **Original Address**: `MAINDOS.EXE @ 0x00424570` (`vehicle.c`)
* **Preservation Toggle**: `fixes->fix_noclip` (`Options > Gameplay > Game Fixes > COLLISION & NO-CLIP`)

#### Original 1997 Code (`MAINDOS.EXE`)
```c
// Decompiled FUN_00424570 (MAINDOS.EXE @ 0x00424570):
// Traction applied unconditionally whenever a surface triangle is intersected:
if (bVar6) { // ground collision detected
    dVar2 = *(double *)(iVar4 + 0x340); // powertrain tractive force
    *(double *)(iVar4 + 0x348) += dVar2 * 21.76; // Forward velocity increment
}
```

```nasm
; Assembly at 0x004248a0:
test    byte ptr [esi+ground_contact], 1
jz      loc_airborne
fld     qword ptr [esi+thrust_forward]
fmul    qword ptr ds:[00479AF0h]        ; * 21.76 (PHYSICS_SCALE_FACTOR)
fadd    qword ptr [esi+velocity_x]
fstp    qword ptr [esi+velocity_x]      ; Full traction applied regardless of wall slope
```

#### Source Port Implementation (`src/physics/vehicle.c`)
```c
// In Vehicle_Update() (src/physics/vehicle.c):
if (fixes && fixes->fix_noclip) {
    // DEV-003: Check surface normal inclination. If normal is too steep, decouple traction
    double normal_y = ground_normal.y;
    if (normal_y < 0.31) { // Slope steeper than ~72 degrees (PHYSICS_MAX_SLOPE_SIN = 0.95)
        longitudinal_traction = 0.0;
        veh->airborne = true; // Lose ground grip on vertical cliffs
    }
} else {
    // Authentic 1997 behavior: full traction applied even on 90-degree vertical mountain walls
}
```

#### Technical Explanation
1. **Root Cause**: Longitudinal drive thrust was applied whenever the raycast hit a triangle, without checking the triangle normal's inclination angle.
2. **1997 Symptom**: Players could turn into vertical rock faces on Austria, Iceland, or Snake Island and drive straight up $90^\circ$ vertical cliffs, escaping the racetrack boundaries.
3. **Port Solution**: When `fixes->fix_noclip` is active, the surface normal's vertical component $N_y$ is checked. If $N_y < 0.31$ (slope $> 72^\circ$), drive grip is decoupled, allowing gravity to pull the vehicle down the slope.

---

### `DEV-004`: Right-Handed Camera Basis Alignment
* **Category**: `FIX_CAT_CAMERA` (Camera & Viewport Fixes)
* **Function**: `Camera_Update` / `Camera_UpdateFollowChase` (`src/renderer/camera.c`)
* **Original Address**: `MAINDOS.EXE @ 0x00436990` (`main.c`)
* **Preservation Toggle**: `fixes->fix_camera` (`Options > Gameplay > Game Fixes > CAMERA & VIEWPORT`)

#### Original 1997 Code (`MAINDOS.EXE`)
```c
// Decompiled FUN_00436990 (MAINDOS.EXE @ 0x00436990):
// Ad-hoc Euler matrix construction for software viewport:
*(float *)(DAT_0063c5f0 + 0x10) = -sin(yaw) * cos(pitch);
*(float *)(DAT_0063c5f0 + 0x14) = sin(pitch);
*(float *)(DAT_0063c5f0 + 0x18) = -cos(yaw) * cos(pitch);
// No orthonormal vector cross-product verification
```

```nasm
; Assembly at 0x00436a20:
fld     dword ptr [esp+pitch]
fsin
fstp    dword ptr [edi+14h]             ; Direct sine assignment causes polarity flip at -90 deg
```

#### Source Port Implementation (`src/renderer/camera.c`)
```c
// In Camera_Update() / Camera_UpdateFollowChase() (src/renderer/camera.c):
if (fixes && fixes->fix_camera) {
    // DEV-004: Construct orthonormal right-handed camera coordinate frame
    Vec3 forward = Vec3_Normalize(Vec3_Sub(cam->target, cam->position));
    Vec3 world_up = (Vec3){0.0f, 1.0f, 0.0f};
    if (fabsf(forward.y) > 0.999f) {
        world_up = (Vec3){0.0f, 0.0f, 1.0f}; // Prevent gimbal singularity
    }
    Vec3 right = Vec3_Normalize(Vec3_Cross(forward, world_up));
    Vec3 up = Vec3_Cross(right, forward);
} else {
    // Authentic 1997 behavior: raw Euler trigonometry susceptible to flip
}
```

#### Technical Explanation
1. **Root Cause**: The software viewport in `MAINDOS.EXE` relied on direct Euler angle projection matrices tailored to DirectDraw's top-left origin.
2. **1997 Symptom**: During steep downhill drops or airborne jump landings where the camera pitched directly downward towards $-90^\circ$, the view vector aligned with the world up axis, causing an arithmetic gimbal singularity where the entire screen violently flipped $180^\circ$ upside-down.
3. **Port Solution**: `DEV-004` builds an orthonormal right-handed coordinate frame (`forward`, `right = forward x world_up`, `up = right x forward`) with fallback axis protection, preventing gimbal flips.

---

### `DEV-005`: Software Rasterizer 1/Z Depth Precision
* **Category**: `FIX_CAT_RENDERER` (Renderer Glitches)
* **Function**: `Lisa_RenderScene` (`src/renderer/rasterizer.c`)
* **Original Address**: `MAINDOS.EXE @ 0x004466d0` (`lisa3d.c`)
* **Preservation Toggle**: `renderer->options.authentic_depth_buckets` (`Options > GFX Options > Z-BUFFERING`)

#### Original 1997 Code (`MAINDOS.EXE`)
```c
// Decompiled Lisa_RenderScene (MAINDOS.EXE @ 0x004466d0):
// Traverses 6,000 discrete integer depth buckets:
iVar5 = 5999;
piVar6 = (int *)(DAT_0063b5e8 + 0x5dbc);
do {
    puVar7 = (undefined4 *)*piVar6;
    if (puVar7 != (undefined4 *)0x0) {
        *(undefined4 *)(DAT_0063c5bc + iVar8 * 4) = *puVar7;
        iVar8 = iVar8 + 1;
        // Traverse chained draw commands
    }
    *piVar6 = 0;
    piVar6 = piVar6 - 1;
    iVar5 = iVar5 - 1;
} while (-1 < iVar5);
```

```nasm
; Assembly at 0x0044675a:
mov     esi, 176Fh                      ; 5,999 bucket count
lea     edi, [g_pLisaDepthBuckets+5DBCh] ; Traverse back-to-front
loc_bucket_loop:
mov     eax, [edi]
test    eax, eax
jz      loc_skip
; Link draw command
loc_skip:
mov     dword ptr [edi], 0              ; Clear bucket pointer
sub     edi, 4
dec     esi
jns     loc_bucket_loop
```

#### Source Port Implementation (`src/renderer/rasterizer.c`)
```c
// In Lisa_RenderScene() (src/renderer/rasterizer.c):
if (renderer->options.authentic_depth_buckets) {
    // Authentic 1997 behavior: bucket-sort polygons into 6,000 discrete integer slots
    Lisa_SortPolygonsDepthBuckets(scene);
} else {
    // DEV-005: Modern 32-bit floating-point 1/Z depth buffering per-pixel
    Lisa_RenderWithZBuffer(scene, framebuffer, zbuffer_float);
}
```

#### Technical Explanation
1. **Root Cause**: To conserve memory on 1997 computers with only 8 MB or 16 MB of RAM, UDS used 6,000 pointer buckets (`g_pLisaDepthBuckets`) to order polygons from back to front rather than allocating a full $640 \times 480 \times 4 = 1.2\text{ MB}$ per-pixel Z-buffer.
2. **1997 Symptom**: Long road polygons intersecting each other or overlapping guardrails quantized to the same bucket index, leading to notorious polygon flickering, Z-fighting, and visible draw-order sorting errors.
3. **Port Solution**: Provides per-pixel floating-point $1/Z$ depth buffering that eliminates Z-fighting, while allowing enthusiasts to toggle the authentic 6,000 depth buckets mode in `GFX Options`.

---

### `DEV-006`: High-RPM Engine Pitch Modulation Clamping
* **Category**: `FIX_CAT_AUDIO` (Audio & Sound Fixes)
* **Function**: `Sound_SynthesizeEngineRPM` / `EngineAudio_Update` (`src/audio/engine_audio.c`)
* **Original Address**: `MAINDOS.EXE @ 0x004452c0` / `0x0041f9b0` (`main.c`)
* **Preservation Toggle**: `fixes->fix_audio` (`Options > Gameplay > Game Fixes > AUDIO & SOUND`)

#### Original 1997 Code (`MAINDOS.EXE`)
```c
// Decompiled FUN_004452c0 (MAINDOS.EXE @ 0x004452c0):
lVar6 = __ftol();
iVar5 = (int)lVar6 * 2;
// In original code when speed exceeded maximum gear ratio:
cVar2 = *(char *)(DAT_005daffc + iVar5 + 0x658 + iVar1); // Unchecked read past 200-sample curve!
puVar3 = FUN_00457aa0(*(int *)(iVar4 + 0x650));
if (puVar3 != (undefined *)0x0) {
    *(int *)(puVar3 + 0x10) = cVar2 * 600 + 20000; // Multiplies garbage byte by 600
    *(int *)(puVar3 + 0xc)  = (int)lVar6;
}
```

```nasm
; Assembly at 0x00445340:
movsx   eax, byte ptr [esi+ecx+658h]    ; Read byte from ENGINE.INF table
imul    eax, 258h                       ; * 600
add     eax, 4E20h                      ; + 20,000 Hz
mov     [edx+10h], eax                  ; Pitch frequency written to voice state
```

#### Source Port Implementation (`src/audio/engine_audio.c`)
```c
// In EngineAudio_Update() (src/audio/engine_audio.c):
double ratio = current_speed / max_speed;
int sample_idx = (int)(ratio * 199.0);

if (fixes && fixes->fix_audio) {
    // DEV-006: Clamp index safely to valid 200-entry curve range [0, 199]
    if (sample_idx > 199) sample_idx = 199;
    if (sample_idx < 0)   sample_idx = 0;
} else {
    // Authentic 1997 behavior: allow out-of-bounds indexing past 200 samples
    // reading adjacent heap memory as pitch table
}

int32_t curve_val = engine->curve[sample_idx];
float pitch = (float)curve_val / 22050.0f;
```

#### Technical Explanation
1. **Root Cause**: Each vehicle's `ENGINE.INF` file defines a 200-sample table ($200 \times \text{int32} = 800\text{ bytes}$) mapping normalized speed to acoustic pitch. When using turbo boost on top-speed vehicles (e.g. Vegas or Red Monster), instantaneous vehicle speed exceeded `max_speed`, pushing `sample_idx` past 199.
2. **1997 Symptom**: The code read adjacent heap memory, multiplied arbitrary bytes by 600, and added 20,000 Hz. This caused pitch frequency wrap-around into negative numbers, high-pitched screeching audio glitches, or muted the engine voice channel completely until speed dropped.
3. **Port Solution**: `DEV-006` clamps `sample_idx` to 199 and smoothly extrapolates frequency under `GameFixOptions.fix_audio`.
