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
#include "ignition/log.h"
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <string.h>

#define PI 3.14159265358979323846f

static void test_camera_init_defaults(void) {
    LOG_INFO("TEST", "Running test_camera_init_defaults...");
    Camera3D cam;
    Camera_Init(&cam, 640.0f / 480.0f);

    assert(cam.mode == CAMERA_MODE_CLASSIC);
    assert(fabsf(cam.distance - 1100.0f) < 0.01f);
    assert(fabsf(cam.pitch - 32.0f) < 0.01f);
    assert(fabsf(cam.fov - 60.0f) < 0.01f);
    assert(fabsf(cam.lag_factor - 0.125f) < 0.001f);
    assert(fabsf(cam.aspect - (4.0f / 3.0f)) < 0.01f);

    // Verify Cartesian coordinates populated
    assert(cam.position.y > cam.target.y);
    LOG_INFO("TEST", "test_camera_init_defaults PASSED");
}

static void test_camera_modes_and_cycling(void) {
    LOG_INFO("TEST", "Running test_camera_modes_and_cycling...");
    Camera3D cam;
    Camera_Init(&cam, 1.333f);

    // Close view
    Camera_SetMode(&cam, CAMERA_MODE_CLOSE);
    assert(cam.mode == CAMERA_MODE_CLOSE);
    assert(fabsf(cam.distance - 650.0f) < 0.01f);
    assert(fabsf(cam.pitch - 24.0f) < 0.01f);

    // Far view
    Camera_SetMode(&cam, CAMERA_MODE_FAR);
    assert(cam.mode == CAMERA_MODE_FAR);
    assert(fabsf(cam.distance - 1900.0f) < 0.01f);
    assert(fabsf(cam.pitch - 40.0f) < 0.01f);

    // Bumper view
    Camera_SetMode(&cam, CAMERA_MODE_BUMPER);
    assert(cam.mode == CAMERA_MODE_BUMPER);
    assert(fabsf(cam.distance - 50.0f) < 0.01f);
    assert(fabsf(cam.pitch - 5.0f) < 0.01f);

    // Cycle through modes
    Camera_CycleMode(&cam); // Bumper (3) -> Classic (0)
    assert(cam.mode == CAMERA_MODE_CLASSIC);

    Camera_CycleMode(&cam); // Classic (0) -> Close (1)
    assert(cam.mode == CAMERA_MODE_CLOSE);

    LOG_INFO("TEST", "test_camera_modes_and_cycling PASSED");
}

static void test_camera_velocity_lookahead(void) {
    LOG_INFO("TEST", "Running test_camera_velocity_lookahead...");
    Camera3D cam;
    Camera_Init(&cam, 1.333f);

    VehicleState veh;
    memset(&veh, 0, sizeof(veh));
    veh.x = 100.0;
    veh.y = 50.0;
    veh.z = 200.0;
    veh.yaw = 0.0; // Heading facing +Z
    veh.speed_long = 25.0; // Moving forward

    Camera_UpdateFollowChase(&cam, &veh, 0.016f, NULL);

    // Lookahead vector should project ahead along heading (+Z)
    assert(cam.lookahead.z > 50.0f);
    assert(fabsf(cam.lookahead.x) < 0.01f);
    assert(cam.target.z > (float)veh.z);

    LOG_INFO("TEST", "test_camera_velocity_lookahead PASSED");
}

static void test_camera_azimuth_lag_and_wrapping(void) {
    LOG_INFO("TEST", "Running test_camera_azimuth_lag_and_wrapping...");
    Camera3D cam;
    Camera_Init(&cam, 1.333f);

    cam.yaw = 355.0f; // Near wrap boundary

    VehicleState veh;
    memset(&veh, 0, sizeof(veh));
    veh.yaw = 0.0; // Heading +Z, target camera angle behind car = 180 deg

    Camera_UpdateFollowChase(&cam, &veh, 0.016f, NULL);

    // Yaw must remain normalized within [0, 360)
    assert(cam.yaw >= 0.0f && cam.yaw < 360.0f);

    // Test reverse wrap: target 10 degrees, current 350 degrees
    veh.yaw = 190.0 * (PI / 180.0); // Target = 190 + 180 = 370 = 10 deg
    cam.yaw = 350.0f;
    Camera_UpdateFollowChase(&cam, &veh, 0.016f, NULL);
    assert(cam.yaw >= 0.0f && cam.yaw < 360.0f);

    LOG_INFO("TEST", "test_camera_azimuth_lag_and_wrapping PASSED");
}

static void test_camera_terrain_pitch_adaptation(void) {
    LOG_INFO("TEST", "Running test_camera_terrain_pitch_adaptation...");
    Camera3D cam;
    Camera_Init(&cam, 1.333f);
    Camera_SetMode(&cam, CAMERA_MODE_CLASSIC); // Base pitch 35 deg

    VehicleState veh;
    memset(&veh, 0, sizeof(veh));
    veh.pitch = 20.0 * (PI / 180.0); // 20 degrees uphill incline

    Camera_UpdateFollowChase(&cam, &veh, 0.016f, NULL);

    // Pitch should adapt upwards to avoid ground clipping
    assert(cam.pitch > 35.0f);
    assert(cam.pitch < 85.0f);

    LOG_INFO("TEST", "test_camera_terrain_pitch_adaptation PASSED");
}

int main(void) {
    LOG_INFO("TEST", "=== RACING DYNAMITE CAMERA SUBSYSTEM TEST SUITE ===");

    test_camera_init_defaults();
    test_camera_modes_and_cycling();
    test_camera_velocity_lookahead();
    test_camera_azimuth_lag_and_wrapping();
    test_camera_terrain_pitch_adaptation();

    LOG_INFO("TEST", "ALL 5 CAMERA TESTS PASSED SUCCESSFULLY!");
    return 0;
}
