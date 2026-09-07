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

DECOMP_SRC = ROOT_DIR / "decomp" / "src"
DECOMP_INC = ROOT_DIR / "decomp" / "include"
BUILD_DIR = ROOT_DIR / "build" / "decomp"

def compile_file(src_path: Path):
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
    # -zq : quiet
    # -fo : output object path
    cmd = [
        str(WCC386),
        "-3r",
        "-s",
        "-omaxet",
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

def main():
    if len(sys.argv) > 1:
        sources = [Path(p) for p in sys.argv[1:]]
    else:
        sources = list(DECOMP_SRC.glob("*.c"))
        
    if not sources:
        print("No C source files found in decomp/src/.")
        return
        
    success = True
    for s in sources:
        if not compile_file(s):
            success = False
            
    if success:
        print("\nAll files compiled successfully.")
    else:
        sys.exit(1)

if __name__ == "__main__":
    main()
