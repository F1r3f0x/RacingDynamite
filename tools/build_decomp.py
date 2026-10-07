#!/usr/bin/env python3
"""
Decompilation Build & Verify Tool
Compiles authentic C source files in decomp/src/ using the local portable Open Watcom V2 compiler.
Flags: -3r (register calling convention), -s (no stack checks), -omaxet (full optimization).
"""

import os
import subprocess
import sys
import re
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
WATCOM_DIR = ROOT_DIR / "tools" / "openwatcomv2"
WCC386 = WATCOM_DIR / "binnt64" / "wcc386.exe"
if not WCC386.exists():
    WCC386 = WATCOM_DIR / "binnt" / "wcc386.exe"

WLINK = WATCOM_DIR / "binnt64" / "wlink.exe"
if not WLINK.exists():
    WLINK = WATCOM_DIR / "binnt" / "wlink.exe"

DECOMP_SRC = ROOT_DIR / "decomp" / "src"
DECOMP_INC = ROOT_DIR / "decomp" / "include"
BUILD_DIR = ROOT_DIR / "build" / "decomp"

def compile_file(src_path: Path):
    if src_path.suffix == ".asm":
        print("Error: handwritten assembly is prohibited by AGENTS.md.", file=sys.stderr)
        return False
    if not WCC386.exists():
        print(f"Error: Watcom compiler not found at {WCC386}. Run tools/setup_decomp_tools.py first.", file=sys.stderr)
        return False
        
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    obj_path = BUILD_DIR / f"{src_path.stem}.obj"
    
    obj_path.unlink(missing_ok=True)

    # Setup Watcom environment
    env = os.environ.copy()
    env["WATCOM"] = str(WATCOM_DIR)
    env["INCLUDE"] = f"{DECOMP_INC};{WATCOM_DIR / 'h'}"
    env["PATH"] = f"{WCC386.parent};{env.get('PATH', '')}"
    
    # Watcom 386 C compiler arguments:
    # -3r : 386 register calling convention (eax, edx, ebx, ecx)
    # -s  : omit stack check calls (__CHK)
    # -omaxet : optimize for maximum execution time, loops, frame pointers
    # -fo : output object path
    MODULE_FLAGS = {
        "mem": ["-3r", "-s", "-ort", "-ez"],
    }
    flags = MODULE_FLAGS.get(src_path.stem, ["-3r", "-s", "-omaxet", "-ez"])

    cmd = [
        str(WCC386),
        *flags,
        f"-fo={obj_path}",
        str(src_path)
    ]
    
    print(f"Compiling {src_path.name} -> {obj_path.name}...")
    try:
        res = subprocess.run(cmd, env=env, capture_output=True, text=True, cwd=ROOT_DIR)
    except OSError as exc:
        print(f"Compilation could not start: {exc}", file=sys.stderr)
        return False
    if res.stdout:
        print(res.stdout, end="" if res.stdout.endswith("\n") else "\n")
    if res.stderr:
        print(res.stderr, file=sys.stderr, end="" if res.stderr.endswith("\n") else "\n")
    if res.returncode != 0 or not obj_path.is_file() or obj_path.stat().st_size == 0:
        print(f"Compilation FAILED (exit {res.returncode}; fresh nonempty object required).", file=sys.stderr)
        return False
        
    print(f"Compilation SUCCESS: {obj_path} ({obj_path.stat().st_size} bytes)")
    return True

def compile_all(sources=None):
    if sources is None:
        sources = list(DECOMP_SRC.glob("*.c")) + list((DECOMP_SRC / "asm").glob("*.asm"))
    if not sources:
        print("No C/ASM source files found in decomp/src/.")
        return False
    success = True
    for s in sources:
        if not compile_file(s):
            success = False
    return success

def link_rebuilt_binary():
    """Links compiled decompiled objects using Watcom WLINK into a DOS executable."""
    wlink_script = BUILD_DIR / "wlink.lnk"
    if not wlink_script.exists():
        print(f"Error: missing link script: {wlink_script}", file=sys.stderr)
        return False
    # Use a generated strict script: never permit unresolved externals.
    script = wlink_script.read_text(encoding="utf-8")
    script = re.sub(r"(?im)^\s*option\s+undefsok\s*$", "", script)
    script = re.sub(r"(?i)tools/watcom/lib386", str(WATCOM_DIR / "lib386").replace("\\", "/"), script)
    strict_script = BUILD_DIR / "wlink_strict.lnk"
    strict_script.write_text(script, encoding="utf-8")
    rebuilt_exe = BUILD_DIR / "MAINDOS_REBUILT.EXE"
    map_path = BUILD_DIR / "MAINDOS_REBUILT.MAP"
    rebuilt_exe.unlink(missing_ok=True)
    map_path.unlink(missing_ok=True)

    if not WLINK.exists():
        print(f"Error: Watcom linker not found at {WLINK}.", file=sys.stderr)
        return False
        
    env = os.environ.copy()
    env["WATCOM"] = str(WATCOM_DIR)
    env["PATH"] = f"{WLINK.parent};{WATCOM_DIR / 'BINW'};{env.get('PATH', '')}"
    
    cmd = [str(WLINK), f"@{strict_script}"]
    print(f"Linking objects with wlink ({wlink_script.name})...")
    log_path = BUILD_DIR / "wlink.log"
    try:
        with open(log_path, "w", encoding="utf-8") as log_f:
            res = subprocess.run(cmd, env=env, stdin=subprocess.DEVNULL,
                                 stdout=log_f, stderr=subprocess.STDOUT, cwd=ROOT_DIR)
    except OSError as exc:
        print(f"Linker could not start: {exc}", file=sys.stderr)
        return False
    diagnostics = log_path.read_text(encoding="utf-8", errors="replace")
    if diagnostics:
        print(diagnostics, end="" if diagnostics.endswith("\n") else "\n")
    if (res.returncode != 0 or re.search(r"(?i)undefined|unresolved|error!", diagnostics)
            or not rebuilt_exe.is_file() or rebuilt_exe.stat().st_size == 0
            or not map_path.is_file() or map_path.stat().st_size == 0):
        print(f"Linking FAILED (exit {res.returncode}); see {log_path}", file=sys.stderr)
        return False
    print(f"Linking SUCCESS: fresh executable and map generated at {rebuilt_exe}.")
    return True

def run_in_dosbox():
    """Run from a disposable asset copy; originals are read-only inputs."""
    import shutil
    rebuilt_exe = BUILD_DIR / "MAINDOS_REBUILT.EXE"
    game_dir = ROOT_DIR / "Ignition" / "Ignition"
    runtime_dir = ROOT_DIR / "build" / "runtime" / "rebuilt"
    dosbox_exe = game_dir / "DOSBOX" / "DOSBox.exe"
    if not rebuilt_exe.exists() or not dosbox_exe.exists():
        print("Error: fresh build and DOSBox are required.", file=sys.stderr)
        return False
    shutil.copytree(game_dir, runtime_dir, dirs_exist_ok=True)
    shutil.copy2(rebuilt_exe, runtime_dir / "MREBUILT.EXE")
    return subprocess.run([str(dosbox_exe), "-c", f'mount c "{runtime_dir}"',
                           "-c", "c:", "-c", "MREBUILT.EXE"],
                          cwd=runtime_dir).returncode == 0


def main():
    import argparse
    parser = argparse.ArgumentParser(description="Build and link decompiled Watcom C files")
    parser.add_argument("sources", nargs="*", type=str, help="Specific source files to compile")
    parser.add_argument("--link", action="store_true", help="Link compiled objects with wlink")
    parser.add_argument("--run", action="store_true", help="Build, link, and launch in DOSBox")
    parser.add_argument("--slice", action="store_true", help="Re-generate assembly stubs and objdiff.json")
    args = parser.parse_args()

    if args.slice:
        sys.path.insert(0, str(ROOT_DIR / "tools"))
        from unpack_dos_le import slice_all_modules
        slice_all_modules()

    sources = [Path(p) for p in args.sources] if args.sources else None
    if not compile_all(sources):
        sys.exit(1)
        
    print("\nAll files compiled successfully.")

    if args.link or args.run:
        if not link_rebuilt_binary():
            sys.exit(1)

    if args.run:
        if not run_in_dosbox():
            sys.exit(1)

if __name__ == "__main__":
    main()
