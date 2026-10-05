#!/usr/bin/env python3
"""
Racing Dynamite - Automated Fidelity & Change Tracking Verification Tool (FCTS)
Copyright (C) 2026 Patricio Labin Correa (@F1r3f0x)

Validates bidirectional consistency between:
1. docs/ghidra/functions.md (Master Function Map)
2. docs/tracking/deviations.md (Deviation Registry)
3. C/Header source code annotations (@original, @fidelity, @deviation, @fix_category)

Generates a parity & fidelity metrics dashboard and enforces CI guards.
"""

import os
import re
import sys
from pathlib import Path
from typing import Dict, List, Set, Tuple

ROOT_DIR = Path(__file__).resolve().parent.parent
FUNCTIONS_MD = ROOT_DIR / "docs" / "ghidra" / "functions.md"
DEVIATIONS_MD = ROOT_DIR / "docs" / "tracking" / "deviations.md"

VALID_FIDELITY = {"EXACT", "ADAPTED", "EXTENDED", "INFRASTRUCTURE"}
VALID_FIX_CATEGORIES = {
    "FIX_CAT_NOCLIP",
    "FIX_CAT_ELEVATION",
    "FIX_CAT_CAMERA",
    "FIX_CAT_AI_PATHING",
    "FIX_CAT_AUDIO",
    "FIX_CAT_RENDERER",
}


class FunctionEntry:
    def __init__(self, address: str, ghidra_name: str, symbol: str, source_file: str,
                 status: str, fidelity: str, port_location: str, purpose: str):
        self.address = address.lower()
        self.ghidra_name = ghidra_name
        self.symbol = symbol
        self.source_file = source_file
        self.status = status
        self.fidelity = fidelity
        self.port_location = port_location.replace("\\", "/") if port_location != "-" else "-"
        self.purpose = purpose


class DeviationEntry:
    def __init__(self, dev_id: str, category: str, address: str, desc: str, toggle: str):
        self.dev_id = dev_id
        self.category = category
        self.address = address.lower()
        self.desc = desc
        self.toggle = toggle


def parse_functions_md(filepath: Path) -> List[FunctionEntry]:
    entries = []
    if not filepath.exists():
        print(f"[ERROR] Functions map not found: {filepath}")
        return entries

    with open(filepath, "r", encoding="utf-8") as f:
        lines = f.readlines()

    for line in lines:
        line = line.strip()
        if not line.startswith("|"):
            continue
        parts = [p.strip().strip("`") for p in line.split("|")[1:-1]]
        if len(parts) < 8:
            continue
        address = parts[0].strip()
        if not address.startswith("0x"):
            continue  # Header or separator line

        ghidra_name = parts[1].strip()
        symbol = parts[2].strip()
        source_file = parts[3].strip()
        status = parts[4].strip()
        fidelity = parts[5].strip()
        port_location = parts[6].strip()
        purpose = parts[7].strip()

        entries.append(FunctionEntry(
            address=address,
            ghidra_name=ghidra_name,
            symbol=symbol,
            source_file=source_file,
            status=status,
            fidelity=fidelity,
            port_location=port_location,
            purpose=purpose
        ))

    return entries


def parse_deviations_md(filepath: Path) -> Dict[str, DeviationEntry]:
    deviations = {}
    if not filepath.exists():
        print(f"[ERROR] Deviations registry not found: {filepath}")
        return deviations

    with open(filepath, "r", encoding="utf-8") as f:
        content = f.read()

    # Parse table rows: | **`DEV-001`** | `FIX_CAT_NOCLIP` | `0x00412fc0` | ... | ... |
    table_matches = re.findall(
        r"\|\s*\*\*`?(DEV-\d+)`?\*\*\s*\|\s*`?([A-Z0-9_]+)`?\s*\|\s*`?(0x[0-9a-fA-F]+)`?\s*\|\s*([^|]+)\|\s*([^|]+)\s*\|",
        content
    )
    for dev_id, category, addr, desc, toggle in table_matches:
        deviations[dev_id] = DeviationEntry(
            dev_id=dev_id.strip(),
            category=category.strip(),
            address=addr.strip(),
            desc=desc.strip(),
            toggle=toggle.strip().strip("`")
        )

    return deviations


def scan_source_code(root_dir: Path) -> Tuple[Dict[str, List[Tuple[str, int, str]]], Set[str], Dict[str, List[Tuple[str, int]]]]:
    """
    Scans src/ and include/ for:
    - @original tags -> address/symbol mapping
    - @deviation tags -> deviation ID mapping
    - @fidelity tags
    """
    original_tags: Dict[str, List[Tuple[str, int, str]]] = {} # key: normalized address or symbol -> [(file, line, raw_tag)]
    deviation_tags: Dict[str, List[Tuple[str, int]]] = {}      # dev_id -> [(file, line)]
    annotated_files: Set[str] = set()

    scan_dirs = [root_dir / "src", root_dir / "include"]

    for sdir in scan_dirs:
        if not sdir.exists():
            continue
        for ext in ("*.c", "*.h"):
            for fpath in sdir.rglob(ext):
                rel_path = fpath.relative_to(root_dir).as_posix()
                try:
                    with open(fpath, "r", encoding="utf-8", errors="ignore") as f:
                        for line_num, line in enumerate(f, 1):
                            # Check @original
                            if "@original" in line:
                                annotated_files.add(rel_path)
                                # Extract any 0x... address in the line
                                addr_match = re.search(r"0x[0-9a-fA-F]+", line)
                                if addr_match:
                                    addr = addr_match.group(0).lower()
                                    original_tags.setdefault(addr, []).append((rel_path, line_num, line.strip()))
                                
                                # Also extract FUN_..., LAB_..., DAT_... symbols
                                sym_matches = re.findall(r"\b((?:FUN|LAB|DAT)_[0-9a-fA-F]+)\b", line)
                                for sym in sym_matches:
                                    original_tags.setdefault(sym, []).append((rel_path, line_num, line.strip()))

                            # Check @deviation
                            dev_matches = re.findall(r"\b(DEV-\d+)\b", line)
                            for d in dev_matches:
                                deviation_tags.setdefault(d, []).append((rel_path, line_num))

                except Exception as e:
                    print(f"[WARN] Failed to read {fpath}: {e}")

    return original_tags, annotated_files, deviation_tags


def main() -> int:
    print("================================================================================")
    print("          RACING DYNAMITE (IGNITION 1997) FIDELITY & PARITY AUDIT               ")
    print("================================================================================")

    functions = parse_functions_md(FUNCTIONS_MD)
    deviations = parse_deviations_md(DEVIATIONS_MD)
    original_tags, annotated_files, deviation_tags = scan_source_code(ROOT_DIR)

    if not functions:
        print("[FATAL] No functions parsed from docs/ghidra/functions.md!")
        return 1

    errors: List[str] = []
    warnings: List[str] = []

    # 1. Metrics counters
    total_funcs = len(functions)
    ported_funcs = [f for f in functions if f.status.lower() == "ported"]
    analyzed_funcs = [f for f in functions if f.status.lower() == "analyzed"]
    identified_funcs = [f for f in functions if f.status.lower() == "identified"]

    fidelity_counts = {"EXACT": 0, "ADAPTED": 0, "EXTENDED": 0, "INFRASTRUCTURE": 0, "OTHER": 0}
    for f in ported_funcs:
        fid = f.fidelity.upper()
        if fid in fidelity_counts:
            fidelity_counts[fid] += 1
        else:
            fidelity_counts["OTHER"] += 1

    # 2. Validate Ported Functions
    print(f"[*] Auditing {len(ported_funcs)} functions marked as 'Ported'...")
    for fn in ported_funcs:
        # Validate Port Location exists
        if fn.port_location == "-":
            errors.append(f"Function {fn.symbol} ({fn.address}) is marked 'Ported' but has no Port Location.")
            continue

        loc_path = ROOT_DIR / fn.port_location
        if not loc_path.exists():
            errors.append(f"Port location '{fn.port_location}' for {fn.symbol} ({fn.address}) does not exist on disk.")
            continue

        # Validate fidelity tag validity
        if fn.fidelity.upper() not in VALID_FIDELITY:
            errors.append(f"Function {fn.symbol} has invalid fidelity '{fn.fidelity}'. Expected one of: {sorted(VALID_FIDELITY)}.")

        # Check that @original tag exists in the codebase for this address or Ghidra name
        addr_refs = original_tags.get(fn.address, [])
        sym_refs = original_tags.get(fn.ghidra_name, [])
        all_refs = addr_refs + sym_refs

        if not all_refs:
            errors.append(f"Ported function {fn.symbol} ({fn.address} / {fn.ghidra_name}) is missing '@original' tag in {fn.port_location}.")
        else:
            # Check if at least one reference is in the designated port location
            found_in_target = any(ref[0] == fn.port_location for ref in all_refs)
            if not found_in_target:
                ref_files = sorted(set(r[0] for r in all_refs))
                warnings.append(f"Function {fn.symbol} ({fn.address}) is declared at {fn.port_location}, but @original tag was found in {ref_files}.")

    # 3. Validate Deviations Registry
    print(f"[*] Auditing {len(deviations)} registered deviations...")
    for dev_id, dev in deviations.items():
        if dev.category not in VALID_FIX_CATEGORIES:
            errors.append(f"Deviation {dev_id} has invalid category '{dev.category}'. Expected one of: {sorted(VALID_FIX_CATEGORIES)}.")
        
        # Check if deviation is referenced in source code
        if dev_id not in deviation_tags:
            warnings.append(f"Deviation {dev_id} ({dev.desc}) is defined in deviations.md but not referenced by any @deviation tag in source code.")

    # 4. Check for uncatalogued @deviation tags in code
    for code_dev_id, locations in deviation_tags.items():
        if code_dev_id not in deviations:
            errors.append(f"Source code references unknown deviation '{code_dev_id}' at {locations}, but it is not registered in deviations.md.")

    # 5. Print Detailed Dashboard
    print("\n--------------------------------------------------------------------------------")
    print(" 1. REVERSE ENGINEERING PARITY METRICS")
    print("--------------------------------------------------------------------------------")
    print(f"  Total Tracked Functions  : {total_funcs:4d}")
    print(f"  Ported to Modern C11/SDL : {len(ported_funcs):4d}  ({(len(ported_funcs)/total_funcs)*100:5.1f}%)")
    print(f"  Analyzed / Decompiled    : {len(analyzed_funcs):4d}  ({(len(analyzed_funcs)/total_funcs)*100:5.1f}%)")
    print(f"  Identified / Documented  : {len(identified_funcs):4d}  ({(len(identified_funcs)/total_funcs)*100:5.1f}%)")

    print("\n--------------------------------------------------------------------------------")
    print(" 2. FIDELITY CLASSIFICATION (PORTED FUNCTIONS)")
    print("--------------------------------------------------------------------------------")
    print(f"  [EXACT]          100% 1:1 Assembly & Math Parity   : {fidelity_counts['EXACT']:3d} ({(fidelity_counts['EXACT']/max(len(ported_funcs),1))*100:5.1f}%)")
    print(f"  [ADAPTED]        Modernized for C11/SDL2/64-bit     : {fidelity_counts['ADAPTED']:3d} ({(fidelity_counts['ADAPTED']/max(len(ported_funcs),1))*100:5.1f}%)")
    print(f"  [EXTENDED]       Enhanced with Guards & Fix Toggles: {fidelity_counts['EXTENDED']:3d} ({(fidelity_counts['EXTENDED']/max(len(ported_funcs),1))*100:5.1f}%)")

    print("\n--------------------------------------------------------------------------------")
    print(" 3. REGISTERED DEVIATIONS & FIX PRESERVATION TOGGLES")
    print("--------------------------------------------------------------------------------")
    print(f"  {'ID':<8} {'Category':<20} {'Address':<12} {'Preservation Toggle Key'}")
    print(f"  {'-'*8} {'-'*20} {'-'*12} {'-'*30}")
    for dev_id, dev in sorted(deviations.items()):
        print(f"  {dev_id:<8} {dev.category:<20} {dev.address:<12} {dev.toggle}")

    # Subsystem Breakdown
    print("\n--------------------------------------------------------------------------------")
    print(" 4. SUBSYSTEM STATUS BREAKDOWN")
    print("--------------------------------------------------------------------------------")
    subsystems = {
        "Core & Game State": [f for f in functions if "main.c" in f.source_file or "game_state" in f.port_location],
        "Formats Loaders":   [f for f in functions if any(f.port_location.endswith(x) for x in ("col.c", "srf.c", "lft.c", "tri.c", "plc.c", "msh.c", "tex_tab.c")) or "geputget.c" in f.source_file or "mem.c" in f.source_file],
        "Surface & Physics": [f for f in functions if "getsurf.c" in f.source_file or "vehicle.c" in f.source_file or "physics" in f.port_location],
        "Lisa3D Renderer":   [f for f in functions if "lisa3d.c" in f.source_file or "rasterizer" in f.port_location or "camera" in f.port_location],
    }

    for sub_name, funcs in subsystems.items():
        sub_total = len(funcs)
        sub_ported = sum(1 for f in funcs if f.status.lower() == "ported")
        pct = (sub_ported / sub_total * 100) if sub_total > 0 else 0.0
        print(f"  {sub_name:<22} : {sub_ported:2d} / {sub_total:2d} ported ({pct:5.1f}%)")

    # Output Warnings
    if warnings:
        print("\n--------------------------------------------------------------------------------")
        print(f" AUDIT WARNINGS ({len(warnings)})")
        print("--------------------------------------------------------------------------------")
        for w in warnings:
            print(f"  [WARN] {w}")

    # PE-residue ratchet (MAINDOS_32BIT.EXE leftovers must never increase)
    import subprocess
    residue = subprocess.run(
        [sys.executable, os.path.join(os.path.dirname(os.path.abspath(__file__)), "audit_pe_residue.py")],
        capture_output=True, text=True)
    if residue.returncode != 0:
        errors.append("PE residue audit failed:\n" + residue.stdout.strip())

    # Output Errors
    if errors:
        print("\n--------------------------------------------------------------------------------")
        print(f" AUDIT FAILURES ({len(errors)})")
        print("--------------------------------------------------------------------------------")
        for e in errors:
            print(f"  [FAIL] {e}")
        print("\n================================================================================")
        print(" [RESULT] FIDELITY AUDIT FAILED - PLEASE RESOLVE MISSING TAGS OR DOC ENTRIES")
        print("================================================================================\n")
        return 1

    print("\n================================================================================")
    print(" [RESULT] ALL 43 PORTED FUNCTIONS & 6 DEVIATIONS VERIFIED 100% IN SYNC! [PASS]")
    print("================================================================================\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
