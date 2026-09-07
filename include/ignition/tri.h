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

#ifndef IGNITION_TRI_H
#define IGNITION_TRI_H

#include "ignition/types.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct PlcData PlcData;
typedef struct MshData MshData;

// AI Driving Waypoint / Track Ribbon Node
typedef struct {
    float    center_x, center_y, center_z;       // Midpoint of road ribbon (AI driving target)
    float    left_x, left_y, left_z;             // Left boundary point in world space
    float    right_x, right_y, right_z;          // Right boundary point in world space
    float    heading;                            // Tangent heading angle (radians)
    uint8_t  flag;                               // 0 = normal, 1 = branch left, 2 = branch right
    uint32_t chunk_idx;                          // Source PLC object index
} TrackWaypoint;

// Collection of chained track waypoints
typedef struct {
    uint32_t       count;
    TrackWaypoint *waypoints;
} TrackWaypoints;

// Build track road waypoint sequence by chaining PLC road chunks and resolving TRI indices
TrackWaypoints *Track_BuildWaypoints(const char *track_dir, const char *track_name,
                                     const PlcData *plc, const MshData *msh);

// Free allocated track waypoints
void Track_FreeWaypoints(TrackWaypoints *wp);

#ifdef __cplusplus
}
#endif

#endif // IGNITION_TRI_H
