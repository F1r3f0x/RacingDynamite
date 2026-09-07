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

#include "ignition/renderer.h"
#include "ignition/physics.h"
#include <math.h>

#define DEG2RAD (3.14159265358979323846f / 180.0f)
#define RAD2DEG (180.0f / 3.14159265358979323846f)

/**
 * @brief Initializes camera viewing volume, mode, and aspect ratio.
 * @original FUN_00436990 (IGN_WIN.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004 (Right-handed camera basis alignment)
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_Init(Camera3D *cam, float aspect) {
    if (!cam) return;
    cam->target.x = 0.0f;
    cam->target.y = -500.0f;
    cam->target.z = 0.0f;

    cam->yaw = 45.0f;
    cam->pitch = 32.0f;
    cam->distance = 1100.0f;

    cam->fov = 60.0f;
    cam->aspect = aspect > 0.0f ? aspect : (640.0f / 480.0f);
    cam->near_z = 20.0f;
    cam->far_z = 60000.0f;

    cam->mode = CAMERA_MODE_CLASSIC;
    cam->target_yaw = 45.0f;
    cam->target_pitch = 0.0f;
    cam->lookahead.x = 0.0f;
    cam->lookahead.y = 0.0f;
    cam->lookahead.z = 0.0f;
    cam->lag_factor = 0.125f; // Authentic |DAT_0047a720| = |-0.125| = 0.125 (stored positive; applied to signed yaw_diff)

    Camera_Update(cam, 0.0f);
}

/**
 * @brief Switch active camera view mode and reconfigure baseline orbital parameters.
 * @original FUN_00436990 (IGN_WIN.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_SetMode(Camera3D *cam, CameraMode mode) {
    if (!cam) return;
    cam->mode = mode;

    switch (mode) {
        case CAMERA_MODE_CLASSIC:
            cam->distance = 1100.0f;
            cam->pitch = 32.0f;
            cam->fov = 60.0f;
            cam->lag_factor = 0.125f; // Authentic |DAT_0047a720| = 0.125
            break;
        case CAMERA_MODE_CLOSE:
            cam->distance = 650.0f;
            cam->pitch = 24.0f;
            cam->fov = 65.0f;
            cam->lag_factor = 0.20f;
            break;
        case CAMERA_MODE_FAR:
            cam->distance = 1900.0f;
            cam->pitch = 40.0f;
            cam->fov = 55.0f;
            cam->lag_factor = 0.08f;
            break;
        case CAMERA_MODE_BUMPER:
            cam->distance = 50.0f;
            cam->pitch = 5.0f;
            cam->fov = 75.0f;
            cam->lag_factor = 0.95f;
            break;
        case CAMERA_MODE_FREE_ORBIT:
            cam->distance = 3000.0f;
            cam->pitch = 35.0f;
            cam->lag_factor = 1.0f;
            break;
    }

    Camera_Update(cam, 0.0f);
}

/**
 * @brief Cycle to next in-race camera view mode.
 * @fidelity INFRASTRUCTURE
 */
void Camera_CycleMode(Camera3D *cam) {
    if (!cam) return;
    CameraMode next = (cam->mode + 1) % 4; // Cycles Classic -> Close -> Far -> Bumper
    Camera_SetMode(cam, next);
}

/**
 * @brief Orbits camera around target point with yaw, pitch, and zoom constraints.
 * @original FUN_00436990 (IGN_WIN.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_Orbit(Camera3D *cam, float delta_yaw, float delta_pitch, float delta_dist) {
    if (!cam) return;

    cam->yaw += delta_yaw;
    while (cam->yaw >= 360.0f) cam->yaw -= 360.0f;
    while (cam->yaw < 0.0f) cam->yaw += 360.0f;

    cam->pitch += delta_pitch;
    if (cam->pitch < 10.0f) cam->pitch = 10.0f;
    if (cam->pitch > 85.0f) cam->pitch = 85.0f;

    cam->distance += delta_dist;
    if (cam->distance < 50.0f) cam->distance = 50.0f;
    if (cam->distance > 25000.0f) cam->distance = 25000.0f;

    Camera_Update(cam, 0.0f);
}

/**
 * @brief Computes Cartesian camera position from spherical orbital parameters.
 * @original FUN_00436990 (IGN_WIN.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_Update(Camera3D *cam, float delta_time) {
    (void)delta_time;
    if (!cam) return;

    float yaw_rad = cam->yaw * DEG2RAD;
    float pitch_rad = cam->pitch * DEG2RAD;

    float cos_p = cosf(pitch_rad);
    float sin_p = sinf(pitch_rad);
    float cos_y = cosf(yaw_rad);
    float sin_y = sinf(yaw_rad);

    cam->position.x = cam->target.x + cam->distance * cos_p * sin_y;
    cam->position.y = cam->target.y + cam->distance * sin_p;
    cam->position.z = cam->target.z + cam->distance * cos_p * cos_y;
}

/**
 * @brief Pans camera target in world horizontal plane.
 * @original FUN_00436990 (IGN_WIN.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_Pan(Camera3D *cam, float delta_forward, float delta_right, float delta_up) {
    if (!cam) return;

    float yaw_rad = cam->yaw * DEG2RAD;
    float sin_y = sinf(yaw_rad);
    float cos_y = cosf(yaw_rad);

    // Forward vector in horizontal XZ plane (direction from camera towards target)
    float fwd_x = -sin_y;
    float fwd_z = -cos_y;
    // Right vector in horizontal XZ plane
    float right_x = cos_y;
    float right_z = -sin_y;

    cam->target.x += (fwd_x * delta_forward + right_x * delta_right);
    cam->target.z += (fwd_z * delta_forward + right_z * delta_right);
    cam->target.y += delta_up;

    Camera_Update(cam, 0.0f);
}

/**
 * @brief Update camera position and heading following a vehicle with authentic lag and slope tracking.
 * @original FUN_0043c910 (IGN_WIN.EXE @ 0x0043c910, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004
 * @fix_category FIX_CAT_CAMERA
 */
void Camera_UpdateFollowChase(Camera3D *cam, const VehicleState *veh, float delta_time, const GameFixOptions *fixes) {
    (void)delta_time;
    (void)fixes;
    if (!cam || !veh) return;

    if (cam->mode == CAMERA_MODE_FREE_ORBIT) {
        Camera_Update(cam, delta_time);
        return;
    }

    // Velocity lookahead lead: project target point along vehicle forward heading
    float lookahead_distance = (float)veh->speed_long * 3.5f;
    if (lookahead_distance > 180.0f) lookahead_distance = 180.0f;
    if (lookahead_distance < -60.0f) lookahead_distance = -60.0f;
    cam->lookahead.x = sinf((float)veh->yaw) * lookahead_distance;
    cam->lookahead.z = cosf((float)veh->yaw) * lookahead_distance;
    cam->lookahead.y = 0.0f;

    // Center target on car chassis with lookahead offset
    cam->target.x = (float)veh->x + cam->lookahead.x;
    cam->target.z = (float)veh->z + cam->lookahead.z;
    cam->target.y = (float)veh->y + ((cam->mode == CAMERA_MODE_BUMPER) ? 12.0f : 40.0f);

    // Vehicle heading angle in degrees (positioned behind car at +180 deg)
    float target_yaw = (float)(veh->yaw * RAD2DEG) + 180.0f;
    cam->target_yaw = target_yaw;

    // Azimuth difference wrapped to [-180, +180] degrees
    float yaw_diff = target_yaw - cam->yaw;
    while (yaw_diff > 180.0f)  yaw_diff -= 360.0f;
    while (yaw_diff < -180.0f) yaw_diff += 360.0f;

    // Authentic lag interpolation
    cam->yaw += yaw_diff * cam->lag_factor;
    while (cam->yaw >= 360.0f) cam->yaw -= 360.0f;
    while (cam->yaw < 0.0f)    cam->yaw += 360.0f;

    // Hill and terrain pitch adaptation matching FUN_0043c910 formulas
    float terrain_pitch_deg = (float)(veh->pitch * RAD2DEG);
    cam->target_pitch = terrain_pitch_deg;

    if (cam->mode == CAMERA_MODE_BUMPER) {
        cam->pitch = 5.0f + terrain_pitch_deg;
    } else {
        float base_pitch = 32.0f;
        if (cam->mode == CAMERA_MODE_CLOSE) base_pitch = 24.0f;
        else if (cam->mode == CAMERA_MODE_FAR) base_pitch = 40.0f;

        cam->pitch = base_pitch + terrain_pitch_deg * 0.35f;
        if (cam->pitch < 10.0f) cam->pitch = 10.0f;
        if (cam->pitch > 85.0f) cam->pitch = 85.0f;
    }

    Camera_Update(cam, delta_time);
}
