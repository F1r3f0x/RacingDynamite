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

#ifndef IGNITION_RENDERER_H
#define IGNITION_RENDERER_H

#include "ignition/types.h"
#include "ignition/formats.h"
#include <stdint.h>
#include <stdbool.h>

struct VehicleState;
typedef struct VehicleState VehicleState;

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    RENDERER_BACKEND_LISA3D_SOFTWARE = 0, // Authentic 1997 UDS Lisa3D software rasterizer
    RENDERER_BACKEND_GLIDE_3DFX      = 1, // 3dfx Glide mode (IGN_3DFX.EXE / Voodoo emulation)
    RENDERER_BACKEND_DIRECT3D        = 2  // Direct3D hardware acceleration (IGN_D3D.EXE)
} RendererBackendType;

typedef struct {
    RendererBackendType backend_type;
    
    // Granular Software / Lisa3D Authenticity Toggles
    bool authentic_fixed_point_uv; // 16.16 fixed point UV stepping vs exact floating point
    bool authentic_depth_buckets;  // 6,000 depth buckets vs floating point Z-buffer
    bool authentic_dithering;      // 1997 Lisa3D dither matrix
    bool force_8bit_paletted;      // Authentic 8bpp color lookup vs 32-bit RGBA
    bool render_panoramas;         // 64KB cylindrical horizon background blitting
    
    // Resolution mode
    int  internal_width;           // 640 or 320
    int  internal_height;          // 480 or 200
} RendererOptions;

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    float u, v;
} Vec2;

typedef struct {
    Vec3     pos;             // World or view position (X, Y, Z)
    Vec2     uv;              // Normalized texture coordinates (0.0 to 1.0 within page)
    float    light;           // Lighting factor (0.0 to 1.0)
    uint32_t page_offset;     // Byte offset of 64KB page within texture buffer
    bool     is_transparent;  // True for opcodes 0x12, 0x13, 0x16, 0x17 (color key 0)
    bool     is_shadow_blend; // True for opcodes 0x13, 0x17 (passes through SHD table)
    uint8_t  color;           // Fallback flat color index
} Vertex3D;

typedef enum {
    CAMERA_MODE_CLASSIC    = 0, // Authentic 1997 Elevated Isometric Follow Camera (default)
    CAMERA_MODE_CLOSE      = 1, // Close Follow Chase Camera
    CAMERA_MODE_FAR        = 2, // Far Elevated Chase Camera
    CAMERA_MODE_BUMPER     = 3, // Hood / Front Bumper View
    CAMERA_MODE_FREE_ORBIT = 4  // Free orbit & pan inspection camera
} CameraMode;

typedef struct {
    Vec3       position;
    Vec3       target;
    float      yaw;          // Degrees
    float      pitch;        // Degrees
    float      distance;     // Chase distance from target
    float      fov;          // Field of view in degrees (e.g. 60.0f)
    float      aspect;       // Width / Height (640 / 480 = 1.333f)
    float      near_z;
    float      far_z;

    CameraMode mode;         // Active camera view mode
    float      target_yaw;   // Target vehicle yaw angle in degrees
    float      target_pitch; // Target terrain/vehicle pitch in degrees
    Vec3       lookahead;    // Dynamic velocity lookahead offset
    float      lag_factor;   // Azimuth smoothing lag (authentic 0.125f)
} Camera3D;

typedef struct {
    int          width;
    int          height;
    uint8_t     *framebuffer;   // 8bpp color buffer (width * height)
    float       *zbuffer;       // Floating-point depth buffer (width * height)
    
    const TexData *active_texture;  // Current 1024x1024 texture page
    const TabData *active_shading;  // 64KB shading lookup table
    const ShdData *active_shadow;   // 64KB shadow / alpha lookup table

    RendererOptions options;        // Active renderer backend & fidelity options
} Renderer3D;

/**
 * @brief Retrieves default recommended renderer configuration (authentic Lisa3D software rasterizer).
 */
void Renderer_GetDefaultOptions(RendererOptions *opts);

/**
 * @brief Updates renderer configuration and options.
 */
void Renderer_SetOptions(Renderer3D *r, const RendererOptions *opts);

/**
 * @brief Initializes 3D renderer state, framebuffers, depth buffer, and default options.
 * @original FUN_0043e2a0 (MAINDOS_32BIT.EXE @ 0x0043e2a0, lisa3d.c)
 * @fidelity ADAPTED
 */
bool Renderer_Init(Renderer3D *r, int width, int height);

/**
 * @brief Releases renderer framebuffers and allocated depth buffers.
 * @original FUN_00412530 (MAINDOS_32BIT.EXE @ 0x00412530, main.c)
 * @fidelity ADAPTED
 */
void Renderer_Shutdown(Renderer3D *r);

/**
 * @brief Clears virtual framebuffer and depth buffer.
 * @original FUN_004468d0 (MAINDOS_32BIT.EXE @ 0x004468d0, lisa3d.c)
 * @fidelity ADAPTED
 */
void Renderer_Clear(Renderer3D *r, uint8_t clear_color, float clear_depth);

// Set active assets
void Renderer_SetTexture(Renderer3D *r, const TexData *tex);
void Renderer_SetShading(Renderer3D *r, const TabData *tab);
void Renderer_SetShadow(Renderer3D *r, const ShdData *shd);

/**
 * @brief Initializes camera viewing volume and aspect ratio.
 * @original FUN_00436990 (MAINDOS_32BIT.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_Init(Camera3D *cam, float aspect);

/**
 * @brief Computes Cartesian camera position from spherical orbital parameters.
 * @original FUN_00436990 (MAINDOS_32BIT.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_Update(Camera3D *cam, float delta_time);

/**
 * @brief Orbits camera around target point with yaw, pitch, and zoom constraints.
 * @original FUN_00436990 (MAINDOS_32BIT.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_Orbit(Camera3D *cam, float delta_yaw, float delta_pitch, float delta_dist);

/**
 * @brief Pans camera target in world horizontal plane.
 * @original FUN_00436990 (MAINDOS_32BIT.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_Pan(Camera3D *cam, float delta_forward, float delta_right, float delta_up);

/**
 * @brief Switch active camera view mode and reconfigure baseline orbital parameters.
 * @original FUN_00436990 (MAINDOS_32BIT.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 *
 * @param cam Camera state
 * @param mode Target view mode
 */
void Camera_SetMode(Camera3D *cam, CameraMode mode);

/**
 * @brief Cycle to next in-race camera view mode.
 * @fidelity INFRASTRUCTURE
 *
 * @param cam Camera state
 */
void Camera_CycleMode(Camera3D *cam);

/**
 * @brief Update camera position and heading following a vehicle with authentic lag and slope tracking.
 * @original FUN_0043c910 (MAINDOS_32BIT.EXE @ 0x0043c910, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 *
 * @param cam Camera state
 * @param veh Vehicle dynamics state
 * @param delta_time Timestep delta
 * @param fixes Active game fix options
 */
void Camera_UpdateFollowChase(Camera3D *cam, const VehicleState *veh, float delta_time, const GameFixOptions *fixes);

/**
 * @brief Rasterizes single 3D triangle with perspective texture mapping, shading, and alpha/shadow blend.
 * @original FUN_0044d550 (MAINDOS_32BIT.EXE @ 0x0044d550, lisa3d.c)
 * @fidelity ADAPTED
 * @deviation DEV-005
 * @fix_category FIX_CAT_RENDERER
 */
void Renderer_DrawTriangle(Renderer3D *r, const Camera3D *cam,
                           const Vertex3D *v0, const Vertex3D *v1, const Vertex3D *v2);

void Renderer_DrawTrackSurface(Renderer3D *r, const Camera3D *cam, const SrfData *srf);
void Renderer_DrawPlacedObjects(Renderer3D *r, const Camera3D *cam, const PlcData *plc);

/**
 * @brief Renders all scenery and track meshes placed by .PLC file across two passes.
 * @original FUN_00416250 (MAINDOS_32BIT.EXE @ 0x00416250, main.c)
 * @fidelity ADAPTED
 */
void Renderer_DrawTrackMesh(Renderer3D *r, const Camera3D *cam, const MshData *msh, const PlcData *plc);

// 3D Line and Waypoint visualization
void Renderer_DrawLine3D(Renderer3D *r, const Camera3D *cam, Vec3 p0, Vec3 p1, uint8_t color);
void Renderer_DrawWaypoints(Renderer3D *r, const Camera3D *cam, const TrackWaypoints *wp);

typedef struct VehicleState VehicleState;

/**
 * @brief Renders vehicle 3D model with chassis roll/pitch/yaw transforms and ground alignment.
 * @original FUN_0041d190 (MAINDOS_32BIT.EXE @ 0x0041d190, main.c / lisa3d.c)
 * @fidelity EXTENDED
 * @deviation DEV-002
 * @fix_category FIX_CAT_ELEVATION
 */
void Renderer_DrawCar(Renderer3D *r, const Camera3D *cam, const MshData *cars_msh, const TexData *cars_tex, const VehicleState *veh);

/**
 * @brief Prints Lisa 2 Development System build banner and timestamp.
 * @original FUN_0045b4f0 (MAINDOS_32BIT.EXE @ 0x0045b4f0, lisa3d.c)
 * @fidelity EXACT
 */
void Renderer_PrintVersion(void);

/**
 * @brief Renders 360-degree cylindrical horizon backdrop sampling 64KB (256x256) .PAN texture.
 * @original FUN_00438210 (MAINDOS_32BIT.EXE @ 0x00438210, lisa3d.c)
 * @fidelity ADAPTED
 */
void Renderer_RenderPanorama(Renderer3D *r, const Camera3D *cam, const uint8_t *pan_pixels);

#ifdef __cplusplus
}
#endif

#endif // IGNITION_RENDERER_H
