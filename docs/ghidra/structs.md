# Reconstructed Data Structures

This document catalogues the reconstructed C structures, member byte offsets, and original memory alignments recovered from `IGN_WIN.EXE` and `MAINDOS.EXE`.

---

## 1. Surface & Collision Grid (`getsurf.c`)

### `SrfHeader` (36 bytes / 9 x `int32_t`)
```c
typedef struct {
    int32_t grid_cells_x;     // 0x00: Grid cells along X (200)
    int32_t grid_cells_z;     // 0x04: Grid cells along Z (200)
    int32_t cell_size_z;      // 0x08: World dimension of cell Z (512)
    int32_t cell_size_x;      // 0x0C: World dimension of cell X (512)
    int32_t grid_stride_x;    // 0x10: Spatial index stride X (101)
    int32_t grid_stride_z;    // 0x14: Spatial index stride Z (101)
    int32_t triangle_count;   // 0x18: Total surface collision triangles
    int32_t table1_count;     // 0x1C: Primary index buffer integer count
    int32_t table2_count;     // 0x20: Secondary index buffer integer count
} SrfHeader;
```

### `SrfCell` (12 bytes)
```c
typedef struct {
    int32_t  table2_offset;   // 0x00: Byte offset into table2 (divide by 4)
    int32_t  table1_offset;   // 0x04: Byte offset into table1 (divide by 4)
    uint16_t table1_count;    // 0x08: Number of triangles intersecting cell
    uint16_t table2_count;    // 0x0A: Number of secondary entities
} SrfCell;
```

### `SrfTriangle` (24 bytes / 6 x `int32_t`)
```c
typedef struct {
    int32_t x_base;           // 0x00: Base apex X coordinate
    int32_t z_base;           // 0x04: Base apex Z coordinate
    int32_t slope1;           // 0x08: 16.16 fixed-point slope dx1/dz
    int32_t slope2;           // 0x0C: 16.16 fixed-point slope dx2/dz
    int32_t flags_and_dz;     // 0x10: Low 16 bits = dz (int16_t), High 16 bits = submesh polygon dword offset
    int32_t v_ptr;            // 0x14: Byte offset into .PLC placed object array (obj_idx = v_ptr / 42)
} SrfTriangle;
```

---

## 2. Object Placement (`main.c`)

### `PlcObject` (20 bytes)
```c
typedef struct {
    int32_t submesh_offset;   // 0x00: Offset in 4-byte dwords into .MSH geometry
    int32_t model_type;       // 0x04: Scenery / collision model archetype (e.g. 300, 2, 3)
    int32_t pos_x;            // 0x08: World X position
    int32_t pos_y;            // 0x0C: World Y elevation
    int32_t pos_z;            // 0x10: World Z position
} PlcObject;
```

---

## 3. 3D Meshes & Geometry (`lisa3d.c`, `main.c`)

### `MshPolygon` (44 bytes / 11 x `uint32_t`)
```c
typedef struct {
    uint32_t header;          // 0x00: Opcode in low byte (0x11, 0x12, 0x13, 0x15, 0x16, 0x17), flags in high 24 bits
    uint32_t vi0;             // 0x04: Index of vertex 0 in submesh vertex buffer
    uint32_t vi1;             // 0x08: Index of vertex 1 in submesh vertex buffer
    uint32_t vi2;             // 0x0C: Index of vertex 2 in submesh vertex buffer
    int32_t  tu0;             // 0x10: Vertex 0 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)
    int32_t  tv0;             // 0x14: Vertex 0 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)
    int32_t  tu1;             // 0x18: Vertex 1 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)
    int32_t  tv1;             // 0x1C: Vertex 1 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)
    int32_t  tu2;             // 0x20: Vertex 2 U texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)
    int32_t  tv2;             // 0x24: Vertex 2 V texture coordinate (8.8 fixed-point, divide by 65536.0f to normalize, or 256.0f for texels)
    uint32_t extra;           // 0x28: Texture page byte offset within .TEX file (page_index * 65536)
} MshPolygon;
```


### `MshSubmesh` (Variable length)
```c
typedef struct {
    int32_t     vertex_count;  // 0x00: Number of 3D vertices
    int32_t     polygon_count; // 0x04: Number of polygon records
    // Followed by vertex_count * 3 x int32_t (X, Y, Z for each vertex)
    // Followed by polygon_count * MshPolygon (44 bytes each)
} MshSubmesh;
```

### `LisaCamera` (168 bytes / `0xA8` allocated in `Lisa_InitEngineMemory`)
```c
#pragma pack(push, 1)
typedef struct LisaCamera {
    int32_t pad00[6];              // 0x00: 24 bytes internal transformation state
    double  rot_x;                 // 0x18: Camera rotation pitch
    double  rot_y;                 // 0x20: Camera rotation yaw
    double  rot_z;                 // 0x28: Camera rotation roll
    double  zoom;                  // 0x30: Focal length / zoom factor
    int32_t enable_sky;            // 0x38: Sky backdrop enable flag
    int32_t enable_frustum_cull;   // 0x3C: Spatial grid frustum cull flag
    int32_t enable_transform;      // 0x40: Vertex transformation flag
    int32_t pad44[3];              // 0x44: 12 bytes
    int32_t enable_submeshes;      // 0x50: Submesh processing flag
    int32_t enable_depth_sort;     // 0x54: Depth bucket sorting flag
    int32_t shading_mode;          // 0x58: 0 = unshaded, 1 = shaded/alpha
    int32_t vertex_counter;        // 0x5C: Transformed vertex counter
    int32_t visible_obj_count;     // 0x60: Count of visible objects passed culling
    int32_t submesh_count;         // 0x64: Count of visible submeshes
    int32_t active_draw_cmd;       // 0x68: Active draw command index
    int32_t pad6c[5];              // 0x6C: 20 bytes
    int32_t viewport_x;            // 0x80: Viewport center X
    int32_t viewport_y;            // 0x84: Viewport center Y
    int32_t viewport_width;        // 0x88: Viewport screen width
    int32_t pad8c[4];              // 0x8C: 16 bytes
    int32_t fov_x;                 // 0x9C: Horizontal FOV scale (8.8 fixed-point)
    int32_t fov_y;                 // 0xA0: Vertical FOV scale (8.8 fixed-point)
    int32_t projection_type;       // 0xA4: 0 = perspective 3D, 1 = panorama/ortho
} LisaCamera;
#pragma pack(pop)
```

---

## 3. Vehicle Physics & Simulation (`main.c`)

> [!NOTE]
> **Vehicle Runtime State Block Stride**: In `IGN_WIN.EXE`, vehicle runtime state is stored in a flat array indexed as `base + car_idx * 0x484c` (18,508 bytes per vehicle), confirmed by `Camera_UpdateChase` (`FUN_0043c910`) decompilation: `DAT_005285c0 * 0x484c`. This total block size encompasses the full vehicle simulation state (physics parameters, wheel contact, audio curves, mesh geometry) and is larger than any individual struct below due to embedded engine audio data and runtime matrices.

### `CarPhysicsState` (124 bytes / `0x7C` allocated in `Track_LoadSplines`)
```c
typedef struct {
    double  engine_power;     // 0x00: Mass / power scaled by difficulty mode
    double  acceleration;     // 0x08: Forward traction acceleration
    double  top_speed;        // 0x10: Terminal velocity clamp
    double  steering_rate;    // 0x18: Turning responsiveness
    double  brake_force;      // 0x20: Deceleration coefficient
    double  turbo_boost;      // 0x28: Turbo propulsion multiplier
    double  suspension_k;     // 0x30: Spring rate
    double  damping_c;        // 0x38: Shock absorber damping
    double  collision_radius; // 0x40: Spherical bounding volume
    int32_t mass_integer;     // 0x48: Fixed-point mass value
    int32_t wheel_fl_y;       // 0x60: Front-left wheel elevation
    int32_t wheel_fr_y;       // 0x68: Front-right wheel elevation
    int32_t wheel_rl_y;       // 0x70: Rear-left wheel elevation
    int32_t wheel_rr_y;       // 0x78: Rear-right wheel elevation
} CarPhysicsState;
```


---

## 4. Track Splines & Waypoints (`main.c`)

### `TrackSplineNode` (75 bytes / `0x4B` allocated in `Track_LoadSplines`)
```c
typedef struct {
    uint8_t  type;            // 0x00: Node type identifier
    uint8_t  code;            // 0x01: Road code passed from .TRI
    int32_t  left_x;          // 0x02: Left rail coordinate X (+ 0x6400)
    int32_t  left_y;          // 0x06: Left rail coordinate Y
    int32_t  left_z;          // 0x0A: Left rail coordinate Z (+ 0x6400)
    int32_t  right_x;         // 0x0E: Right rail coordinate X (+ 0x6400)
    int32_t  right_y;         // 0x12: Right rail coordinate Y
    int32_t  right_z;         // 0x16: Right rail coordinate Z (+ 0x6400)
    uint8_t  reserved[16];    // 0x1A..0x29: Physics & boundary padding
    float    heading;         // 0x2A: Tangent heading angle in radians
    uint8_t  padding[28];     // 0x2E..0x49: Secondary attributes
    uint8_t  fork_flag;       // 0x4A: Branching directive (0 = mainline, 1 = left, 2 = right)
} TrackSplineNode;
```

---

## 5. Road Spline Chunks (`.TRI`)

### `TriChunk` (500 bytes per PLC object)
```c
typedef struct {
    uint8_t  road_code;       // 0x00: Surface material code
    int16_t  left_vertex;     // 0x01: Submesh vertex index for left boundary
    uint8_t  padding1[20];    // 0x03..0x16: Secondary surface attributes
    int16_t  right_vertex;    // 0x17: Submesh vertex index for right boundary
    uint8_t  padding2[84];    // 0x19..0x6C: Internal friction parameters
    uint8_t  fork_flag;       // 0x6D: Branching directive (0 = normal, 1 = left, 2 = right)
    uint8_t  reserved[390];   // 0x6E..0x1F3: Reserved padding
} TriChunk;
```

---

## 6. Dynamic Scenery Keyframe Animation (`.POS`)

### `PosKeyframe` (24 bytes)
```c
typedef struct {
    int32_t pos_x;            // 0x00: X position delta
    int32_t pos_y;            // 0x04: Y elevation delta
    int32_t pos_z;            // 0x08: Z position delta
    int32_t rot_x;            // 0x0C: Pitch angle (tenths of degree)
    int32_t rot_y;            // 0x10: Yaw angle (tenths of degree)
    int32_t rot_z;            // 0x14: Roll angle (tenths of degree)
} PosKeyframe;
```

---

## 7. Vehicle Dynamics & Raycast Suspension (`getsurf.c` / `vehicle.c`)

### `SurfaceRaycastResult`
```c
typedef struct {
    bool     hit;             // True if raycast intersected terrain triangle
    int32_t  triangle_idx;    // .SRF triangle index
    int32_t  material_id;     // Surface material identifier (0..7)
    double   elevation;       // Evaluated 3D surface elevation Y
    double   normal_x;        // Triangle surface normal X
    double   normal_y;        // Triangle surface normal Y
    double   normal_z;        // Triangle surface normal Z
    double   v0[3];           // Triangle world vertex 0 (X, Y, Z)
    double   v1[3];           // Triangle world vertex 1 (X, Y, Z)
    double   v2[3];           // Triangle world vertex 2 (X, Y, Z)
    double   friction;        // Surface friction coefficient (1.0 = tarmac)
    bool     is_boost_pad;    // Yellow accelerator boost pad flag
    bool     is_kill_plane;   // Water hazard or off-track kill plane
} SurfaceRaycastResult;
```

### `WheelPhysics`
```c
typedef struct {
    double   offset_x;        // Local chassis offset X relative to CG
    double   offset_z;        // Local chassis offset Z relative to CG
    double   world_x;         // Calculated wheel world X (rotated by yaw)
    double   world_y;         // Calculated wheel world Y (suspension contact)
    double   world_z;         // Calculated wheel world Z (rotated by yaw)
    double   contact_y;       // Surface elevation directly beneath wheel
    double   suspension_len;  // Suspension compression displacement
    double   friction;        // Surface friction at wheel contact
    int32_t  active_triangle; // Triangle index in contact (-1 if in air)
    bool     grounded;        // Ground contact flag
} WheelPhysics;
```

### `VehicleState`
```c
typedef struct VehicleState {
    // 3D Spatial Position and Velocities (World coordinates)
    double   x, y, z;
    double   vx, vy, vz;      // vy: negative is downward gravity
    
    // Chassis Orientation (Radians)
    double   yaw;             // Heading angle around Y axis
    double   pitch;           // Elevation tilt around X axis (arcsin clamped to +/-0.95)
    double   roll;            // Bank tilt around Z axis (arcsin clamped to +/-0.95)
    double   yaw_rate;        // Angular yaw velocity (rad/s)
    double   steering_angle;  // Active front wheel turn angle (rad)
    
    // Body-frame Velocity Components
    double   speed_long;      // Longitudinal forward velocity
    double   speed_lat;       // Lateral slip velocity
    
    // Normalized Driver Controls
    double   throttle_input;  // 0.0 to 1.0
    double   brake_input;     // 0.0 to 1.0
    double   steer_input;     // -1.0 (left) to +1.0 (right)
    bool     boost_input;     // Turbo boost activation request
    
    // Powertrain & Transmission
    int      car_index;       // Vehicle archetype index (0..7)
    int      gear;            // 0 = Reverse, 1 = 1st, 2 = 2nd
    double   engine_rpm;      // Engine RPM (1000..8000)
    
    // 4-Wheel Independent Raycast Suspension
    WheelPhysics wheels[4];   // 0 = FL, 1 = FR, 2 = RL, 3 = RR
    double   avg_ground_y;    // Average ground elevation beneath 4 wheels
    double   avg_friction;    // Average friction coefficient
    bool     airborne;        // True when wheels lose contact
    int      landing_timer;   // Ticks since landing impact
    
    // Turbo Boost Subsystem
    int      turbo_timer;     // Remaining turbo ticks (200 = ~2.8s)
    bool     turbo_active;    // Turbo boost active flag (1.75x thrust)
    
    VehicleParams params;
} VehicleState;
```

---

## 4. Audio Subsystem (`audio.h`, `main.c`)

### `SoundSample` (Loaded 16-bit PCM Audio Record)
```c
typedef struct {
    char        name[64];        // Filename identifier (e.g. "00_BOOST.WAV")
    int16_t    *data;            // Interleaved 16-bit signed PCM audio samples
    uint32_t    sample_count;    // Total sample frames
    uint32_t    sample_rate;     // Native sample rate in Hz (e.g. 22050, 11025)
    int         channels;        // 1 = Mono, 2 = Stereo
} SoundSample;
```

### `AudioVoice` (Software Mixer Channel Voice Record)
```c
typedef struct {
    bool         active;         // Voice is actively producing audio
    bool         looping;        // Loop continuously upon reaching end
    bool         paused;         // Playback temporarily halted
    SoundSample *sample;         // Pointer to source PCM sample
    uint32_t     playhead_fp;    // 16.16 fixed-point playback sample cursor
    uint32_t     step_fp;        // 16.16 fixed-point playback rate increment
    float        volume;         // Attenuation gain [0.0, 1.0]
    float        pan;            // Stereo balance [-1.0 left .. +1.0 right]
    float        pitch;          // Frequency multiplier (1.0 = native pitch)
} AudioVoice;
```

### `EngineAudio` (Synthesizer Profile & Curve Record)
```c
typedef struct {
    int32_t     curve[200];      // 800-byte ENGINE.INF frequency curve
    SoundSample sample_low;      // Looping low-RPM rumble (00_A.WAV)
    SoundSample sample_high;     // Looping high-RPM whine (01_A.WAV)
    int         voice_low;       // Active mixer channel for low-RPM sound
    int         voice_high;      // Active mixer channel for high-RPM sound
    bool        initialized;     // Initialization success flag
} EngineAudio;
```

---

## 5. Dynamic Chase Camera (`renderer.h`, `camera.c`)

### `CameraMode`
```c
typedef enum {
    CAMERA_MODE_CLASSIC = 0,     // 1997 authentic default isometric chase
    CAMERA_MODE_CLOSE   = 1,     // Close-in dynamic chase
    CAMERA_MODE_FAR     = 2,     // Distant high-angle tactical view
    CAMERA_MODE_BUMPER  = 3,     // Low bumper / hood perspective
    CAMERA_MODE_ORBIT   = 4      // Free orbiting debug / spectator camera
} CameraMode;
```

### `Camera3D`
```c
typedef struct {
    Vec3        position;        // Eye position in world space
    Vec3        target;          // Focus target in world space
    float       yaw;             // Azimuth angle in degrees
    float       pitch;           // Elevation angle in degrees
    float       distance;        // Distance to target
    float       fov;             // Field of view in degrees
    float       aspect;          // Viewport aspect ratio
    float       near_plane;      // Near clipping plane distance
    float       far_plane;       // Far clipping plane distance
    Mat4        view_matrix;     // World-to-view transform
    Mat4        proj_matrix;     // View-to-clip transform
    CameraMode  mode;            // Active camera mode
    float       smooth_yaw;      // Filtered yaw for lag damping
    float       smooth_pitch;    // Filtered pitch for terrain slope damping
    Vec3        smooth_target;   // Filtered lookahead target position
} Camera3D;
```

