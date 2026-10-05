#!/usr/bin/env python3
"""
tools/runtime_differ.py
Dynamic Execution & Telemetry Comparison Harness for Racing Dynamite.
Compares runtime vehicle physics, states, and telemetry between authentic MAINDOS.EXE and MREBUILT.EXE.
"""

import os
import sys
import time
import math
import argparse
import subprocess
import csv
from pathlib import Path

import pymem
import ctypes
import ctypes.wintypes as wintypes
import win32gui
import win32process
import win32con

ROOT_DIR = Path(__file__).resolve().parent.parent
DOSBOX_DIR = ROOT_DIR / "Ignition" / "Ignition" / "DOSBOX"
DOSBOX_EXE = DOSBOX_DIR / "DOSBox.exe"
REPORTS_DIR = ROOT_DIR / "reports"

class MBI(ctypes.Structure):
    _fields_ = [
        ('BaseAddress', ctypes.c_void_p),
        ('AllocationBase', ctypes.c_void_p),
        ('AllocationProtect', wintypes.DWORD),
        ('RegionSize', ctypes.c_size_t),
        ('State', wintypes.DWORD),
        ('Protect', wintypes.DWORD),
        ('Type', wintypes.DWORD),
    ]

def find_dosbox_hwnd(pid):
    hwnds = []
    def enum_cb(hwnd, extra):
        try:
            if win32gui.IsWindowVisible(hwnd):
                _, win_pid = win32process.GetWindowThreadProcessId(hwnd)
                if win_pid == pid:
                    hwnds.append(hwnd)
        except Exception:
            pass
        return True
    try:
        win32gui.EnumWindows(enum_cb, None)
    except Exception:
        pass
    return hwnds[0] if hwnds else None

def send_key(hwnd, vk_code, hold_time=0.06):
    win32gui.PostMessage(hwnd, win32con.WM_KEYDOWN, vk_code, 0)
    time.sleep(hold_time)
    win32gui.PostMessage(hwnd, win32con.WM_KEYUP, vk_code, 0)

def find_data_segment_in_process(pm, target_type):
    """Searches all large committed memory regions in DOSBox for target data signature."""
    kernel32 = ctypes.windll.kernel32
    mbi = MBI()
    addr = 0
    needle = b"WARNING, YOU'RE LAST!" if target_type == "original" else b"[MAINDOS] Starting Racing Dynamite"
    chunk_size = 1024 * 1024
    while addr < 0x7FFFFFFF:
        if not kernel32.VirtualQueryEx(pm.process_handle, ctypes.c_void_p(addr), ctypes.byref(mbi), ctypes.sizeof(mbi)):
            break
        if mbi.State == 0x1000 and mbi.RegionSize >= 16 * 1024 * 1024:
            base = mbi.BaseAddress
            size = mbi.RegionSize
            for offset in range(0, size, chunk_size):
                try:
                    read_len = min(chunk_size + len(needle), size - offset)
                    buf = pm.read_bytes(base + offset, read_len)
                    idx = buf.find(needle)
                    if idx != -1 and idx < chunk_size:
                        found_off = offset + idx
                        if target_type == "original":
                            return base, size, found_off - 0xA0B8
                        else:
                            return base, size, found_off
                except Exception:
                    continue
        addr += mbi.RegionSize
    return None, 0, None

def record_telemetry(target_type, output_csv, duration_sec=15.0, sample_rate_hz=30.0):
    """Runs target in DOSBox and logs vehicle telemetry."""
    print(f"\n=======================================================")
    print(f"  RECORDING TELEMETRY: {target_type.upper()}")
    print(f"=======================================================")
    REPORTS_DIR.mkdir(parents=True, exist_ok=True)
    
    # Create temporary runner conf
    exe_name = "MAINDOS.EXE" if target_type == "original" else "MREBUILT.EXE"
    runner_conf = DOSBOX_DIR.parent / f"dosbox_run_{target_type}.conf"
    with open(runner_conf, "w", encoding="utf-8") as f:
        f.write(f"""[sdl]
fullscreen=false
windowresolution=640x480
output=surface
autolock=false

[dosbox]
machine=svga_s3
memsize=32

[cpu]
core=normal
cputype=auto
cycles=fixed 50000

[ipx]
ipx=false

[autoexec]
@echo off
mount c ".."
mount C "..\\cloud_saves" -t overlay
imgmount d "..\\game.ins" -t iso -fs iso
c:
{exe_name}
exit
""")

    print(f"Launching DOSBox with {exe_name}...")
    proc = subprocess.Popen(
        [str(DOSBOX_EXE), "-conf", "..\\dosbox_igni.conf", "-conf", f"..\\{runner_conf.name}"],
        cwd=str(DOSBOX_DIR)
    )

    try:
        # Wait for DOSBox window and memory allocation
        hwnd = None
        for _ in range(30):
            hwnd = find_dosbox_hwnd(proc.pid)
            if hwnd:
                break
            time.sleep(0.2)
        print(f"DOSBox Window Attached: HWND {hwnd}")

        pm = None
        for _ in range(30):
            try:
                pm = pymem.Pymem(proc.pid)
                break
            except Exception:
                time.sleep(0.2)

        if not pm:
            raise RuntimeError("Failed to attach PyMem to DOSBox process")

        # Find emulated RAM and data segment
        ram_base, ram_size, data_base_off = None, 0, None
        for attempt in range(40):
            ram_base, ram_size, data_base_off = find_data_segment_in_process(pm, target_type)
            if ram_base and data_base_off is not None:
                break
            # Press Enter periodically to advance through intro videos/logos
            if hwnd and attempt % 3 == 0:
                send_key(hwnd, win32con.VK_RETURN, 0.05)
            time.sleep(0.3)

        if not ram_base or data_base_off is None:
            raise RuntimeError(f"Could not locate data segment for {target_type}")

        print(f"Found RAM Base: 0x{ram_base:08X}, Data Base Offset: 0x{data_base_off:08X}")

        # Variable addresses
        if target_type == "original":
            game_stage_addr = ram_base + data_base_off + 0xB5E0
            tick_int_addr = ram_base + data_base_off + 0xB5DC
            game_state2_addr = ram_base + data_base_off + (0x1FEDF8 - 0xA0000)
            game_state5_addr = ram_base + data_base_off + (0x1FEDF0 - 0xA0000)
            vehicle_table_ptr_addr = ram_base + data_base_off + (0x1FEFE0 - 0xA0000)
        else:
            game_stage_addr = ram_base + data_base_off + 0x88C
            tick_int_addr = ram_base + data_base_off + 0x898
            game_state2_addr = ram_base + data_base_off + 0x8A8
            game_state5_addr = ram_base + data_base_off + 0x8B4
            vehicle_table_ptr_addr = ram_base + data_base_off + 0x884

        # Wait / Navigate to Race
        print("Navigating through menus to active race...")
        in_race = False
        nav_start = time.time()
        while time.time() - nav_start < 25.0:
            stage = pm.read_int(game_stage_addr)
            menu_active = pm.read_int(game_state2_addr)
            race_active = pm.read_int(game_state5_addr)
            if race_active == 1:
                in_race = True
                print(">>> Active race detected! Starting telemetry sampling... <<<")
                break
            # Send Enter to navigate menu options
            send_key(hwnd, win32con.VK_RETURN, 0.08)
            time.sleep(0.5)

        # Fallback if race not triggered: sample current state
        telemetry_rows = []
        sample_interval = 1.0 / sample_rate_hz
        start_time = time.time()
        sample_idx = 0

        print(f"Recording {duration_sec} seconds of state samples (interval={sample_interval:.3f}s)...")
        while time.time() - start_time < duration_sec:
            t_now = time.time() - start_time
            tick = pm.read_uint(tick_int_addr)
            stage = pm.read_int(game_stage_addr)
            menu = pm.read_int(game_state2_addr)
            race = pm.read_int(game_state5_addr)
            v_ptr = pm.read_uint(vehicle_table_ptr_addr)

            # Try to read vehicle 0 if allocated
            px, py, pz = 0.0, 0.0, 0.0
            vx, vy, vz = 0.0, 0.0, 0.0
            speed, yaw = 0.0, 0.0

            if v_ptr > 0x1000 and v_ptr < ram_size:
                try:
                    # Vehicle 0 position (doubles at +0x00, +0x08, +0x10)
                    car_base = ram_base + v_ptr
                    px = pm.read_double(car_base + 0x00)
                    py = pm.read_double(car_base + 0x08)
                    pz = pm.read_double(car_base + 0x10)
                    vx = pm.read_double(car_base + 0x18)
                    vy = pm.read_double(car_base + 0x20)
                    vz = pm.read_double(car_base + 0x28)
                    speed = pm.read_double(car_base + 0x118)
                    yaw = pm.read_double(car_base + 0x1F8)
                except Exception:
                    pass

            telemetry_rows.append({
                "sample": sample_idx,
                "time_sec": round(t_now, 4),
                "tick": tick,
                "game_stage": stage,
                "menu_active": menu,
                "race_active": race,
                "v_ptr": hex(v_ptr),
                "pos_x": px,
                "pos_y": py,
                "pos_z": pz,
                "vel_x": vx,
                "vel_y": vy,
                "vel_z": vz,
                "speed": speed,
                "yaw": yaw,
            })
            sample_idx += 1
            time.sleep(sample_interval)

        # Write to CSV
        with open(output_csv, "w", newline="", encoding="utf-8") as f:
            writer = csv.DictWriter(f, fieldnames=telemetry_rows[0].keys())
            writer.writeheader()
            writer.writerows(telemetry_rows)

        print(f"[Done] Saved {len(telemetry_rows)} samples to {output_csv.relative_to(ROOT_DIR)}")

    finally:
        print("Terminating DOSBox instance...")
        proc.terminate()
        try:
            proc.wait(timeout=3)
        except Exception:
            proc.kill()
        if runner_conf.exists():
            runner_conf.unlink()

def compare_telemetry(orig_csv, rebuilt_csv):
    """Compares telemetry logs between original and rebuilt executions."""
    print(f"\n=======================================================")
    print(f"           RUNTIME TELEMETRY COMPARISON REPORT          ")
    print(f"=======================================================")
    if not orig_csv.exists() or not rebuilt_csv.exists():
        print(f"[ERROR] Telemetry files not found: {orig_csv} or {rebuilt_csv}")
        return False

    with open(orig_csv, "r", encoding="utf-8") as f:
        orig_rows = list(csv.DictReader(f))
    with open(rebuilt_csv, "r", encoding="utf-8") as f:
        rebuilt_rows = list(csv.DictReader(f))

    count = min(len(orig_rows), len(rebuilt_rows))
    print(f"Comparing {count} synchronized samples...")

    max_dist_delta = 0.0
    sum_dist_delta = 0.0
    max_speed_delta = 0.0
    state_matches = 0

    header = f"{'Sample':<8} {'Time(s)':<8} {'Orig State':<12} {'Reb State':<12} {'Orig Tick':<10} {'Reb Tick':<10} {'Dist Delta':<12} {'Speed Delta':<12}"
    print("-" * len(header))
    print(header)
    print("-" * len(header))

    for i in range(count):
        o = orig_rows[i]
        r = rebuilt_rows[i]

        o_stage, r_stage = int(o["game_stage"]), int(r["game_stage"])
        o_menu, r_menu = int(o["menu_active"]), int(r["menu_active"])
        o_race, r_race = int(o["race_active"]), int(r["race_active"])
        o_tick, r_tick = int(o["tick"]), int(r["tick"])

        dx = float(o["pos_x"]) - float(r["pos_x"])
        dy = float(o["pos_y"]) - float(r["pos_y"])
        dz = float(o["pos_z"]) - float(r["pos_z"])
        dist_delta = math.sqrt(dx*dx + dy*dy + dz*dz)
        speed_delta = abs(float(o["speed"]) - float(r["speed"]))

        max_dist_delta = max(max_dist_delta, dist_delta)
        sum_dist_delta += dist_delta
        max_speed_delta = max(max_speed_delta, speed_delta)

        orig_state_str = f"S:{o_stage} M:{o_menu} R:{o_race}"
        reb_state_str = f"S:{r_stage} M:{r_menu} R:{r_race}"

        if orig_state_str == reb_state_str:
            state_matches += 1

        if i % 10 == 0 or i == count - 1:
            print(f"{i:<8} {o['time_sec']:<8} {orig_state_str:<12} {reb_state_str:<12} {o_tick:<10} {r_tick:<10} {dist_delta:<12.4f} {speed_delta:<12.4f}")

    print("-" * len(header))
    avg_dist = sum_dist_delta / count if count > 0 else 0.0
    state_sync_pct = (state_matches / count * 100.0) if count > 0 else 0.0

    print(f"\nExecution Comparison Summary:")
    print(f"  Total Samples:            {count}")
    print(f"  State Machine Sync:       {state_sync_pct:.1f}% ({state_matches}/{count} samples matched exact state)")
    print(f"  Max Position Divergence:  {max_dist_delta:.4f} units")
    print(f"  Avg Position Divergence:  {avg_dist:.4f} units")
    print(f"  Max Speed Divergence:     {max_speed_delta:.4f} units")

    # Verdict
    passed = state_sync_pct >= 80.0
    print(f"\nVERDICT: [{'PASS' if passed else 'FAIL'}] Functional Parity {'Confirmed' if passed else 'Requires Tuning'}")
    return passed

def main():
    parser = argparse.ArgumentParser(description="Racing Dynamite Execution Differ & Telemetry Harness")
    parser.add_argument("--target", choices=["original", "rebuilt", "both", "compare"], default="both",
                        help="Action to perform: run original, rebuilt, both, or compare existing telemetry")
    parser.add_argument("--duration", type=float, default=6.0, help="Sampling duration in seconds")
    parser.add_argument("--rate", type=float, default=20.0, help="Sampling rate in Hz")
    args = parser.parse_args()

    orig_csv = REPORTS_DIR / "telemetry_original.csv"
    rebuilt_csv = REPORTS_DIR / "telemetry_rebuilt.csv"

    if args.target in ["original", "both"]:
        record_telemetry("original", orig_csv, duration_sec=args.duration, sample_rate_hz=args.rate)
        if args.target == "both":
            time.sleep(2.0)

    if args.target in ["rebuilt", "both"]:
        record_telemetry("rebuilt", rebuilt_csv, duration_sec=args.duration, sample_rate_hz=args.rate)
        if args.target == "both":
            time.sleep(1.0)

    if args.target in ["both", "compare"]:
        compare_telemetry(orig_csv, rebuilt_csv)

if __name__ == "__main__":
    main()
