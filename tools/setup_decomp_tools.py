#!/usr/bin/env python3
"""
Setup Decompilation Toolchain
Downloads and configures portable Open Watcom V2 and objdiff for 1:1 matching decompilation.
All downloaded binaries are placed in tools/watcom/ and tools/objdiff/ (excluded by .gitignore).
"""

import os
import shutil
import subprocess
import sys
import urllib.request
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
TOOLS_DIR = ROOT_DIR / "tools"
OBJDIFF_DIR = TOOLS_DIR / "objdiff"
WATCOM_DIR = TOOLS_DIR / "openwatcomv2"

OBJDIFF_GUI_URL = "https://github.com/encounter/objdiff/releases/download/v3.8.1/objdiff-windows-x86_64.exe"
OBJDIFF_CLI_URL = "https://github.com/encounter/objdiff/releases/download/v3.8.1/objdiff-cli-windows-x86_64.exe"
WATCOM_URL = "https://github.com/open-watcom/open-watcom-v2/releases/download/Current-build/ow-snapshot.tar.xz"

def download_file(url: str, dest: Path, desc: str):
    if dest.exists() and dest.stat().st_size > 0:
        print(f"[{desc}] Already downloaded at {dest}")
        return True
    
    print(f"[{desc}] Downloading from {url}...")
    dest.parent.mkdir(parents=True, exist_ok=True)
    temp_dest = dest.with_suffix(".tmp")
    
    req = urllib.request.Request(url, headers={"User-Agent": "RacingDynamite-Decomp"})
    try:
        with urllib.request.urlopen(req) as resp, open(temp_dest, "wb") as out:
            total = int(resp.headers.get("Content-Length", 0))
            downloaded = 0
            block_size = 1024 * 1024
            while True:
                buf = resp.read(block_size)
                if not buf:
                    break
                out.write(buf)
                downloaded += len(buf)
                if total > 0:
                    pct = downloaded / total * 100
                    mb_cur = downloaded / (1024 * 1024)
                    mb_tot = total / (1024 * 1024)
                    print(f"\r  Progress: {mb_cur:.1f}/{mb_tot:.1f} MB ({pct:.1f}%)", end="", flush=True)
                else:
                    print(f"\r  Downloaded: {downloaded / (1024 * 1024):.1f} MB", end="", flush=True)
            print()
        temp_dest.replace(dest)
        print(f"[{desc}] Download complete.")
        return True
    except Exception as e:
        print(f"[{desc}] Download failed: {e}", file=sys.stderr)
        if temp_dest.exists():
            temp_dest.unlink()
        return False

def setup_objdiff():
    print("=== Setting up objdiff ===")
    OBJDIFF_DIR.mkdir(parents=True, exist_ok=True)
    
    gui_exe = OBJDIFF_DIR / "objdiff.exe"
    cli_exe = OBJDIFF_DIR / "objdiff-cli.exe"
    
    download_file(OBJDIFF_GUI_URL, gui_exe, "objdiff GUI")
    download_file(OBJDIFF_CLI_URL, cli_exe, "objdiff CLI")
    
    if cli_exe.exists():
        try:
            res = subprocess.run([str(cli_exe), "--version"], capture_output=True, text=True)
            print(f"objdiff-cli verified: {res.stdout.strip()}")
        except Exception as e:
            print(f"objdiff-cli check error: {e}")

def setup_watcom():
    print("\n=== Setting up Open Watcom V2 ===")
    wcc386 = WATCOM_DIR / "binnt64" / "wcc386.exe"
    if not wcc386.exists():
        # Check if binnt (32-bit) exists
        wcc386 = WATCOM_DIR / "binnt" / "wcc386.exe"
        
    if wcc386.exists():
        print(f"Open Watcom already installed at {wcc386}")
        return
        
    archive_path = TOOLS_DIR / "watcom_archive.tar.xz"
    if not download_file(WATCOM_URL, archive_path, "Open Watcom Snapshot Archive"):
        return
        
    print(f"Extracting {archive_path} to {WATCOM_DIR}...")
    WATCOM_DIR.mkdir(parents=True, exist_ok=True)
    
    # Use Windows built-in tar.exe
    cmd = ["tar", "-xvf", str(archive_path), "-C", str(WATCOM_DIR)]
    res = subprocess.run(cmd, capture_output=True, text=True)
    if res.returncode != 0:
        print(f"Extraction error: {res.stderr}", file=sys.stderr)
    else:
        print("Open Watcom extraction complete.")
        if archive_path.exists():
            archive_path.unlink()

def main():
    setup_objdiff()
    setup_watcom()
    print("\nToolchain setup complete.")

if __name__ == "__main__":
    main()
