# Ignition (1997) Track Splines & AI Waypoint Subsystem

## Overview
AI vehicle navigation, checkpoint timing, and camera follow rails in Ignition are governed by a closed-loop parametric spline system reconstructed from three collaborating asset types:
1. `.PLC`: World positions, instances, and archetype classifications.
2. `.MSH`: 3D geometry submeshes containing road vertices.
3. `.TRI`: Index tables mapping road boundary vertices (left rail, right rail, center) and fork branches.

---

## 1. Placed Object Archetype De-packing (`0x0041b360`)

Before building splines, the engine unpacks the `model_type` dword from `.PLC`:

```c
void Track_PreprocessPlacements(void) {
    for (int i = 0; i < plc->count; ++i) {
        uint32_t raw = plc->objects[i].model_type;
        plc->objects[i].model_type = raw & 0xFFF; // Archetype ID in low 12 bits
        // High bits define lighting, animation channels, and group flags
    }
}
```

### Archetype Mapping (`model_type & 0xFFF`):
* `0`: Starting line / finish gate road chunk.
* `1`: First forward road chunk immediately succeeding the start line.
* `2..9`: Sequential circuit road segments.
* `3`: Fork diversion start segment (diverges into parallel routes).
* `4`: Left fork road segments.
* `5`: Right fork road segments.
* `6`: Fork convergence segment (rejoins back to mainline).
* `7`: Tunnel or bridge elevated road segment.
* `150..154`: Split-time timing checkpoints and track boundary penalty gates.
* `200`: Grid spawn slots for player and AI opponents.
* `300..357`: Scenery models (trees, stadium bleachers, buildings).

---

## 2. Road Chunk Sequencing Algorithm (`0x00416250`)

To construct a continuous circuit trajectory, `Mesh_InstantiatePlacedObjects` chains road segments into an ordered sequence `g_pTrackRoadSequence` (`0x00552f60`):

1. **Centroid Normalization**: For all chunks where `type < 50`, the submesh vertex coordinates are averaged in world space to yield the true physical centroid $\vec{C}_i$.
2. **Start Anchor**: The sequence begins at `start_chunk` (`type == 0`) and proceeds to `first_chunk` (`type == 1`).
3. **Nearest-Neighbor Traversal**: From the current road segment $S_k$, the engine selects the next segment $S_{k+1}$ minimizing 3D distance with an elevation penalty:
   $$D(A, B) = \sqrt{(B_x - A_x)^2 + 4 \cdot (B_y - A_y)^2 + (B_z - A_z)^2}$$
4. **Branch Traversal**:
   - Encountering `type == 3` forks the traversal into branch paths (`type == 4` and `type == 5`).
   - Both paths re-converge at `type == 6`.
5. **Loop Closure**: Chaining concludes when returning to `start_chunk`, forming a closed circuit spline.

---

## 3. Waypoint Generation from `.TRI` (`0x00414e40`)

For each road chunk $k$ in the ordered sequence, `Track_LoadSplines` loads the corresponding 500-byte record from `.TRI`:

* Left boundary vertex index: $v_L = \text{rec}[1..2]$ (`int16_t`).
* Right boundary vertex index: $v_R = \text{rec}[23..24]$ (`int16_t`).
* Split flag: $F = \text{rec}[109]$ (`uint8_t`).

```c
Vec3 P_left  = submesh_vertices[v_L] + chunk_world_pos;
Vec3 P_right = submesh_vertices[v_R] + chunk_world_pos;
Vec3 P_center = (P_left + P_right) * 0.5f;
float heading = atan2f(next_center.x - P_center.x, next_center.z - P_center.z);
```

---

## 4. AI Opponent Driving Simulation (`0x004134e0`)

During race simulation, each computer-controlled car executes `AI_FollowTrackSplines`:

1. **Active Node Tracking**: The car checks proximity to current waypoint $W_n$. When distance falls below tracking radius, lap progress advances to $W_{n+1}$.
2. **Target Steering Angle**:
   $$\theta_{target} = \text{atan2}(W_{n+1}.z - \text{car}.z, W_{n+1}.x - \text{car}.x)$$
   $$\Delta\theta = \theta_{target} - \text{car.yaw}$$
3. **Speed & Cornering Control**:
   - The engine computes path curvature between nodes:
     $$\kappa = \frac{\Delta\text{heading}}{\Delta\text{distance}}$$
   - When $\kappa$ exceeds turn thresholds, the AI applies brake force or releases turbo boost.

---

## 5. 3D Visualizer & Toggle (`RacingDynamite`)

In the source port, waypoints can be toggled on/off in real-time using **`P`** (or **`V`**):
* **Centerline Ribbon**: Golden/yellow lines (`color 215`) connecting consecutive waypoint coordinates.
* **Road Width Crossbars**: Cyan lines (`color 160`) spanning between Left Rail and Right Rail.
* **Vertical Node Pins**: Elevated white pins (`color 251`) at each node center.
* **HUD Indicator**: `[WP ON]` or `[WP OFF]` displayed in the race status overlay.
