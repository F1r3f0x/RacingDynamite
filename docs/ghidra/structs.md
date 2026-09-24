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

### `VehicleState` (18,508 bytes / `0x484C` allocated in `main.c`)
```c
typedef struct VehicleState {
    double pos_x;                  // 0x0000: World X coordinate
    double pos_y;                  // 0x0008: World Y elevation
    double pos_z;                  // 0x0010: World Z coordinate
    double vel_x;                  // 0x0018: Linear velocity X
    double vel_y;                  // 0x0020: Linear velocity Y
    double vel_z;                  // 0x0028: Linear velocity Z
    double accel_x;                // 0x0030: Forward traction acceleration X
    double accel_y;                // 0x0038: Forward traction acceleration Y
    double accel_z;                // 0x0040: Forward traction acceleration Z
    double rear_axle_pos_x;        // 0x0048: Rear axle center X
    double rear_axle_pos_z;        // 0x0050: Rear axle center Z
    double rear_axle_vel_x;        // 0x0058: Rear axle velocity X
    double rear_axle_vel_z;        // 0x0060: Rear axle velocity Z
    double front_axle_pos_x;       // 0x0068: Front axle center X
    double front_axle_pos_z;       // 0x0070: Front axle center Z
    double front_axle_vel_x;       // 0x0078: Front axle velocity X
    double front_axle_vel_z;       // 0x0080: Front axle velocity Z
    double angular_drag_x;         // 0x0088: Angular drag damping X
    double angular_drag_y;         // 0x0090: Angular drag damping Y
    double angular_drag_z;         // 0x0098: Angular drag damping Z
    double wheel_angle_fl;         // 0x00A0: Front-left wheel steering angle
    uint8_t gap_a8[8];             // 0x00A8: Padding
    double wheel_angle_fr;         // 0x00B0: Front-right wheel steering angle
    double wheel_rot_vel_fl;       // 0x00B8: Front-left wheel rotational velocity
    double wheel_rot_vel_fr;       // 0x00C0: Front-right wheel rotational velocity
    uint8_t gap_c8[48];            // 0x00C8: Internal physics state
    double angle_yaw;              // 0x00F8: Heading orientation angle (radians)
    double angle_pitch;            // 0x0100: Longitudinal pitch inclination angle
    double angular_vel_roll;       // 0x0108: Roll angular velocity
    double angle_roll;             // 0x0110: Lateral roll tilt angle
    double steering_angle;         // 0x0118: Current front wheel steering angle
    double ground_y;               // 0x0120: Ground contact surface plane elevation
    double ground_y_rear;          // 0x0128: Rear contact surface plane elevation
    uint8_t gap_130[32];           // 0x0130: Internal collision state
    int32_t wheel_surface_id[4];   // 0x0150: Ground triangle ID under each of 4 wheels
    uint8_t gap_160[16];           // 0x0160: Padding
    int32_t wheel_surface_info[64];// 0x0170: 4 wheels x 16 ints surface probe cache
    int32_t is_airborne;           // 0x0270: Airborne status flag (1 = wheels off ground)
    int32_t landing_impact;        // 0x0274: Hard landing impact trigger flag
    int32_t landing_flag;          // 0x0278: Touchdown trigger flag
    int32_t current_gear;          // 0x027C: Active transmission gear (1 to 5, -1 reverse)
    uint8_t gap_280[8];            // 0x0280: Padding
    double target_rpm;             // 0x0288: Target engine RPM
    double engine_rpm;             // 0x0290: Smoothed engine RPM
    int32_t throttle_input;        // 0x0298: Throttle command state
    int32_t brake_input;           // 0x029C: Brake command state
    uint8_t gap_2a0[88];           // 0x02A0: Internal input buffer
    int32_t handbrake;             // 0x02F8: Emergency handbrake toggle
    uint8_t gap_2fc[64];           // 0x02FC: Padding
    int32_t surface_type;          // 0x033C: Road material type under chassis
    uint8_t gap_340[4];            // 0x0340: Padding
    int32_t checkpoint_pass1;      // 0x0344: Sector 1 checkpoint trigger
    int32_t checkpoint_pass2;      // 0x0348: Sector 2 checkpoint trigger
    int32_t checkpoint_pass3;      // 0x034C: Sector 3 checkpoint trigger
    int32_t checkpoint_counter;    // 0x0350: Next required checkpoint sequence index
    int32_t crash_flag1;           // 0x0354: Elimination / blown vehicle flag
    int32_t crash_flag2;           // 0x0358: Severe crash impact state
    int32_t crash_flag3;           // 0x035C: Roll-over / upside-down crash state
    uint8_t gap_360[4];            // 0x0360: Padding
    int32_t current_node_idx;      // 0x0364: Current track road sequence node index
    int32_t target_node_idx;       // 0x0368: Target track road sequence node index
    int32_t road_surface_node;     // 0x036C: Nearest track road spline node index
    uint8_t gap_370[44];           // 0x0370: Waypoint tracking state
    int32_t car_model_id;          // 0x039C: Vehicle archetype model ID
    int32_t race_rank;             // 0x03A0: Current race standing rank (1 to 6)
    double lap_time;               // 0x03A4: Active lap time in seconds
    uint8_t gap_3ac[380];          // 0x03AC: Split times and AI telemetry
    int32_t turbo_active;          // 0x0528: Turbo boost activation flag
    uint8_t gap_52c[44];           // 0x052C: Turbo recharge timers
    int32_t mesh_damage_flag;      // 0x0558: Mesh damage / deformation status flag
    int32_t detached_wheel_mask;   // 0x055C: Bitmask of detached / flying wheels
    int32_t wreck_debris_flag;     // 0x0560: Wreck debris emission trigger flag
    uint8_t gap_564[32];           // 0x0564: Damage kinematics state
    double chassis_roll_spring;    // 0x0584: Body roll spring equilibrium angle
    double chassis_pitch_spring;   // 0x058C: Body pitch spring equilibrium angle
    uint8_t gap_594[24];           // 0x0594: Padding
    int32_t front_axle_offset;     // 0x05AC: Front axle longitudinal offset distance from center
    int32_t rear_axle_offset;      // 0x05B0: Rear axle longitudinal offset distance from center
    uint8_t gap_5b4[1000];         // 0x05B4: Engine audio frequency & volume curves (ENGINE.INF)
    int32_t voice_engine;          // 0x099C: DirectSound audio channel voice handle for engine
    int32_t voice_skid;            // 0x09A0: DirectSound audio channel voice handle for tire skid
    uint8_t gap_9a4[16040];        // 0x09A4: Collision mesh bounding tree and geometry cache
} VehicleState;
```

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

---

## 10. Dynamic Objects & Chase Camera (`fx.c`)

### `DynamicObjectTransform` (32 bytes / 8 x `int32_t`)
```c
typedef struct DynamicObjectTransform {
    int32_t handle;                    // 0x00: Engine entity / object handle
    int32_t pos_x;                     // 0x04: World position X
    int32_t pos_y;                     // 0x08: World position Y (elevation)
    int32_t pos_z;                     // 0x0C: World position Z
    int32_t rot_x;                     // 0x10: Pitch rotation (tenths of a degree)
    int32_t rot_y;                     // 0x14: Yaw rotation (tenths of a degree)
    int32_t rot_z;                     // 0x18: Roll rotation (tenths of a degree)
    int32_t is_placed;                 // 0x1C: Active / placed status flag
} DynamicObjectTransform;
```

### `RoadSequenceNode` (24 bytes)
```c
typedef struct RoadSequenceNode {
    int32_t road_index;                // 0x00: Forward road chunk index
    double  pitch1;                    // 0x04: Forward road pitch angle (radians)
    int32_t road_index_rev;            // 0x0C: Reverse road chunk index
    double  pitch2;                    // 0x10: Reverse road pitch angle (radians)
} RoadSequenceNode;
```

### `VehicleConfig` (200 bytes / 0xC8 bytes)
```c
typedef struct VehicleConfig {
    double  cam_height;                // 0x00: Camera height/elevation
    double  camera_yaw;                // 0x08: Chase camera yaw angle (radians)
    uint8_t pad_10[0x28 - 0x10];       // 0x10
    double  camera_pitch;              // 0x28: Chase camera pitch angle (radians)
    double  chase_cam_dist_factor;     // 0x30: Camera zoom/distance factor
    int32_t cam_param1;                // 0x38: Secondary camera parameter 1
    int32_t cam_param2;                // 0x3C: Secondary camera parameter 2
    uint8_t pad_40[0x48 - 0x40];       // 0x40
    double  pitch_offset;              // 0x48: Dynamic pitch/shake offset
    int32_t chase_target_height;       // 0x50: Target camera height
    int32_t chase_camera_distance;     // 0x54: Base camera distance
    int32_t viewport_h;                // 0x58: Viewport height
    int32_t viewport_w;                // 0x5C: Viewport width
    int32_t unused_60;                 // 0x60
    float   camera_fov_preset;         // 0x64: FOV preset
    double  yaw_lag_angle;             // 0x68: Yaw in tenths of degrees (0-3599)
    double  pitch_lag_angle;           // 0x70: Pitch in tenths of degrees (0-3599)
    double  target_pos_x;              // 0x78: Interpolated lookat target X
    double  target_pos_y;              // 0x80: Interpolated lookat target Y
    double  target_pos_z;              // 0x88: Interpolated lookat target Z
    double  cam_pos_x;                 // 0x90: Camera eye position X
    double  cam_pos_y;                 // 0x98: Camera eye position Y
    double  cam_pos_z;                 // 0xA0: Camera eye position Z
    double  vehicle_yaw;               // 0xA8: Copied vehicle yaw (radians)
    double  vehicle_pitch;             // 0xB0: Copied vehicle pitch (radians)
    double  camera_yaw_desired;        // 0xB8: Desired camera yaw (vehicle_yaw + PI)
    double  elevation_smooth_offset;   // 0xC0: Smoothed road elevation/incline offset
} VehicleConfig;
```

