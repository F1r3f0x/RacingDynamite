# Ignition (1997) .POS Track Paths & Waypoints Format

## Overview
`.POS` files (e.g. `LEVELS/<TRACK>/<TRACK>.POS`) define track trajectory nodes, checkpoint gates, starting grid positions, and AI driving waypoints.

## Core Data Components

### 1. Checkpoint Gates
Checkpoints define the finish line and split-time timing gates around the circuit.
* Each checkpoint consists of a left/right coordinate pair forming a trigger plane across the track surface.
* Cars crossing the plane in the correct direction increment lap progress counters (`g_LapProgress`).

### 2. AI Waypoint Splines
* Waypoints form a closed cyclic spline loop along the optimal driving line.
* Attributes per waypoint:
  * 3D Position (`x, y, z`).
  * Target speed / throttle recommendation (e.g. slowing for sharp turns).
  * Ideal racing line offset from road center.
  * Road width boundaries.

### 3. Dynamic Camera Paths
* Camera follower splines dictate smooth tracking behavior for overhead and chase cameras around tight curves and loopings.
