#!/usr/bin/env python3
"""
Install the retained objdiff GUI and CLI only.
Downloads go to tools/objdiff/ (excluded by .gitignore).
Retarget objdiff.json for Windows objects before using it; see tools/MIGRATION.md.
"""

import subprocess
import sys
import urllib.request
from pathlib import Path

ROOT_DIR = Path(__file__).resolve().parent.parent
TOOLS_DIR = ROOT_DIR / "tools"
OBJDIFF_DIR = TOOLS_DIR / "objdiff"

OBJDIFF_GUI_URL = "https://github.com/encounter/objdiff/releases/download/v3.8.1/objdiff-windows-x86_64.exe"
OBJDIFF_CLI_URL = "https://github.com/encounter/objdiff/releases/download/v3.8.1/objdiff-cli-windows-x86_64.exe"

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

def main():
    setup_objdiff()
    print("\nObjdiff setup complete. Retarget objdiff.json before use.")

if __name__ == "__main__":
    main()
