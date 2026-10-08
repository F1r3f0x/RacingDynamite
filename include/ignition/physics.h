/*
 * Racing Dynamite - Modern open-source source port of Ignition (1997)
 * Copyright (C) 2026 Patricio Labin Correa (@F1r3f0x)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef IGNITION_PHYSICS_H
#define IGNITION_PHYSICS_H

#include "ignition/types.h"
#include "ignition/formats.h"
#include "ignition/renderer.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Authentic engine constants extracted from MAINDOS.EXE
#define CAR_ARCHETYPE_COUNT     11              // 11 authentic vehicle archetypes (0x00494a70, 0x00495800)
#define PHYSICS_DT_SEC          (1.0 / 72.0)    // 0.013888889 s (72 Hz tick rate, 0x004792f0)

#define PHYSICS_SCALE_FACTOR    21.76           // World coordinate scaling factor (0x00479af0)
#define PHYSICS_GRAVITY         9.81            // Gravitational acceleration (0x004792c8)
#define PHYSICS_GRAVITY_TICK    0.2             // Vertical velocity gravity delta per tick (0x00479b50)
#define PHYSICS_RIDE_HEIGHT     5.0             // Ground clearance equilibrium offset (0x00479b58)
#define PHYSICS_MAX_SLOPE_SIN   0.95            // Arcsin clamping limit for pitch/roll (0x00479ca0)
#define PHYSICS_RATE_LIMIT_RAD  0.1             // Max pitch/roll angle change per tick (0x00479cb0)
#define PHYSICS_WORLD_OFFSET    25600.0         // 50 * 512 units center offset (0x00479c48)

// Surface raycast query result from getsurf
typedef struct {
    bool     hit;
    int32_t  triangle_idx;
    int32_t  material_id;
    double   elevation;             // Evaluated Y surface elevation
    double   normal_x;              // Triangle plane normal X
    double   normal_y;              // Triangle plane normal Y
    double   normal_z;              // Triangle plane normal Z
    double   v0[3];                 // World vertex 0 (X, Y, Z)
    double   v1[3];                 // World vertex 1 (X, Y, Z)
    double   v2[3];                 // World vertex 2 (X, Y, Z)
    double   friction;              // Surface friction coefficient (1.0 = tarmac)
    bool     is_boost_pad;          // Turbo boost accelerator pad
    bool     is_kill_plane;         // Water hazard / fall off track
} SurfaceRaycastResult;

// Independent wheel physics state (FL, FR, RL, RR)
typedef struct {
    double   offset_x;              // Local chassis offset X relative to CG
    double   offset_z;              // Local chassis offset Z relative to CG
    double   world_x;               // Calculated wheel world X
    double   world_y;               // Calculated wheel world Y (suspension contact)
    double   world_z;               // Calculated wheel world Z
    double   contact_y;             // Surface elevation directly beneath wheel
    double   suspension_len;        // Suspension compression displacement
    double   friction;              // Surface friction at wheel contact
    int32_t  active_triangle;       // Triangle index in contact (-1 if in air)
    bool     grounded;              // Ground contact flag
} WheelPhysics;

// Vehicle model archetype parameters
typedef struct {
    double   mass;                  // Chassis mass (kg, ~1000.0)
    double   inertia_z;             // Yaw moment of inertia (~1500.0)
    double   wheelbase_a;           // Distance from CG to front axle (m)
    double   wheelbase_b;           // Distance from CG to rear axle (m)
    double   track_width;           // Distance between left and right wheels (m)
    double   max_steer_angle;       // Maximum steering wheel lock (radians, ~0.45)
    double   engine_power;          // Forward thrust multiplier
    double   brake_power;           // Braking deceleration multiplier
    double   chassis_scale;         // Wheel spread / chassis scale multiplier
} VehicleParams;

// Complete dynamic vehicle state
typedef struct VehicleState {
    // 3D Spatial Position and Velocities (World coordinates)
    double   x;
    double   y;
    double   z;
    double   vx;
    double   vy;                    // Vertical velocity (+ downward in screen space)
    double   vz;
    
    // Chassis Orientation (Radians)
    double   yaw;                   // Heading angle around Y axis
    double   pitch;                 // Elevation tilt around X axis
    double   roll;                  // Bank tilt around Z axis
    double   yaw_rate;              // Angular yaw velocity (rad/s)
    double   steering_angle;        // Active front wheel turn angle (rad)
    
    // Body-frame Velocity Components
    double   speed_long;            // Forward velocity along vehicle longitudinal axis
    double   speed_lat;             // Lateral slip velocity
    
    // Player / AI Inputs (Normalized)
    double   throttle_input;        // 0.0 to 1.0
    double   brake_input;           // 0.0 to 1.0
    double   steer_input;           // -1.0 (left) to +1.0 (right)
    bool     boost_input;           // Turbo boost activation request
    
    // Powertrain & Transmission
    int      car_index;             // Vehicle archetype index (0..7)
    int      gear;                  // 0 = Reverse, 1 = 1st, 2 = 2nd
    double   engine_rpm;            // Current engine RPM (sound pitch)
    
    // 4-Wheel Independent Raycast Suspension
    // 0 = Front-Left, 1 = Front-Right, 2 = Rear-Left, 3 = Rear-Right
    WheelPhysics wheels[4];
    double   avg_ground_y;          // Average elevation beneath all 4 wheels
    double   avg_friction;          // Average friction coefficient of active surface
    bool     airborne;              // True when all wheels lose contact
    int      landing_timer;         // Ticks since landing impact (sound triggering)
    
    // Turbo Boost Subsystem
    int      turbo_timer;           // Remaining turbo boost duration ticks
    bool     turbo_active;          // Turbo boost active flag

    // Transition state tracking (for event logging)
    bool     prev_airborne;
    int      prev_gear;
    bool     prev_turbo_active;
    
    // Vehicle Archetype Parameters
    VehicleParams params;
} VehicleState;

/**
 * @brief Output detailed telemetry for the vehicle state.
 * @fidelity INFRASTRUCTURE
 *
 * @param veh Active vehicle dynamic state
 * @param tick_num Simulation tick sequence number
 */
void Vehicle_LogTelemetry(const VehicleState *veh, uint32_t tick_num);

/**
 * @brief Raycast world coordinate against .SRF terrain heightfield and collision grid.
 * @original FUN_00412fc0 (MAINDOS.EXE @ 0x00412fc0, getsurf.c)
 * @fidelity EXTENDED
 * @deviation DEV-001 (Boundary safety clamp preventing memory fault on off-track jumps)
 * @fix_category FIX_CAT_NOCLIP
 * @notes Uses 50 * 512 unit coordinate offset matching DAT_00479c48.
 *
 * @param srf Loaded surface data
 * @param plc Scenery placement table for model lookup
 * @param msh Track mesh geometry data
 * @param world_x Vehicle wheel world X coordinate
 * @param world_z Vehicle wheel world Z coordinate
 * @param ref_y Reference chassis elevation
 * @param result Output raycast intersection result
 * @param fixes Active game fix options (or NULL for authentic 1997 behavior)
 * @return true if candidate surface triangle was hit
 */
bool Surface_Raycast(const SrfData *srf, const PlcData *plc, const MshData *msh,
                     double world_x, double world_z, double ref_y,
                     SurfaceRaycastResult *result, const GameFixOptions *fixes);

/**
 * @brief Initialize vehicle physics state, dimensions, and starting grid placement.
 * @original FUN_0041b470 (MAINDOS.EXE @ 0x0041b470, main.c)
 * @fidelity ADAPTED
 *
 * @param veh Vehicle state structure to initialize
 * @param car_index Vehicle archetype index (0..7)
 * @param start_x Starting world X coordinate
 * @param start_y Starting world Y coordinate
 * @param start_z Starting world Z coordinate
 * @param start_yaw Starting heading angle in radians
 */
void Vehicle_Init(VehicleState *veh, int car_index,
                  double start_x, double start_y, double start_z, double start_yaw);

/**
 * @brief Apply driver control inputs (throttle, brake, steering, turbo boost).
 * @fidelity INFRASTRUCTURE
 *
 * @param veh Target vehicle dynamic state
 * @param throttle Normalized acceleration demand (0.0 to 1.0)
 * @param brake Normalized braking demand (0.0 to 1.0)
 * @param steer Normalized steering demand (-1.0 left to +1.0 right)
 * @param boost Turbo boost activation flag
 */
void Vehicle_ApplyInput(VehicleState *veh, double throttle, double brake, double steer, bool boost);

/**
 * @brief Perform a single fixed-timestep 72 Hz physics integration tick.
 * @original FUN_00424570 (MAINDOS.EXE @ 0x00424570, vehicle.c)
 * @fidelity EXTENDED
 * @deviation DEV-003 (Mountain wall climbing steep gradient adhesion clamp)
 * @fix_category FIX_CAT_NOCLIP
 *
 * @param veh Target vehicle dynamic state
 * @param srf Active track surface collision data
 * @param plc Active track placement table
 * @param msh Active track 3D mesh
 * @param dt Timestep in seconds (authentic: 1.0 / 72.0)
 * @param fixes Active game fix options (or NULL for authentic 1997 behavior)
 */
void Vehicle_Update(VehicleState *veh, const SrfData *srf, const PlcData *plc, const MshData *msh,
                    double dt, const GameFixOptions *fixes);

/**
 * @brief Compute smooth isometric chase camera parameters targeting the vehicle.
 * @original FUN_00436990 (MAINDOS.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004 (Right-handed camera basis vector normalization)
 * @fix_category FIX_CAT_CAMERA
 *
 * @param veh Active vehicle dynamic state
 * @param cam Target camera structure to populate
 */
void Vehicle_GetChaseCamera(const VehicleState *veh, Camera3D *cam);

#ifdef __cplusplus
}
#endif

#endif // IGNITION_PHYSICS_H
