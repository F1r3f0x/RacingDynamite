#!/usr/bin/env python3
"""
Decompilation Build & Verify Tool
Compiles authentic C source files in decomp/src/ using the local portable Open Watcom V2 compiler.
Flags: -3r (register calling convention), -s (no stack checks), -omaxet (full optimization).
"""

import os
import subprocess
import sys
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
WATCOM_DIR = ROOT_DIR / "tools" / "watcom"
WCC386 = WATCOM_DIR / "binnt64" / "wcc386.exe"
if not WCC386.exists():
    WCC386 = WATCOM_DIR / "binnt" / "wcc386.exe"

WLINK = WATCOM_DIR / "binnt64" / "wlink.exe"
if not WLINK.exists():
    WLINK = WATCOM_DIR / "binnt" / "wlink.exe"

DECOMP_SRC = ROOT_DIR / "decomp" / "src"
DECOMP_INC = ROOT_DIR / "decomp" / "include"
BUILD_DIR = ROOT_DIR / "build" / "decomp"

WASM = WATCOM_DIR / "binnt64" / "wasm.exe"
if not WASM.exists():
    WASM = WATCOM_DIR / "binnt" / "wasm.exe"

def compile_file(src_path: Path):
    if src_path.suffix == ".asm":
        if not WASM.exists():
            print(f"Error: Watcom assembler not found at {WASM}.", file=sys.stderr)
            return False
            
        BUILD_DIR.mkdir(parents=True, exist_ok=True)
        obj_path = BUILD_DIR / f"{src_path.stem}_asm.obj"
        
        env = os.environ.copy()
        env["WATCOM"] = str(WATCOM_DIR)
        env["PATH"] = f"{WASM.parent};{env.get('PATH', '')}"
        
        # -3 : 386 instructions
        # -mf : flat memory model
        # -zq : quiet
        cmd = [
            str(WASM),
            "-3",
            "-mf",
            "-zq",
            f"-fo={obj_path}",
            str(src_path)
        ]
        
        print(f"Assembling {src_path.name} -> {obj_path.name}...")
        res = subprocess.run(cmd, env=env, capture_output=True, text=True)
        if res.returncode != 0:
            print(f"Assembly FAILED:\n{res.stdout}\n{res.stderr}", file=sys.stderr)
            return False
            
        print(f"Assembly SUCCESS: {obj_path} ({obj_path.stat().st_size} bytes)")
        return True

    if not WCC386.exists():
        print(f"Error: Watcom compiler not found at {WCC386}. Run tools/setup_decomp_tools.py first.", file=sys.stderr)
        return False
        
    BUILD_DIR.mkdir(parents=True, exist_ok=True)
    obj_path = BUILD_DIR / f"{src_path.stem}.obj"
    
    # Setup Watcom environment
    env = os.environ.copy()
    env["WATCOM"] = str(WATCOM_DIR)
    env["INCLUDE"] = f"{DECOMP_INC};{WATCOM_DIR / 'h'}"
    env["PATH"] = f"{WCC386.parent};{env.get('PATH', '')}"
    
    # Watcom 386 C compiler arguments:
    # -3r : 386 register calling convention (eax, edx, ebx, ecx)
    # -s  : omit stack check calls (__CHK)
    # -omaxet : optimize for maximum execution time, loops, frame pointers
    # -eoc : emit standard COFF object file for objdiff compatibility
    # -zq : quiet
    # -fo : output object path
    MODULE_FLAGS = {
        "mem": ["-3r", "-s", "-ort"],
    }
    flags = MODULE_FLAGS.get(src_path.stem, ["-3r", "-s", "-omaxet"])

    cmd = [
        str(WCC386),
        *flags,
        "-eoc",
        "-zq",
        f"-fo={obj_path}",
        str(src_path)
    ]
    
    print(f"Compiling {src_path.name} -> {obj_path.name}...")
    res = subprocess.run(cmd, env=env, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Compilation FAILED:\n{res.stdout}\n{res.stderr}", file=sys.stderr)
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
        sys.path.insert(0, str(ROOT_DIR / "tools"))
        try:
            from unpack_dos_le import slice_all_modules
            slice_all_modules()
        except Exception as e:
            print(f"Warning: Could not auto-generate wlink script: {e}")

    if not WLINK.exists():
        print(f"Error: Watcom linker not found at {WLINK}.", file=sys.stderr)
        return False
        
    env = os.environ.copy()
    env["WATCOM"] = str(WATCOM_DIR)
    env["PATH"] = f"{WLINK.parent};{env.get('PATH', '')}"
    
    cmd = [str(WLINK), f"@{wlink_script}"]
    print(f"Linking objects with wlink ({wlink_script.name})...")
    res = subprocess.run(cmd, env=env, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Link step note (partial binary links may report undefined symbols until all modules are linked):\n{res.stdout}\n{res.stderr}")
        return False
    print(f"Linking SUCCESS: {BUILD_DIR / 'MAINDOS_REBUILT.EXE'} generated successfully.")
    return True

def main():
    import argparse
    parser = argparse.ArgumentParser(description="Build and link decompiled Watcom C files")
    parser.add_argument("sources", nargs="*", type=str, help="Specific source files to compile")
    parser.add_argument("--link", action="store_true", help="Link compiled objects with wlink")
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

    if args.link:
        link_rebuilt_binary()

if __name__ == "__main__":
    main()
