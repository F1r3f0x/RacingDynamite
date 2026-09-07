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

#include "ignition/physics.h"
#include "ignition/log.h"
#include <math.h>
#include <string.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static double Clamp(double val, double min_val, double max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

/**
 * @brief Initialize vehicle physics state, dimensions, and starting grid placement.
 * @original FUN_0041b470 (IGN_WIN.EXE @ 0x0041b470, main.c)
 * @fidelity ADAPTED
 */
void Vehicle_Init(VehicleState *veh, int car_index,
                  double start_x, double start_y, double start_z, double start_yaw) {
    if (!veh) return;
    memset(veh, 0, sizeof(VehicleState));

    veh->car_index = car_index;
    veh->x = start_x;
    veh->y = start_y;
    veh->z = start_z;
    veh->yaw = start_yaw;
    veh->gear = 1;
    veh->engine_rpm = 1000.0;
    veh->avg_friction = 1.0;
    veh->avg_ground_y = start_y - PHYSICS_RIDE_HEIGHT;
    veh->airborne = true;
    veh->prev_airborne = true;
    veh->prev_gear = 1;
    veh->prev_turbo_active = false;

    // Archetype parameters
    veh->params.mass = 950.0;
    veh->params.inertia_z = 1400.0;
    veh->params.wheelbase_a = 1.15; // Distance from CG to front axle (m)
    veh->params.wheelbase_b = 1.15; // Distance from CG to rear axle (m)
    veh->params.track_width = 1.45; // Distance between left and right wheels (m)
    veh->params.max_steer_angle = 0.45; // ~26 degrees steering lock
    veh->params.engine_power = 24000.0; // Engine propulsion thrust
    veh->params.brake_power = 32000.0;  // Braking power
    veh->params.chassis_scale = 1.0;

    // Specific archetype characteristics (authentic 11 vehicle catalog)
    switch (car_index) {
        case 0: // Coop (Cooper): nimble, lightweight, responsive
            veh->params.mass = 750.0;
            veh->params.engine_power = 22000.0;
            veh->params.max_steer_angle = 0.44;
            break;
        case 1: // Evor (Porsche): high acceleration, rear-bias sports car
            veh->params.mass = 880.0;
            veh->params.engine_power = 30000.0;
            veh->params.max_steer_angle = 0.42;
            break;
        case 2: // Buggy (Jeep): agile off-road
            veh->params.mass = 780.0;
            veh->params.engine_power = 24000.0;
            veh->params.max_steer_angle = 0.45;
            break;
        case 3: // Enforcer (Police Cruiser): heavy, high stability pursuit
            veh->params.mass = 1150.0;
            veh->params.engine_power = 29000.0;
            veh->params.max_steer_angle = 0.40;
            break;
        case 4: // Red Devil (Mustang): high torque muscle car
            veh->params.mass = 920.0;
            veh->params.engine_power = 31000.0;
            veh->params.max_steer_angle = 0.42;
            break;
        case 5: // School Bus: massive weight, long wheelbase
            veh->params.mass = 2400.0;
            veh->params.engine_power = 42000.0;
            veh->params.wheelbase_a = 1.75;
            veh->params.wheelbase_b = 1.75;
            veh->params.max_steer_angle = 0.36;
            break;
        case 6: // Smoke (Van): heavy utility vehicle
            veh->params.mass = 1300.0;
            veh->params.engine_power = 26000.0;
            veh->params.max_steer_angle = 0.38;
            break;
        case 7: // Bug (VW Beetle): balanced, compact
            veh->params.mass = 820.0;
            veh->params.engine_power = 23000.0;
            veh->params.max_steer_angle = 0.45;
            break;
        case 8: // Monster (Truck): massive torque, wide track
            veh->params.mass = 1700.0;
            veh->params.engine_power = 40000.0;
            veh->params.track_width = 1.85;
            veh->params.max_steer_angle = 0.44;
            break;
        case 9: // Vegas (Dodge): heavyweight American cruiser
            veh->params.mass = 1200.0;
            veh->params.engine_power = 32000.0;
            veh->params.max_steer_angle = 0.40;
            break;
        case 10: // Ignition (NASCAR): maximum top speed & acceleration
            veh->params.mass = 850.0;
            veh->params.engine_power = 36000.0;
            veh->params.max_steer_angle = 0.38;
            break;
        default:
            break;
    }


    // Configure 4 independent wheel local offsets (FL, FR, RL, RR)
    double half_tw = veh->params.track_width * 0.5;
    double front_a = veh->params.wheelbase_a;
    double rear_b = veh->params.wheelbase_b;

    veh->wheels[0].offset_x = -half_tw; veh->wheels[0].offset_z =  front_a; // Front-Left
    veh->wheels[1].offset_x =  half_tw; veh->wheels[1].offset_z =  front_a; // Front-Right
    veh->wheels[2].offset_x = -half_tw; veh->wheels[2].offset_z = -rear_b;  // Rear-Left
    veh->wheels[3].offset_x =  half_tw; veh->wheels[3].offset_z = -rear_b;  // Rear-Right

    for (int i = 0; i < 4; i++) {
        veh->wheels[i].contact_y = start_y - PHYSICS_RIDE_HEIGHT;
        veh->wheels[i].world_y = start_y - PHYSICS_RIDE_HEIGHT;
        veh->wheels[i].friction = 1.0;
        veh->wheels[i].grounded = true;
        veh->wheels[i].active_triangle = -1;
    }
}

/**
 * @brief Apply driver control inputs (throttle, brake, steering, turbo boost).
 * @fidelity INFRASTRUCTURE
 */
void Vehicle_ApplyInput(VehicleState *veh, double throttle, double brake, double steer, bool boost) {
    if (!veh) return;
    veh->throttle_input = Clamp(throttle, 0.0, 1.0);
    veh->brake_input = Clamp(brake, 0.0, 1.0);
    veh->steer_input = Clamp(steer, -1.0, 1.0);
    veh->boost_input = boost;
}

/**
 * @brief Perform a single fixed-timestep 72 Hz physics integration tick.
 * @original FUN_00424570 (IGN_WIN.EXE @ 0x00424570, vehicle.c)
 * @original FUN_00427d70 (IGN_WIN.EXE @ 0x00427d70, main.c)
 * @original FUN_0040e6b0 (IGN_WIN.EXE @ 0x0040e6b0, vehicle.c)
 * @original FUN_00423aa0 (IGN_WIN.EXE @ 0x00423aa0, vehicle.c)
 * @original FUN_00442030 (IGN_WIN.EXE @ 0x00442030, vehicle.c)
 * @original FUN_00442670 (IGN_WIN.EXE @ 0x00442670, vehicle.c)
 * @fidelity EXTENDED
 * @deviation DEV-003 (Mountain wall climbing steep gradient adhesion clamp)
 * @fix_category FIX_CAT_NOCLIP
 */
void Vehicle_Update(VehicleState *veh, const SrfData *srf, const PlcData *plc, const MshData *msh,
                    double dt, const GameFixOptions *fixes) {
    if (!veh) return;

    if (dt <= 0.0) dt = PHYSICS_DT_SEC;
    if (dt > 0.05) dt = 0.05; // Clamp delta time spike to prevent physics explosion

    // 1. Transform 4 wheel contact coordinates to world space and raycast surface
    double cos_yaw = cos(veh->yaw);
    double sin_yaw = sin(veh->yaw);
    double sum_ground_y = 0.0;
    double sum_friction = 0.0;
    int grounded_wheel_count = 0;

    bool fix_noclip = (!fixes || fixes->fix_noclip);

    for (int i = 0; i < 4; i++) {
        WheelPhysics *wheel = &veh->wheels[i];
        // World coordinates: rotated by heading and scaled
        wheel->world_x = veh->x + (wheel->offset_x * cos_yaw + wheel->offset_z * sin_yaw) *
                                  veh->params.chassis_scale * PHYSICS_SCALE_FACTOR;
        wheel->world_z = veh->z + (-wheel->offset_x * sin_yaw + wheel->offset_z * cos_yaw) *
                                  veh->params.chassis_scale * PHYSICS_SCALE_FACTOR;

        SurfaceRaycastResult ray;
        Surface_Raycast(srf, plc, msh, wheel->world_x, wheel->world_z, veh->y, &ray, fixes);

        // Maximum step height that can be climbed based on forward speed (IGN_WIN.EXE 0x00424570)
        double max_step = fabs(veh->speed_long) * 0.5 + 25.0;
        bool is_wall = (ray.normal_y < 0.25) || (ray.material_id >= 80);
        bool is_steep_step = false;
        if (!veh->airborne && wheel->contact_y != 0.0) {
            double step_delta = ray.elevation - wheel->contact_y;
            if (step_delta > max_step) {
                is_steep_step = true;
            }
        }

        if (fix_noclip && (is_wall || is_steep_step)) {
            // Cannot climb steep mountain wall or sheer cliff: clamp contact height to vehicle elevation
            wheel->contact_y = veh->y - PHYSICS_RIDE_HEIGHT;
            wheel->grounded = false;
            if (i < 2 && veh->speed_long > 0.0) {
                // Front wheels hit obstacle while driving forward: halt propulsion
                veh->speed_long = 0.0;
                veh->vx = 0.0;
                veh->vz = 0.0;
            } else if (i >= 2 && veh->speed_long < 0.0) {
                // Rear wheels hit obstacle while reversing: halt propulsion
                veh->speed_long = 0.0;
                veh->vx = 0.0;
                veh->vz = 0.0;
            }
        } else {
            wheel->contact_y = ray.elevation;
            // Suspension contact test: within 25.0 units above ground
            if (veh->y >= wheel->contact_y - 2.0 && veh->y <= wheel->contact_y + 25.0) {
                wheel->grounded = true;
                grounded_wheel_count++;
            } else {
                wheel->grounded = false;
            }
        }

        wheel->friction = ray.friction;
        wheel->active_triangle = ray.triangle_idx;
        wheel->world_y = wheel->contact_y;

        if (ray.is_boost_pad && veh->turbo_timer <= 0) {
            // Trigger turbo boost from track pad
            veh->turbo_timer = 200; // ~2.8 seconds
            veh->turbo_active = true;
        }

        sum_ground_y += wheel->contact_y;
        sum_friction += wheel->friction;
    }

    veh->avg_ground_y = sum_ground_y * 0.25;
    veh->avg_friction = sum_friction * 0.25;
    if (veh->avg_friction < 0.1) veh->avg_friction = 0.1;

    // 2. Chassis Pitch & Roll Calculation from Axle Elevation Differences
    double front_y = (veh->wheels[0].contact_y + veh->wheels[1].contact_y) * 0.5;
    double rear_y  = (veh->wheels[2].contact_y + veh->wheels[3].contact_y) * 0.5;
    double left_y  = (veh->wheels[0].contact_y + veh->wheels[2].contact_y) * 0.5;
    double right_y = (veh->wheels[1].contact_y + veh->wheels[3].contact_y) * 0.5;

    double wheelbase_world = (veh->params.wheelbase_a + veh->params.wheelbase_b) * PHYSICS_SCALE_FACTOR;
    double trackwidth_world = veh->params.track_width * PHYSICS_SCALE_FACTOR;

    double sin_pitch_target = Clamp((front_y - rear_y) / wheelbase_world,
                                    -PHYSICS_MAX_SLOPE_SIN, PHYSICS_MAX_SLOPE_SIN);
    double pitch_target = asin(sin_pitch_target);
    double pitch_delta = Clamp(pitch_target - veh->pitch,
                               -PHYSICS_RATE_LIMIT_RAD, PHYSICS_RATE_LIMIT_RAD);
    veh->pitch += pitch_delta;

    double sin_roll_target = Clamp((left_y - right_y) / trackwidth_world,
                                   -PHYSICS_MAX_SLOPE_SIN, PHYSICS_MAX_SLOPE_SIN);
    double roll_target = asin(sin_roll_target);
    double roll_delta = Clamp(roll_target - veh->roll,
                              -PHYSICS_RATE_LIMIT_RAD, PHYSICS_RATE_LIMIT_RAD);
    veh->roll += roll_delta;

    // 3. Vertical Dynamics & Suspension Equilibrium
    double target_y = veh->avg_ground_y + PHYSICS_RIDE_HEIGHT;

    // Apply gravity acceleration downward (-Y)
    double gravity_delta = PHYSICS_GRAVITY_TICK * (dt / PHYSICS_DT_SEC);
    veh->vy -= gravity_delta;
    veh->y += veh->vy;

    if (veh->y <= target_y) {
        // Impact or grounded on surface
        if (veh->airborne) {
            // Landing rebound response
            if (veh->vy < -8.0) {
                veh->vy *= -0.20; // Rebound bounce
            } else {
                veh->vy *= 0.50;  // Damped landing
            }
            if (veh->vy < -15.0) veh->vy = -15.0;
            veh->airborne = false;
            veh->landing_timer = 1;
        } else {
            // Contour following on surface
            veh->y = target_y;
            veh->vy = 0.0;
        }
    } else {
        veh->airborne = true;
    }

    // 4. Turbo Boost Countdown
    if (veh->turbo_timer > 0) {
        veh->turbo_timer--;
        veh->turbo_active = true;
    } else {
        veh->turbo_active = false;
    }

    if (veh->boost_input && veh->turbo_timer <= 0) {
        // Player manual turbo boost activation
        veh->turbo_timer = 200; // ~2.8 seconds
        veh->turbo_active = true;
    }

    double boost_mult = veh->turbo_active ? 1.75 : 1.0;

    // 5. Steering Dynamics (Bicycle Handling Model with Speed Attenuation)
    double speed_abs = fabs(veh->speed_long);
    // Smooth speed attenuation: progressively reduce steering angle at high speeds to avoid twitchiness
    double steer_attenuation = 1.0 / (1.0 + (speed_abs / 22.0) * 0.90);
    if (steer_attenuation < 0.28) steer_attenuation = 0.28;
    double target_steer = veh->steer_input * veh->params.max_steer_angle * steer_attenuation;

    // Filter steering angle smoothly (factor ~0.10 per tick at 72 Hz matches FUN_00444c20)
    veh->steering_angle += (target_steer - veh->steering_angle) * (0.12 * (dt / PHYSICS_DT_SEC));

    // Bicycle kinematic yaw rate: psi_dot = (sin(delta) / (a + b)) * V_long
    double total_wb = veh->params.wheelbase_a + veh->params.wheelbase_b;
    double target_yaw_rate = (sin(veh->steering_angle) / total_wb) * veh->speed_long;

    // Lateral grip limits maximum yaw rate to prevent spinning out of control
    double max_yaw_rate = 3.0 * veh->avg_friction;
    target_yaw_rate = Clamp(target_yaw_rate, -max_yaw_rate, max_yaw_rate);

    double max_yaw_accel = 8.0 * veh->avg_friction;
    double yaw_accel = Clamp(target_yaw_rate - veh->yaw_rate,
                             -max_yaw_accel * dt * 25.0, max_yaw_accel * dt * 25.0);
    veh->yaw_rate += yaw_accel;
    veh->yaw += veh->yaw_rate * dt;


    // Keep yaw in [-PI, +PI]
    while (veh->yaw > M_PI)  veh->yaw -= 2.0 * M_PI;
    while (veh->yaw < -M_PI) veh->yaw += 2.0 * M_PI;

    // 6. Powertrain & Longitudinal Dynamics
    double thrust = 0.0;
    double braking = 0.0;

    if (veh->gear == 1) { // Forward gear
        thrust = veh->throttle_input * veh->params.engine_power * boost_mult * veh->avg_friction;
        braking = veh->brake_input * veh->params.brake_power * veh->avg_friction;

        // Shift into reverse when stopped and brake is held
        if (speed_abs < 1.5 && veh->brake_input > 0.4 && veh->throttle_input < 0.1) {
            veh->gear = 0; // Reverse gear
        }
    } else { // Reverse gear
        thrust = -veh->brake_input * (veh->params.engine_power * 0.45) * veh->avg_friction;
        braking = veh->throttle_input * veh->params.brake_power * veh->avg_friction;

        // Shift back to 1st gear when throttle is pressed
        if (veh->throttle_input > 0.2) {
            veh->gear = 1;
        }
    }

    // Aerodynamic and rolling drag
    double drag = 0.40 * veh->speed_long * speed_abs + 25.0 * veh->speed_long;

    // Gravity slope component pulling car along incline
    double slope_gravity = veh->params.mass * PHYSICS_GRAVITY * sin(veh->pitch);

    // Net longitudinal force
    double net_long_force = thrust - (braking * (veh->speed_long > 0 ? 1.0 : -1.0)) - drag - slope_gravity;
    double accel_long = net_long_force / veh->params.mass;

    if (!veh->airborne) {
        veh->speed_long += accel_long * dt;
    } else {
        // Airborne: reduced drag, no ground propulsion
        veh->speed_long += (-drag / veh->params.mass) * dt;
    }

    // Centrifugal lateral slip stabilization
    double lat_drag = 16.0 * veh->avg_friction;
    veh->speed_lat += (-veh->speed_lat * lat_drag) * dt;

    // Engine RPM calculation for sound pitch
    veh->engine_rpm = 1000.0 + speed_abs * 65.0;
    if (veh->engine_rpm > 8000.0) veh->engine_rpm = 8000.0;

    // 7. World Velocity & Position Integration
    veh->vx = veh->speed_long * sin(veh->yaw) + veh->speed_lat * cos(veh->yaw);
    veh->vz = veh->speed_long * cos(veh->yaw) - veh->speed_lat * sin(veh->yaw);

    veh->x += veh->vx * dt * PHYSICS_SCALE_FACTOR;
    veh->z += veh->vz * dt * PHYSICS_SCALE_FACTOR;

    // 8. Event Logging: Detect and log significant vehicle state transitions
    if (!veh->prev_airborne && veh->airborne) {
        LOG_DEBUG("PHYSICS", "Vehicle %d TAKEOFF: speed=%.1f y=%.1f vy=%.2f",
                  veh->car_index, veh->speed_long, veh->y, veh->vy);
    } else if (veh->prev_airborne && !veh->airborne) {
        LOG_DEBUG("PHYSICS", "Vehicle %d TOUCHDOWN: speed=%.1f impact_vy=%.2f ground_y=%.1f",
                  veh->car_index, veh->speed_long, veh->vy, veh->avg_ground_y);
    }
    if (veh->gear != veh->prev_gear) {
        LOG_DEBUG("PHYSICS", "Vehicle %d GEAR SHIFT: %s -> %s (speed=%.1f, RPM=%.0f)",
                  veh->car_index,
                  veh->prev_gear == 0 ? "R" : "1st",
                  veh->gear == 0 ? "R" : "1st",
                  veh->speed_long, veh->engine_rpm);
    }
    if (!veh->prev_turbo_active && veh->turbo_active) {
        LOG_INFO("PHYSICS", "Vehicle %d TURBO ACTIVATED (timer=%d ticks)",
                 veh->car_index, veh->turbo_timer);
    } else if (veh->prev_turbo_active && !veh->turbo_active) {
        LOG_DEBUG("PHYSICS", "Vehicle %d TURBO EXPIRED", veh->car_index);
    }

    veh->prev_airborne = veh->airborne;
    veh->prev_gear = veh->gear;
    veh->prev_turbo_active = veh->turbo_active;
}

/**
 * @brief Output detailed telemetry for the vehicle state.
 * @fidelity INFRASTRUCTURE
 */
void Vehicle_LogTelemetry(const VehicleState *veh, uint32_t tick_num) {
    if (!veh) return;
    LOG_DEBUG("PHYSICS", "Tick #%u Car %d: pos=(%.1f, %.1f, %.1f) vel=(%.1f, %.1f, %.1f) speed_long=%.1f yaw=%.1f deg pitch=%.1f deg roll=%.1f deg RPM=%.0f gear=%s grounded=%d friction=%.2f",
              tick_num, veh->car_index,
              veh->x, veh->y, veh->z,
              veh->vx, veh->vy, veh->vz,
              veh->speed_long,
              veh->yaw * (180.0 / M_PI),
              veh->pitch * (180.0 / M_PI),
              veh->roll * (180.0 / M_PI),
              veh->engine_rpm,
              veh->gear == 0 ? "R" : (veh->gear == 1 ? "1" : "2"),
              !veh->airborne,
              veh->avg_friction);
}

/**
 * @brief Compute smooth isometric chase camera parameters targeting the vehicle.
 * @original FUN_00436990 (IGN_WIN.EXE @ 0x00436990, main.c)
 * @fidelity EXTENDED
 * @deviation DEV-004 (Right-handed camera basis vector normalization)
 * @fix_category FIX_CAT_CAMERA
 */
void Vehicle_GetChaseCamera(const VehicleState *veh, Camera3D *cam) {
    if (!veh || !cam) return;
    Camera_UpdateFollowChase(cam, veh, 0.0f, NULL);
}
