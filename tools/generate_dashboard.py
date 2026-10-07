#!/usr/bin/env python3
"""
Racing Dynamite (Ignition 1997) - Decompilation Progress Dashboard Generator
Generates an interactive, zero-dependency, self-contained HTML progress dashboard
from database/decomp.db and documentation registries.
"""

import argparse
import datetime
import json
import os
import re
import sqlite3
import sys
from pathlib import Path
from typing import Any, Dict, List

ROOT_DIR = Path(__file__).resolve().parent.parent
DB_PATH = ROOT_DIR / "database" / "decomp.db"
DEVIATIONS_MD = ROOT_DIR / "docs" / "tracking" / "deviations.md"
DECOMP_SRC = ROOT_DIR / "decomp" / "src"
BUILD_DECOMP = ROOT_DIR / "build" / "decomp"


def get_db_data() -> Dict[str, Any]:
    if not DB_PATH.exists():
        print(f"[ERROR] Database not found at {DB_PATH}", file=sys.stderr)
        sys.exit(1)

    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    cur = conn.cursor()

    # 1. Metadata
    metadata = {}
    try:
        cur.execute("SELECT key, value FROM metadata")
        for row in cur.fetchall():
            metadata[row["key"]] = row["value"]
    except Exception:
        pass

    # 2. Modules
    cur.execute("""
        SELECT m.id, m.name, m.original_path, m.decomp_path, m.description,
               COUNT(f.id) as total_funcs,
               SUM(CASE WHEN f.status = 'matching' THEN 1 ELSE 0 END) as matching_funcs,
               SUM(CASE WHEN f.status = 'decompiled' THEN 1 ELSE 0 END) as decompiled_funcs,
               SUM(CASE WHEN f.status = 'analyzed' THEN 1 ELSE 0 END) as analyzed_funcs,
               SUM(CASE WHEN f.status = 'unidentified' THEN 1 ELSE 0 END) as unidentified_funcs
        FROM modules m
        LEFT JOIN functions f ON f.module_id = m.id
        GROUP BY m.id
        ORDER BY total_funcs DESC, m.name ASC
    """)
    modules = []
    for r in cur.fetchall():
        total = r["total_funcs"]
        decomp_or_match = (r["matching_funcs"] or 0) + (r["decompiled_funcs"] or 0)
        pct = round((decomp_or_match / total * 100), 1) if total > 0 else 0.0
        modules.append({
            "id": r["id"],
            "name": r["name"],
            "original_path": r["original_path"] or "",
            "decomp_path": r["decomp_path"] or "",
            "description": r["description"] or "",
            "total": total,
            "matching": r["matching_funcs"] or 0,
            "decompiled": r["decompiled_funcs"] or 0,
            "analyzed": r["analyzed_funcs"] or 0,
            "unidentified": r["unidentified_funcs"] or 0,
            "percent": pct,
        })

    # 3. Functions
    cur.execute("""
        SELECT f.id, f.dos_address, f.win_address, f.symbol_name, f.original_ghidra_name,
               f.module_id, COALESCE(m.name, 'Unknown') as module_name,
               f.status, f.calling_convention, f.return_type, f.parameters,
               f.byte_size, f.line_count, f.fidelity, f.port_location, f.purpose, f.notes
        FROM functions f
        LEFT JOIN modules m ON f.module_id = m.id
        ORDER BY 
            CASE f.status 
                WHEN 'matching' THEN 1 
                WHEN 'decompiled' THEN 2 
                WHEN 'analyzed' THEN 3 
                ELSE 4 
            END,
            f.win_address ASC
    """)
    functions = []
    for r in cur.fetchall():
        functions.append({
            "id": r["id"],
            "dos_address": r["dos_address"] or "-",
            "win_address": r["win_address"] or "-",
            "symbol_name": r["symbol_name"],
            "original_ghidra_name": r["original_ghidra_name"] or "-",
            "module_id": r["module_id"],
            "module_name": r["module_name"],
            "status": r["status"] or "unidentified",
            "calling_convention": r["calling_convention"] or "",
            "return_type": r["return_type"] or "",
            "parameters": r["parameters"] or "",
            "byte_size": r["byte_size"],
            "line_count": r["line_count"],
            "fidelity": r["fidelity"] or "-",
            "port_location": r["port_location"] or "-",
            "purpose": r["purpose"] or "",
            "notes": r["notes"] or "",
        })

    # 4. Globals
    cur.execute("""
        SELECT g.id, g.dos_address, g.win_address, g.name, g.type, g.size, g.description,
               COALESCE(m.name, '-') as module_name
        FROM globals g
        LEFT JOIN modules m ON g.module_id = m.id
        ORDER BY g.win_address ASC
    """)
    globals_list = []
    for r in cur.fetchall():
        globals_list.append({
            "id": r["id"],
            "dos_address": r["dos_address"] or "-",
            "win_address": r["win_address"] or "-",
            "name": r["name"],
            "type": r["type"],
            "size": r["size"],
            "description": r["description"] or "",
            "module_name": r["module_name"],
        })

    # 5. Structs & Fields
    cur.execute("""
        SELECT s.id, s.name, s.size, s.description, s.notes,
               COALESCE(m.name, '-') as module_name
        FROM structs s
        LEFT JOIN modules m ON s.module_id = m.id
        ORDER BY s.name ASC
    """)
    structs = []
    for s_row in cur.fetchall():
        s_id = s_row["id"]

        cur.execute("""
            SELECT offset, type, name, size, description
            FROM struct_fields
            WHERE struct_id = ?
            ORDER BY offset ASC
        """, (s_id,))
        fields = []
        for f in cur.fetchall():
            fields.append({
                "offset": f["offset"],
                "offset_hex": f"0x{f['offset']:02X}",
                "type": f["type"],
                "name": f["name"],
                "size": f["size"],
                "description": f["description"] or "",
            })
        structs.append({
            "id": s_id,
            "name": s_row["name"],
            "size": s_row["size"],
            "description": s_row["description"] or "",
            "notes": s_row["notes"] or "",
            "module_name": s_row["module_name"],
            "fields": fields,
        })

    # 6. Deviations
    deviations = []
    if DEVIATIONS_MD.exists():
        try:
            with open(DEVIATIONS_MD, "r", encoding="utf-8") as f:
                content = f.read()
            table_matches = re.findall(
                r"\|\s*\*\*`?(DEV-\d+)`?\*\*\s*\|\s*`?([A-Z0-9_]+)`?\s*\|\s*`?(0x[0-9a-fA-F]+)`?\s*\|\s*([^|]+)\|\s*([^|]+)\s*\|",
                content
            )
            for dev_id, category, addr, desc, toggle in table_matches:
                deviations.append({
                    "id": dev_id.strip(),
                    "category": category.strip(),
                    "address": addr.strip(),
                    "description": desc.strip(),
                    "toggle": toggle.strip().strip("`"),
                })
        except Exception as e:
            print(f"[WARN] Failed to parse deviations.md: {e}")

    # Fallback to DB if deviations empty
    if not deviations:
        try:
            cur.execute("SELECT id, category, title, description, win_address, toggle_key FROM deviations")
            for r in cur.fetchall():
                deviations.append({
                    "id": r["id"],
                    "category": r["category"],
                    "address": r["win_address"] or "-",
                    "description": r["description"] or r["title"],
                    "toggle": r["toggle_key"] or "-",
                })
        except Exception:
            pass

    # 7. Watcom build status
    watcom_info = {
        "source_files": [],
        "object_files": [],
    }
    if DECOMP_SRC.exists():
        for cfile in DECOMP_SRC.glob("*.c"):
            watcom_info["source_files"].append({
                "name": cfile.name,
                "size": cfile.stat().st_size,
            })
    if BUILD_DECOMP.exists():
        for obj in BUILD_DECOMP.glob("*.obj"):
            watcom_info["object_files"].append({
                "name": obj.name,
                "size": obj.stat().st_size,
            })

    conn.close()

    # Calculate aggregate summary stats
    total_funcs = len(functions)
    matching_funcs = sum(1 for f in functions if f["status"] == "matching")
    decompiled_funcs = sum(1 for f in functions if f["status"] == "decompiled")
    analyzed_funcs = sum(1 for f in functions if f["status"] == "analyzed")
    unidentified_funcs = sum(1 for f in functions if f["status"] == "unidentified")
    completed_funcs = matching_funcs + decompiled_funcs
    completion_pct = round((completed_funcs / total_funcs * 100), 1) if total_funcs > 0 else 0.0
    matching_pct = round((matching_funcs / total_funcs * 100), 1) if total_funcs > 0 else 0.0

    # Fidelity stats
    fidelity_counts = {"EXACT": 0, "ADAPTED": 0, "EXTENDED": 0, "INFRASTRUCTURE": 0, "-": 0}
    for f in functions:
        fid = f["fidelity"].upper()
        if fid in fidelity_counts:
            fidelity_counts[fid] += 1
        else:
            fidelity_counts.setdefault(fid, 0)
            fidelity_counts[fid] += 1

    summary = {
        "total_functions": total_funcs,
        "matching_functions": matching_funcs,
        "decompiled_functions": decompiled_funcs,
        "analyzed_functions": analyzed_funcs,
        "unidentified_functions": unidentified_funcs,
        "completed_functions": completed_funcs,
        "completion_percent": completion_pct,
        "matching_percent": matching_pct,
        "total_modules": len(modules),
        "total_globals": len(globals_list),
        "total_structs": len(structs),
        "total_struct_fields": sum(len(s["fields"]) for s in structs),
        "total_deviations": len(deviations),
        "fidelity_counts": fidelity_counts,
        "generated_at": datetime.datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
    }

    return {
        "metadata": metadata,
        "summary": summary,
        "modules": modules,
        "functions": functions,
        "globals": globals_list,
        "structs": structs,
        "deviations": deviations,
        "watcom": watcom_info,
    }


def generate_html(data: Dict[str, Any]) -> str:
    # Serialize data for embedded client-side JavaScript
    json_data = json.dumps(data, ensure_ascii=False)
    summary = data["summary"]

    html = f"""<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Racing Dynamite // Decompilation Progress Dashboard</title>
  <style>
    :root {{
      --bg: #0b0f19;
      --bg-surface: #111827;
      --bg-card: #172136;
      --bg-card-hover: #1e2c47;
      --border: #243452;
      --border-subtle: #1a273e;
      --text: #f3f4f6;
      --text-muted: #94a3b8;
      --text-dim: #64748b;
      
      --color-matching: #10b981;
      --color-matching-bg: rgba(16, 185, 129, 0.15);
      --color-decomp: #06b6d4;
      --color-decomp-bg: rgba(6, 182, 212, 0.15);
      --color-analyzed: #f59e0b;
      --color-analyzed-bg: rgba(245, 158, 11, 0.15);
      --color-unid: #64748b;
      --color-unid-bg: rgba(100, 116, 139, 0.15);
      
      --accent-red: #ef4444;
      --accent-orange: #f97316;
      --accent-cyan: #38bdf8;
      
      --font-mono: ui-monospace, SFMono-Regular, Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
      --radius: 8px;
    }}

    * {{
      box-sizing: border-box;
      margin: 0;
      padding: 0;
    }}

    body {{
      background-color: var(--bg);
      color: var(--text);
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
      line-height: 1.5;
      padding: 24px;
      min-height: 100vh;
    }}

    /* Scrollbars */
    ::-webkit-scrollbar {{
      width: 8px;
      height: 8px;
    }}
    ::-webkit-scrollbar-track {{
      background: var(--bg);
    }}
    ::-webkit-scrollbar-thumb {{
      background: var(--border);
      border-radius: 4px;
    }}
    ::-webkit-scrollbar-thumb:hover {{
      background: var(--text-dim);
    }}

    .container {{
      max-width: 1560px;
      margin: 0 auto;
    }}

    /* Header */
    header {{
      display: flex;
      flex-wrap: wrap;
      justify-content: space-between;
      align-items: center;
      gap: 16px;
      padding-bottom: 20px;
      border-bottom: 1px solid var(--border);
      margin-bottom: 24px;
    }}

    .title-group {{
      display: flex;
      flex-direction: column;
      gap: 4px;
    }}

    .logo-row {{
      display: flex;
      align-items: center;
      gap: 12px;
    }}

    .badge-flag {{
      background: linear-gradient(135deg, #ef4444, #f97316);
      color: #fff;
      font-weight: 800;
      font-size: 11px;
      letter-spacing: 1px;
      padding: 3px 8px;
      border-radius: 4px;
      text-transform: uppercase;
    }}

    h1 {{
      font-size: 24px;
      font-weight: 800;
      letter-spacing: -0.5px;
      color: #fff;
      display: flex;
      align-items: center;
      gap: 8px;
    }}

    .subtitle {{
      color: var(--text-muted);
      font-size: 13px;
    }}

    .header-badges {{
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      align-items: center;
    }}

    .badge {{
      display: inline-flex;
      align-items: center;
      gap: 6px;
      font-size: 12px;
      font-family: var(--font-mono);
      background: var(--bg-card);
      border: 1px solid var(--border);
      padding: 5px 10px;
      border-radius: 6px;
      color: var(--text-muted);
    }}

    .badge strong {{
      color: var(--text);
    }}

    .badge-live {{
      border-color: rgba(16, 185, 129, 0.4);
      background: rgba(16, 185, 129, 0.1);
      color: #10b981;
    }}

    .badge-dot {{
      width: 7px;
      height: 7px;
      border-radius: 50%;
      background: #10b981;
      display: inline-block;
      box-shadow: 0 0 8px #10b981;
    }}

    /* KPI Cards Grid */
    .metrics-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fit, minmax(210px, 1fr));
      gap: 14px;
      margin-bottom: 24px;
    }}

    .metric-card {{
      background: var(--bg-surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 16px 18px;
      display: flex;
      flex-direction: column;
      gap: 8px;
      transition: transform 0.15s ease, border-color 0.15s ease;
    }}

    .metric-card:hover {{
      border-color: var(--text-dim);
      transform: translateY(-1px);
    }}

    .metric-label {{
      font-size: 11px;
      font-weight: 600;
      text-transform: uppercase;
      letter-spacing: 0.8px;
      color: var(--text-dim);
    }}

    .metric-value-row {{
      display: flex;
      align-items: baseline;
      gap: 8px;
    }}

    .metric-val {{
      font-size: 28px;
      font-weight: 800;
      color: #fff;
      font-family: var(--font-mono);
      line-height: 1;
    }}

    .metric-sub {{
      font-size: 12px;
      color: var(--text-muted);
    }}

    .metric-card.primary {{
      background: linear-gradient(145deg, #111e38, #162444);
      border-color: #2b4573;
    }}

    .metric-card.primary .metric-val {{
      color: #38bdf8;
      text-shadow: 0 0 12px rgba(56, 189, 248, 0.4);
    }}

    .metric-card.matching {{
      border-left: 4px solid var(--color-matching);
    }}
    .metric-card.matching .metric-val {{
      color: var(--color-matching);
    }}

    .metric-card.decomp {{
      border-left: 4px solid var(--color-decomp);
    }}
    .metric-card.decomp .metric-val {{
      color: var(--color-decomp);
    }}

    .metric-card.analyzed {{
      border-left: 4px solid var(--color-analyzed);
    }}
    .metric-card.analyzed .metric-val {{
      color: var(--color-analyzed);
    }}

    /* Global Progress Bar */
    .progress-section {{
      background: var(--bg-surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 16px 20px;
      margin-bottom: 24px;
    }}

    .progress-header {{
      display: flex;
      justify-content: space-between;
      align-items: center;
      margin-bottom: 10px;
      font-size: 13px;
    }}

    .progress-title {{
      font-weight: 700;
      color: #fff;
      display: flex;
      align-items: center;
      gap: 8px;
    }}

    .progress-pct {{
      font-family: var(--font-mono);
      font-weight: 700;
      color: var(--accent-cyan);
      font-size: 15px;
    }}

    .bar-container {{
      width: 100%;
      height: 14px;
      background: #0f1624;
      border-radius: 7px;
      overflow: hidden;
      display: flex;
      box-shadow: inset 0 2px 4px rgba(0,0,0,0.5);
    }}

    .bar-segment {{
      height: 100%;
      transition: width 0.3s ease;
    }}

    .bar-matching {{
      background: var(--color-matching);
      box-shadow: 0 0 8px rgba(16, 185, 129, 0.5);
    }}

    .bar-decomp {{
      background: var(--color-decomp);
      box-shadow: 0 0 8px rgba(6, 182, 212, 0.5);
    }}

    .bar-analyzed {{
      background: var(--color-analyzed);
      box-shadow: 0 0 8px rgba(245, 158, 11, 0.5);
    }}

    .progress-legend {{
      display: flex;
      flex-wrap: wrap;
      gap: 18px;
      margin-top: 12px;
      font-size: 12px;
    }}

    .legend-item {{
      display: flex;
      align-items: center;
      gap: 6px;
      color: var(--text-muted);
    }}

    .legend-box {{
      width: 10px;
      height: 10px;
      border-radius: 2px;
    }}

    /* Navigation Tabs */
    .nav-tabs {{
      display: flex;
      gap: 4px;
      border-bottom: 1px solid var(--border);
      margin-bottom: 20px;
      overflow-x: auto;
    }}

    .tab-btn {{
      background: none;
      border: none;
      color: var(--text-dim);
      font-size: 13px;
      font-weight: 600;
      padding: 10px 16px;
      cursor: pointer;
      border-bottom: 2px solid transparent;
      display: flex;
      align-items: center;
      gap: 8px;
      transition: all 0.15s ease;
      white-space: nowrap;
    }}

    .tab-btn:hover {{
      color: var(--text);
      background: rgba(255, 255, 255, 0.03);
    }}

    .tab-btn.active {{
      color: #fff;
      border-bottom-color: var(--accent-cyan);
      background: rgba(56, 189, 248, 0.05);
    }}

    .tab-count {{
      font-size: 11px;
      font-family: var(--font-mono);
      background: var(--bg-card);
      border: 1px solid var(--border);
      padding: 2px 6px;
      border-radius: 10px;
      color: var(--text-muted);
    }}

    .tab-btn.active .tab-count {{
      background: rgba(56, 189, 248, 0.2);
      border-color: var(--accent-cyan);
      color: #fff;
    }}

    /* Tab Content Views */
    .tab-view {{
      display: none;
    }}

    .tab-view.active {{
      display: block;
    }}

    /* Controls Bar */
    .controls-bar {{
      display: flex;
      flex-wrap: wrap;
      gap: 12px;
      align-items: center;
      justify-content: space-between;
      margin-bottom: 16px;
      background: var(--bg-surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 12px 16px;
    }}

    .search-wrapper {{
      flex: 1;
      min-width: 260px;
      position: relative;
    }}

    .search-input {{
      width: 100%;
      background: var(--bg-card);
      border: 1px solid var(--border);
      border-radius: 6px;
      padding: 8px 12px 8px 34px;
      color: #fff;
      font-size: 13px;
      outline: none;
      transition: border-color 0.15s ease;
    }}

    .search-input:focus {{
      border-color: var(--accent-cyan);
      box-shadow: 0 0 0 2px rgba(56, 189, 248, 0.2);
    }}

    .search-icon {{
      position: absolute;
      left: 10px;
      top: 50%;
      transform: translateY(-50%);
      color: var(--text-dim);
      pointer-events: none;
      font-size: 14px;
    }}

    .filter-group {{
      display: flex;
      flex-wrap: wrap;
      gap: 8px;
      align-items: center;
    }}

    .select-filter {{
      background: var(--bg-card);
      border: 1px solid var(--border);
      border-radius: 6px;
      color: var(--text);
      font-size: 12px;
      padding: 8px 12px;
      outline: none;
      cursor: pointer;
    }}

    .select-filter:focus {{
      border-color: var(--accent-cyan);
    }}

    .pill-btn {{
      background: var(--bg-card);
      border: 1px solid var(--border);
      border-radius: 6px;
      color: var(--text-muted);
      font-size: 12px;
      font-weight: 500;
      padding: 6px 12px;
      cursor: pointer;
      transition: all 0.15s ease;
    }}

    .pill-btn:hover {{
      color: #fff;
      border-color: var(--text-dim);
    }}

    .pill-btn.active {{
      background: rgba(56, 189, 248, 0.15);
      border-color: var(--accent-cyan);
      color: #fff;
    }}

    .results-count {{
      font-size: 12px;
      color: var(--text-dim);
      font-family: var(--font-mono);
    }}

    /* Tables */
    .table-container {{
      background: var(--bg-surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      overflow-x: auto;
    }}

    table {{
      width: 100%;
      border-collapse: collapse;
      text-align: left;
      font-size: 13px;
    }}

    th {{
      background: #131d30;
      color: var(--text-muted);
      font-weight: 600;
      padding: 10px 14px;
      border-bottom: 1px solid var(--border);
      text-transform: uppercase;
      font-size: 11px;
      letter-spacing: 0.5px;
      white-space: nowrap;
      user-select: none;
    }}

    th.sortable {{
      cursor: pointer;
    }}

    th.sortable:hover {{
      color: #fff;
    }}

    td {{
      padding: 10px 14px;
      border-bottom: 1px solid var(--border-subtle);
      vertical-align: middle;
    }}

    tr:hover td {{
      background: rgba(255, 255, 255, 0.02);
    }}

    .font-mono {{
      font-family: var(--font-mono);
    }}

    .addr {{
      font-family: var(--font-mono);
      font-size: 12px;
      color: #93c5fd;
      cursor: pointer;
      padding: 2px 5px;
      border-radius: 4px;
      display: inline-block;
      transition: background 0.15s;
    }}

    .addr:hover {{
      background: rgba(147, 197, 253, 0.15);
    }}

    .symbol-name {{
      font-family: var(--font-mono);
      font-weight: 600;
      color: #fff;
    }}

    .ghidra-label {{
      font-family: var(--font-mono);
      font-size: 11px;
      color: var(--text-dim);
    }}

    .status-badge {{
      display: inline-flex;
      align-items: center;
      gap: 5px;
      font-size: 11px;
      font-weight: 700;
      text-transform: uppercase;
      letter-spacing: 0.4px;
      padding: 3px 8px;
      border-radius: 4px;
      white-space: nowrap;
    }}

    .status-matching {{
      background: var(--color-matching-bg);
      color: var(--color-matching);
      border: 1px solid rgba(16, 185, 129, 0.4);
    }}

    .status-decompiled {{
      background: var(--color-decomp-bg);
      color: var(--color-decomp);
      border: 1px solid rgba(6, 182, 212, 0.4);
    }}

    .status-analyzed {{
      background: var(--color-analyzed-bg);
      color: var(--color-analyzed);
      border: 1px solid rgba(245, 158, 11, 0.4);
    }}

    .status-unidentified {{
      background: var(--color-unid-bg);
      color: var(--color-unid);
      border: 1px solid rgba(100, 116, 139, 0.4);
    }}

    .fidelity-tag {{
      display: inline-block;
      font-size: 10px;
      font-family: var(--font-mono);
      font-weight: 600;
      padding: 2px 6px;
      border-radius: 4px;
      background: #1f293d;
      color: #94a3b8;
      border: 1px solid #2d3c59;
    }}

    .fidelity-EXACT {{
      background: rgba(16, 185, 129, 0.15);
      color: #10b981;
      border-color: rgba(16, 185, 129, 0.3);
    }}

    .fidelity-ADAPTED {{
      background: rgba(56, 189, 248, 0.15);
      color: #38bdf8;
      border-color: rgba(56, 189, 248, 0.3);
    }}

    .fidelity-EXTENDED {{
      background: rgba(245, 158, 11, 0.15);
      color: #f59e0b;
      border-color: rgba(245, 158, 11, 0.3);
    }}

    .port-loc {{
      font-size: 11px;
      font-family: var(--font-mono);
      color: var(--text-dim);
    }}

    .purpose-text {{
      color: var(--text-muted);
      font-size: 12px;
      max-width: 450px;
      line-height: 1.4;
    }}

    /* Module Cards View */
    .modules-grid {{
      display: grid;
      grid-template-columns: repeat(auto-fill, minmax(360px, 1fr));
      gap: 16px;
    }}

    .module-card {{
      background: var(--bg-surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 18px 20px;
      display: flex;
      flex-direction: column;
      gap: 12px;
    }}

    .module-card-header {{
      display: flex;
      justify-content: space-between;
      align-items: baseline;
    }}

    .module-card-title {{
      font-size: 16px;
      font-weight: 700;
      color: #fff;
      font-family: var(--font-mono);
    }}

    .module-card-pct {{
      font-size: 15px;
      font-weight: 800;
      color: var(--accent-cyan);
      font-family: var(--font-mono);
    }}

    .module-desc {{
      font-size: 12px;
      color: var(--text-muted);
      min-height: 36px;
    }}

    .module-bar-wrap {{
      width: 100%;
      height: 8px;
      background: #0f1624;
      border-radius: 4px;
      overflow: hidden;
      display: flex;
    }}

    .module-stats-row {{
      display: flex;
      justify-content: space-between;
      font-size: 12px;
      color: var(--text-dim);
      font-family: var(--font-mono);
      padding-top: 4px;
      border-top: 1px solid var(--border-subtle);
    }}

    .module-action-btn {{
      margin-top: 4px;
      width: 100%;
      padding: 7px;
      background: var(--bg-card);
      border: 1px solid var(--border);
      color: var(--text);
      font-size: 12px;
      font-weight: 600;
      border-radius: 6px;
      cursor: pointer;
      transition: background 0.15s;
    }}

    .module-action-btn:hover {{
      background: var(--bg-card-hover);
      border-color: var(--text-dim);
    }}

    /* Struct Accordion */
    .struct-card {{
      background: var(--bg-surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      margin-bottom: 14px;
      overflow: hidden;
    }}

    .struct-header {{
      padding: 14px 18px;
      background: #141c2e;
      display: flex;
      justify-content: space-between;
      align-items: center;
      cursor: pointer;
      user-select: none;
      border-bottom: 1px solid transparent;
    }}

    .struct-header:hover {{
      background: #18233a;
    }}

    .struct-card.open .struct-header {{
      border-bottom-color: var(--border);
    }}

    .struct-name {{
      font-size: 15px;
      font-weight: 700;
      font-family: var(--font-mono);
      color: #fff;
    }}

    .struct-meta {{
      display: flex;
      gap: 12px;
      font-size: 12px;
      color: var(--text-dim);
      font-family: var(--font-mono);
    }}

    .struct-body {{
      display: none;
      padding: 0;
    }}

    .struct-card.open .struct-body {{
      display: block;
    }}

    /* Deviations */
    .deviation-card {{
      background: var(--bg-surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 16px 20px;
      margin-bottom: 12px;
      display: flex;
      flex-direction: column;
      gap: 8px;
    }}

    .dev-top-row {{
      display: flex;
      justify-content: space-between;
      align-items: center;
    }}

    .dev-id {{
      font-size: 14px;
      font-weight: 800;
      font-family: var(--font-mono);
      color: var(--accent-orange);
    }}

    .dev-cat {{
      font-size: 11px;
      font-family: var(--font-mono);
      padding: 2px 8px;
      border-radius: 4px;
      background: rgba(249, 115, 22, 0.15);
      color: var(--accent-orange);
      border: 1px solid rgba(249, 115, 22, 0.3);
    }}

    .dev-desc {{
      font-size: 13px;
      color: var(--text);
    }}

    .dev-toggle {{
      font-size: 12px;
      font-family: var(--font-mono);
      color: var(--accent-cyan);
      background: #0f1624;
      padding: 4px 8px;
      border-radius: 4px;
      display: inline-block;
      align-self: flex-start;
      border: 1px solid #1e293b;
    }}

    /* Watcom & Info */
    .info-card {{
      background: var(--bg-surface);
      border: 1px solid var(--border);
      border-radius: var(--radius);
      padding: 20px;
      margin-bottom: 16px;
    }}

    .info-card h3 {{
      font-size: 16px;
      color: #fff;
      margin-bottom: 10px;
      display: flex;
      align-items: center;
      gap: 8px;
    }}

    .code-block {{
      background: #0a0d14;
      border: 1px solid var(--border);
      border-radius: 6px;
      padding: 12px 16px;
      font-family: var(--font-mono);
      font-size: 12px;
      color: #94a3b8;
      overflow-x: auto;
      line-height: 1.6;
    }}

    footer {{
      margin-top: 40px;
      padding-top: 20px;
      border-top: 1px solid var(--border);
      display: flex;
      justify-content: space-between;
      align-items: center;
      font-size: 12px;
      color: var(--text-dim);
    }}

    /* Toast Notification */
    #toast {{
      position: fixed;
      bottom: 24px;
      right: 24px;
      background: #1e293b;
      border: 1px solid var(--accent-cyan);
      color: #fff;
      padding: 8px 16px;
      border-radius: 6px;
      font-size: 12px;
      font-family: var(--font-mono);
      box-shadow: 0 4px 12px rgba(0,0,0,0.5);
      opacity: 0;
      transform: translateY(10px);
      transition: all 0.2s ease;
      pointer-events: none;
      z-index: 1000;
    }}
    #toast.show {{
      opacity: 1;
      transform: translateY(0);
    }}
  </style>
</head>
<body>
  <aside style="padding:16px;background:#4b2e0b;color:#fff;text-align:center">
    <strong>LEGACY DOS PROGRESS — not Windows completion.</strong>
    Active Windows evidence: docs/ghidra/windows_startup.md and
    docs/tracking/windows_inventory.json (one bounded routine).
  </aside>
  <div class="container">
    <!-- Header -->
    <header>
      <div class="title-group">
        <div class="logo-row">
          <span class="badge-flag">1997 RE</span>
          <h1>🏁 RACING DYNAMITE</h1>
        </div>
        <div class="subtitle">Ignition (1997) Reverse Engineering &amp; Modern C11 Source Port Dashboard</div>
      </div>
      <div class="header-badges">
        <div class="badge badge-live">
          <span class="badge-dot"></span>
          <span>DATABASE IN SYNC</span>
        </div>
        <div class="badge">
          Target: <strong>MAINDOS.EXE</strong> (Watcom DOS/4GW)
        </div>
        <div class="badge">
          Compiler: <strong>Watcom C/C++ 10.6</strong>
        </div>
        <div class="badge">
          Updated: <strong id="genTime">{summary["generated_at"]}</strong>
        </div>
      </div>
    </header>

    <!-- Top KPI Metrics -->
    <div class="metrics-grid">
      <div class="metric-card primary">
        <div class="metric-label">Overall Completion</div>
        <div class="metric-value-row">
          <span class="metric-val">{summary["completion_percent"]}%</span>
          <span class="metric-sub">{summary["completed_functions"]} / {summary["total_functions"]}</span>
        </div>
        <div class="metric-sub">{summary["matching_percent"]}% byte-matching in Watcom C</div>
      </div>

      <div class="metric-card matching">
        <div class="metric-label">Matching Functions</div>
        <div class="metric-value-row">
          <span class="metric-val">{summary["matching_functions"]}</span>
          <span class="metric-sub">({summary["matching_percent"]}%)</span>
        </div>
        <div class="metric-sub">Watcom 10.6 exact byte match</div>
      </div>

      <div class="metric-card decomp">
        <div class="metric-label">Decompiled / Ported</div>
        <div class="metric-value-row">
          <span class="metric-val">{summary["decompiled_functions"]}</span>
          <span class="metric-sub">({round(summary["decompiled_functions"] / summary["total_functions"] * 100, 1)}%)</span>
        </div>
        <div class="metric-sub">Ported to modern C11/SDL2</div>
      </div>

      <div class="metric-card analyzed">
        <div class="metric-label">Analyzed / Mapped</div>
        <div class="metric-value-row">
          <span class="metric-val">{summary["analyzed_functions"]}</span>
          <span class="metric-sub">({round(summary["analyzed_functions"] / summary["total_functions"] * 100, 1)}%)</span>
        </div>
        <div class="metric-sub">Identified &amp; documented in Ghidra</div>
      </div>

      <div class="metric-card">
        <div class="metric-label">Data Structures</div>
        <div class="metric-value-row">
          <span class="metric-val">{summary["total_structs"]}</span>
          <span class="metric-sub">({summary["total_struct_fields"]} fields)</span>
        </div>
        <div class="metric-sub">Memory layouts &amp; alignments</div>
      </div>

      <div class="metric-card">
        <div class="metric-label">Global Variables</div>
        <div class="metric-value-row">
          <span class="metric-val">{summary["total_globals"]}</span>
        </div>
        <div class="metric-sub">Documented data addresses</div>
      </div>

      <div class="metric-card">
        <div class="metric-label">FCTS Deviations</div>
        <div class="metric-value-row">
          <span class="metric-val">{summary["total_deviations"]}</span>
        </div>
        <div class="metric-sub">Authentic bug fixes &amp; toggles</div>
      </div>
    </div>

    <!-- Global Progress Bar -->
    <div class="progress-section">
      <div class="progress-header">
        <div class="progress-title">
          <span>Global Decompilation &amp; Parity Progress</span>
        </div>
        <div class="progress-pct">
          {summary["completed_functions"]} / {summary["total_functions"]} Functions Complete ({summary["completion_percent"]}%)
        </div>
      </div>
      <div class="bar-container">
        <div class="bar-segment bar-matching" style="width: {summary['matching_percent']}%" title="Matching: {summary['matching_functions']} ({summary['matching_percent']}%)"></div>
        <div class="bar-segment bar-decomp" style="width: {round(summary['decompiled_functions'] / summary['total_functions'] * 100, 1)}%" title="Decompiled: {summary['decompiled_functions']} ({round(summary['decompiled_functions'] / summary['total_functions'] * 100, 1)}%)"></div>
        <div class="bar-segment bar-analyzed" style="width: {round(summary['analyzed_functions'] / summary['total_functions'] * 100, 1)}%" title="Analyzed: {summary['analyzed_functions']} ({round(summary['analyzed_functions'] / summary['total_functions'] * 100, 1)}%)"></div>
      </div>
      <div class="progress-legend">
        <div class="legend-item">
          <div class="legend-box" style="background: var(--color-matching);"></div>
          <span><strong>Matching:</strong> {summary["matching_functions"]} ({summary["matching_percent"]}%)</span>
        </div>
        <div class="legend-item">
          <div class="legend-box" style="background: var(--color-decomp);"></div>
          <span><strong>Decompiled:</strong> {summary["decompiled_functions"]} ({round(summary["decompiled_functions"] / summary["total_functions"] * 100, 1)}%)</span>
        </div>
        <div class="legend-item">
          <div class="legend-box" style="background: var(--color-analyzed);"></div>
          <span><strong>Analyzed:</strong> {summary["analyzed_functions"]} ({round(summary["analyzed_functions"] / summary["total_functions"] * 100, 1)}%)</span>
        </div>
        <div class="legend-item">
          <div class="legend-box" style="background: var(--color-unid);"></div>
          <span><strong>Unidentified:</strong> {summary["unidentified_functions"]}</span>
        </div>
      </div>
    </div>

    <!-- Navigation Tabs -->
    <div class="nav-tabs">
      <button class="tab-btn active" onclick="switchTab('functions')">
        📁 Functions <span class="tab-count" id="countFuncs">{summary["total_functions"]}</span>
      </button>
      <button class="tab-btn" onclick="switchTab('modules')">
        📦 Modules <span class="tab-count">{summary["total_modules"]}</span>
      </button>
      <button class="tab-btn" onclick="switchTab('structs')">
        🏛️ Data Structures <span class="tab-count">{summary["total_structs"]}</span>
      </button>
      <button class="tab-btn" onclick="switchTab('globals')">
        🌐 Globals &amp; Memory <span class="tab-count">{summary["total_globals"]}</span>
      </button>
      <button class="tab-btn" onclick="switchTab('deviations')">
        🛡️ FCTS Deviations <span class="tab-count">{summary["total_deviations"]}</span>
      </button>
      <button class="tab-btn" onclick="switchTab('watcom')">
        ⚙️ Watcom Build &amp; Tooling
      </button>
    </div>

    <!-- TAB 1: FUNCTIONS VIEW -->
    <div id="tab-functions" class="tab-view active">
      <div class="controls-bar">
        <div class="search-wrapper">
          <span class="search-icon">🔍</span>
          <input type="text" id="funcSearch" class="search-input" placeholder="Search functions (name, address, notes, module)... [/]" oninput="filterFunctions()">
        </div>

        <div class="filter-group">
          <button class="pill-btn active" onclick="setStatusFilter('all', this)">All</button>
          <button class="pill-btn" onclick="setStatusFilter('matching', this)">Matching ({summary["matching_functions"]})</button>
          <button class="pill-btn" onclick="setStatusFilter('decompiled', this)">Decompiled ({summary["decompiled_functions"]})</button>
          <button class="pill-btn" onclick="setStatusFilter('analyzed', this)">Analyzed ({summary["analyzed_functions"]})</button>

          <select id="moduleFilter" class="select-filter" onchange="filterFunctions()">
            <option value="all">All Modules</option>
            {''.join(f'<option value="{m["name"]}">{m["name"]} ({m["total"]})</option>' for m in data["modules"])}
          </select>

          <select id="fidelityFilter" class="select-filter" onchange="filterFunctions()">
            <option value="all">All Fidelities</option>
            <option value="EXACT">EXACT</option>
            <option value="ADAPTED">ADAPTED</option>
            <option value="EXTENDED">EXTENDED</option>
          </select>

          <button class="pill-btn" onclick="resetFilters()">Reset</button>
        </div>

        <div class="results-count" id="resultsCount">
          Showing {summary["total_functions"]} of {summary["total_functions"]}
        </div>
      </div>

      <div class="table-container">
        <table id="functionsTable">
          <thead>
            <tr>
              <th class="sortable" onclick="sortTable('status')">Status</th>
              <th class="sortable" onclick="sortTable('symbol_name')">Symbol / Label</th>
              <th class="sortable" onclick="sortTable('dos_address')">DOS Addr</th>
              <th class="sortable" onclick="sortTable('win_address')">Win Addr</th>
              <th class="sortable" onclick="sortTable('module_name')">Module</th>
              <th class="sortable" onclick="sortTable('fidelity')">Fidelity</th>
              <th>Port Location</th>
              <th>Purpose &amp; Technical Notes</th>
            </tr>
          </thead>
          <tbody id="functionsTableBody">
            <!-- Populated via JavaScript -->
          </tbody>
        </table>
      </div>
    </div>

    <!-- TAB 2: MODULES VIEW -->
    <div id="tab-modules" class="tab-view">
      <div class="modules-grid" id="modulesGrid">
        {''.join(f'''
        <div class="module-card">
          <div class="module-card-header">
            <div class="module-card-title">{m["name"]}</div>
            <div class="module-card-pct">{m["percent"]}%</div>
          </div>
          <div class="module-desc">{m["description"] or "Module translation unit"}</div>
          <div class="module-bar-wrap">
            <div class="bar-segment bar-matching" style="width: {(m['matching'] / max(m['total'], 1)) * 100}%;"></div>
            <div class="bar-segment bar-decomp" style="width: {(m['decompiled'] / max(m['total'], 1)) * 100}%;"></div>
            <div class="bar-segment bar-analyzed" style="width: {(m['analyzed'] / max(m['total'], 1)) * 100}%;"></div>
          </div>
          <div class="module-stats-row">
            <span>Total: <strong>{m["total"]}</strong></span>
            <span style="color: var(--color-matching);">Match: <strong>{m["matching"]}</strong></span>
            <span style="color: var(--color-decomp);">Decomp: <strong>{m["decompiled"]}</strong></span>
            <span style="color: var(--color-analyzed);">Analyzed: <strong>{m["analyzed"]}</strong></span>
          </div>
          <button class="module-action-btn" onclick="filterByModule('{m["name"]}')">
            View Functions in Table &rarr;
          </button>
        </div>
        ''' for m in data["modules"])}
      </div>
    </div>

    <!-- TAB 3: STRUCTURES VIEW -->
    <div id="tab-structs" class="tab-view">
      <div class="controls-bar" style="margin-bottom: 16px;">
        <div class="search-wrapper">
          <span class="search-icon">🔍</span>
          <input type="text" id="structSearch" class="search-input" placeholder="Search structures &amp; fields..." oninput="filterStructs()">
        </div>
        <button class="pill-btn" onclick="toggleAllStructs(true)">Expand All</button>
        <button class="pill-btn" onclick="toggleAllStructs(false)">Collapse All</button>
      </div>

      <div id="structsList">
        {''.join(f'''
        <div class="struct-card open" data-name="{s["name"].lower()}">
          <div class="struct-header" onclick="this.parentElement.classList.toggle('open')">
            <div class="struct-name">struct {s["name"]}</div>
            <div class="struct-meta">
              <span>Size: <strong>{s["size"]} bytes</strong></span>
              <span>Fields: <strong>{len(s["fields"])}</strong></span>
              <span>Module: <strong>{s["module_name"]}</strong></span>
            </div>
          </div>
          <div class="struct-body">
            <table style="width: 100%;">
              <thead>
                <tr>
                  <th style="width: 80px;">Offset</th>
                  <th style="width: 140px;">Type</th>
                  <th style="width: 180px;">Name</th>
                  <th>Description</th>
                </tr>
              </thead>
              <tbody>
                {''.join(f"""
                <tr>
                  <td class="font-mono" style="color: var(--text-dim);">{f["offset_hex"]}</td>
                  <td class="font-mono" style="color: #38bdf8;">{f["type"]}</td>
                  <td class="font-mono" style="font-weight: 600;">{f["name"]}</td>
                  <td style="color: var(--text-muted);">{f["description"]}</td>
                </tr>
                """ for f in s["fields"])}
              </tbody>
            </table>
          </div>
        </div>
        ''' for s in data["structs"])}
      </div>
    </div>

    <!-- TAB 4: GLOBALS VIEW -->
    <div id="tab-globals" class="tab-view">
      <div class="controls-bar">
        <div class="search-wrapper">
          <span class="search-icon">🔍</span>
          <input type="text" id="globalSearch" class="search-input" placeholder="Search globals (address, type, name, purpose)..." oninput="filterGlobals()">
        </div>
        <div class="results-count" id="globalsCount">
          Showing {len(data["globals"])} of {len(data["globals"])}
        </div>
      </div>

      <div class="table-container">
        <table>
          <thead>
            <tr>
              <th>Win Addr</th>
              <th>DOS Addr</th>
              <th>Type</th>
              <th>Symbol Name</th>
              <th>Description &amp; Context</th>
            </tr>
          </thead>
          <tbody id="globalsTableBody">
            {''.join(f'''
            <tr data-text="{g["win_address"].lower()} {g["dos_address"].lower()} {g["name"].lower()} {g["type"].lower()} {g["description"].lower()}">
              <td><span class="addr" onclick="copyText('{g["win_address"]}')">{g["win_address"]}</span></td>
              <td><span class="addr" onclick="copyText('{g["dos_address"]}')">{g["dos_address"]}</span></td>
              <td class="font-mono" style="color: #38bdf8;">{g["type"]}</td>
              <td class="symbol-name">{g["name"]}</td>
              <td class="purpose-text">{g["description"]}</td>
            </tr>
            ''' for g in data["globals"])}
          </tbody>
        </table>
      </div>
    </div>

    <!-- TAB 5: DEVIATIONS VIEW -->
    <div id="tab-deviations" class="tab-view">
      <div class="info-card">
        <h3>🛡️ Fidelity &amp; Change Tracking System (FCTS)</h3>
        <p style="font-size: 13px; color: var(--text-muted); line-height: 1.5;">
          Racing Dynamite implements strict fidelity tracking: every authentic 1997 engine bug fix, crash prevention,
          and coordinate safety workaround is catalogued with a unique <code>DEV-XXX</code> identifier.
          Every deviation is wrapped in a runtime preservation toggle under <code>Options &gt; Gameplay &gt; Game Fixes</code>,
          allowing players and testers to run in <strong>1997 AUTHENTIC</strong> mode with original behaviors preserved.
        </p>
      </div>

      <div id="deviationsList">
        {''.join(f'''
        <div class="deviation-card">
          <div class="dev-top-row">
            <div style="display: flex; align-items: center; gap: 10px;">
              <span class="dev-id">{d["id"]}</span>
              <span class="dev-cat">{d["category"]}</span>
            </div>
            <span class="addr" onclick="copyText('{d["address"]}')">{d["address"]}</span>
          </div>
          <div class="dev-desc">{d["description"]}</div>
          <div class="dev-toggle">Toggle: <code>{d["toggle"]}</code></div>
        </div>
        ''' for d in data["deviations"])}
      </div>
    </div>

    <!-- TAB 6: WATCOM TOOLCHAIN -->
    <div id="tab-watcom" class="tab-view">
      <div class="info-card">
        <h3>⚙️ Open Watcom 386 C/C++ 10.6 Decompilation Toolchain</h3>
        <p style="font-size: 13px; color: var(--text-muted); margin-bottom: 12px;">
          Authentic source files under <code>decomp/src/</code> are compiled against original headers
          using portable Open Watcom V2 targeting DOS/4GW flat 32-bit protected mode.
        </p>
        <div class="code-block">
Compiler: Open Watcom C32 Optimizing Compiler (wcc386.exe)
Flags:    -3r       Watcom register calling convention (EAX, EDX, EBX, ECX)
          -s        Omit stack check runtime calls (__CHK)
          -omaxet   Maximum speed optimization, loop unrolling, frame pointers
          -eoc      Standard COFF object format for binary diffing
          -zq       Quiet mode
        </div>
      </div>

      <div class="metrics-grid" style="grid-template-columns: repeat(auto-fit, minmax(280px, 1fr));">
        <div class="metric-card">
          <div class="metric-label">Decomp Source Units</div>
          <div class="metric-value-row">
            <span class="metric-val">{len(data["watcom"]["source_files"])}</span>
          </div>
          <div class="metric-sub">
            {' '.join(f'<code>{s["name"]}</code> ({s["size"]} B)' for s in data["watcom"]["source_files"]) or "No files"}
          </div>
        </div>

        <div class="metric-card matching">
          <div class="metric-label">Compiled Watcom Objects</div>
          <div class="metric-value-row">
            <span class="metric-val">{len(data["watcom"]["object_files"])}</span>
          </div>
          <div class="metric-sub">
            {' '.join(f'<code>{o["name"]}</code> ({o["size"]} B)' for o in data["watcom"]["object_files"]) or "None built"}
          </div>
        </div>
      </div>

      <div class="info-card">
        <h3>🚀 Essential RE &amp; Decompilation CLI Commands</h3>
        <div class="code-block">
# 1. Check decompilation status &amp; metrics
uv run python tools/db.py status

# 2. Compile authentic C code with Watcom
uv run python tools/build_decomp.py

# 3. Verify fidelity annotations against source port &amp; documentation
uv run python tools/verify_fidelity.py

# 4. Regenerate this HTML dashboard
uv run python tools/generate_dashboard.py
        </div>
      </div>
    </div>

    <!-- Footer -->
    <footer>
      <div>Racing Dynamite &bull; Ignition (1997) Reverse Engineering Project</div>
      <div>Source Port: C11 / SDL2 &bull; Decompilation: Open Watcom 10.6 &bull; Ghidra MCP Bridge</div>
    </footer>
  </div>

  <div id="toast">Copied to clipboard!</div>

  <!-- Embedded Project Dataset -->
  <script>
    const PROJECT_DATA = {json_data};
    let currentFunctions = [...PROJECT_DATA.functions];
    let currentStatusFilter = 'all';
    let currentSort = {{ column: 'status', asc: true }};

    function switchTab(tabId) {{
      document.querySelectorAll('.tab-btn').forEach(b => b.classList.remove('active'));
      document.querySelectorAll('.tab-view').forEach(v => v.classList.remove('active'));
      
      const targetBtn = Array.from(document.querySelectorAll('.tab-btn')).find(b => b.getAttribute('onclick').includes(tabId));
      if (targetBtn) targetBtn.classList.add('active');
      
      const targetView = document.getElementById('tab-' + tabId);
      if (targetView) targetView.classList.add('active');
    }}

    function renderFunctionsTable() {{
      const tbody = document.getElementById('functionsTableBody');
      if (!tbody) return;

      if (currentFunctions.length === 0) {{
        tbody.innerHTML = '<tr><td colspan="8" style="text-align: center; padding: 32px; color: var(--text-dim);">No functions match current filters</td></tr>';
        document.getElementById('resultsCount').innerText = 'Showing 0 of ' + PROJECT_DATA.functions.length;
        return;
      }}

      let html = '';
      for (const fn of currentFunctions) {{
        const statusClass = 'status-' + fn.status;
        const fidClass = fn.fidelity ? 'fidelity-' + fn.fidelity : '';
        const fidText = fn.fidelity && fn.fidelity !== '-' ? '<span class="fidelity-tag ' + fidClass + '">' + fn.fidelity + '</span>' : '<span style="color: var(--text-dim);">-</span>';
        
        html += `
          <tr>
            <td><span class="status-badge ${{statusClass}}">${{fn.status}}</span></td>
            <td>
              <div class="symbol-name">${{fn.symbol_name}}</div>
              <div class="ghidra-label">${{fn.original_ghidra_name}}</div>
            </td>
            <td><span class="addr" onclick="copyText('${{fn.dos_address}}')">${{fn.dos_address}}</span></td>
            <td><span class="addr" onclick="copyText('${{fn.win_address}}')">${{fn.win_address}}</span></td>
            <td class="font-mono" style="color: #cbd5e1;">${{fn.module_name}}</td>
            <td>${{fidText}}</td>
            <td><span class="port-loc">${{fn.port_location}}</span></td>
            <td>
              <div class="purpose-text">${{fn.purpose || '<span style="color: var(--text-dim);">No description</span>'}}</div>
            </td>
          </tr>
        `;
      }}
      tbody.innerHTML = html;
      document.getElementById('resultsCount').innerText = `Showing ${{currentFunctions.length}} of ${{PROJECT_DATA.functions.length}}`;
    }}

    function filterFunctions() {{
      const query = (document.getElementById('funcSearch').value || '').toLowerCase().trim();
      const modFilter = document.getElementById('moduleFilter').value;
      const fidFilter = document.getElementById('fidelityFilter').value;

      currentFunctions = PROJECT_DATA.functions.filter(fn => {{
        // Status filter
        if (currentStatusFilter !== 'all' && fn.status !== currentStatusFilter) {{
          return false;
        }}
        // Module filter
        if (modFilter !== 'all' && fn.module_name !== modFilter) {{
          return false;
        }}
        // Fidelity filter
        if (fidFilter !== 'all' && fn.fidelity !== fidFilter) {{
          return false;
        }}
        // Search query
        if (query) {{
          const match = 
            fn.symbol_name.toLowerCase().includes(query) ||
            fn.original_ghidra_name.toLowerCase().includes(query) ||
            fn.dos_address.toLowerCase().includes(query) ||
            fn.win_address.toLowerCase().includes(query) ||
            fn.module_name.toLowerCase().includes(query) ||
            fn.port_location.toLowerCase().includes(query) ||
            fn.purpose.toLowerCase().includes(query) ||
            fn.notes.toLowerCase().includes(query);
          if (!match) return false;
        }}
        return true;
      }});

      applySort();
      renderFunctionsTable();
    }}

    function setStatusFilter(status, btn) {{
      currentStatusFilter = status;
      document.querySelectorAll('.filter-group .pill-btn').forEach(b => {{
        if (b.innerText.toLowerCase().includes('all') || 
            b.innerText.toLowerCase().includes('matching') || 
            b.innerText.toLowerCase().includes('decompiled') || 
            b.innerText.toLowerCase().includes('analyzed')) {{
          b.classList.remove('active');
        }}
      }});
      if (btn) btn.classList.add('active');
      filterFunctions();
    }}

    function filterByModule(moduleName) {{
      switchTab('functions');
      document.getElementById('moduleFilter').value = moduleName;
      filterFunctions();
    }}

    function resetFilters() {{
      document.getElementById('funcSearch').value = '';
      document.getElementById('moduleFilter').value = 'all';
      document.getElementById('fidelityFilter').value = 'all';
      currentStatusFilter = 'all';
      document.querySelectorAll('.filter-group .pill-btn').forEach((b, idx) => {{
        b.classList.toggle('active', idx === 0);
      }});
      filterFunctions();
    }}

    function sortTable(column) {{
      if (currentSort.column === column) {{
        currentSort.asc = !currentSort.asc;
      }} else {{
        currentSort.column = column;
        currentSort.asc = true;
      }}
      applySort();
      renderFunctionsTable();
    }}

    function applySort() {{
      const col = currentSort.column;
      const asc = currentSort.asc;

      currentFunctions.sort((a, b) => {{
        let vA = a[col] || '';
        let vB = b[col] || '';

        if (col === 'status') {{
          const order = {{ 'matching': 1, 'decompiled': 2, 'analyzed': 3, 'unidentified': 4 }};
          vA = order[vA] || 99;
          vB = order[vB] || 99;
        }}

        if (vA < vB) return asc ? -1 : 1;
        if (vA > vB) return asc ? 1 : -1;
        return 0;
      }});
    }}

    function filterGlobals() {{
      const q = (document.getElementById('globalSearch').value || '').toLowerCase().trim();
      const rows = document.querySelectorAll('#globalsTableBody tr');
      let visible = 0;
      rows.forEach(r => {{
        const text = r.getAttribute('data-text') || '';
        const match = !q || text.includes(q);
        r.style.display = match ? '' : 'none';
        if (match) visible++;
      }});
      document.getElementById('globalsCount').innerText = `Showing ${{visible}} of ${{rows.length}}`;
    }}

    function filterStructs() {{
      const q = (document.getElementById('structSearch').value || '').toLowerCase().trim();
      const cards = document.querySelectorAll('.struct-card');
      cards.forEach(c => {{
        const name = c.getAttribute('data-name') || '';
        const inner = c.innerText.toLowerCase();
        const match = !q || name.includes(q) || inner.includes(q);
        c.style.display = match ? '' : 'none';
      }});
    }}

    function toggleAllStructs(open) {{
      document.querySelectorAll('.struct-card').forEach(c => {{
        c.classList.toggle('open', open);
      }});
    }}

    function copyText(text) {{
      if (!text || text === '-') return;
      navigator.clipboard.writeText(text).then(() => {{
        showToast('Copied ' + text + ' to clipboard!');
      }}).catch(() => {{}});
    }}

    function showToast(msg) {{
      const t = document.getElementById('toast');
      t.innerText = msg;
      t.classList.add('show');
      setTimeout(() => {{
        t.classList.remove('show');
      }}, 2000);
    }}

    // Keyboard shortcut: '/' focuses search
    window.addEventListener('keydown', e => {{
      if (e.key === '/' && document.activeElement.tagName !== 'INPUT') {{
        e.preventDefault();
        switchTab('functions');
        const s = document.getElementById('funcSearch');
        if (s) {{
          s.focus();
          s.select();
        }}
      }} else if (e.key === 'Escape') {{
        const s = document.getElementById('funcSearch');
        if (s && document.activeElement === s) {{
          s.value = '';
          filterFunctions();
          s.blur();
        }}
      }}
    }});

    // Initial render
    document.addEventListener('DOMContentLoaded', () => {{
      applySort();
      renderFunctionsTable();
    }});
  </script>
</body>
</html>
"""
    return html


def main(argv: List[str] = None) -> int:
    parser = argparse.ArgumentParser(description="Generate Decompilation Progress HTML Dashboard")
    parser.add_argument("--output", "-o", type=str, default="dashboard.html",
                        help="Output HTML file path (default: dashboard.html in project root)")
    args = parser.parse_args(argv)

    print("================================================================================")
    print("      RACING DYNAMITE (IGNITION 1997) DECOMPILATION DASHBOARD GENERATOR        ")
    print("================================================================================")

    data = get_db_data()
    summary = data["summary"]

    print(f"[*] Loaded {summary['total_functions']} functions ({summary['completed_functions']} completed, {summary['completion_percent']}%)")
    print(f"[*] Loaded {summary['total_modules']} modules, {summary['total_structs']} structs, {summary['total_globals']} globals, {summary['total_deviations']} deviations")

    html_content = generate_html(data)

    out_path = Path(args.output)
    if not out_path.is_absolute():
        out_path = ROOT_DIR / out_path

    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w", encoding="utf-8") as f:
        f.write(html_content)

    print(f"[SUCCESS] Dashboard written to: {out_path} ({len(html_content):,} bytes)")

    # Also write a copy to docs/dashboard.html for documentation hub convenience
    docs_copy = ROOT_DIR / "docs" / "dashboard.html"
    try:
        with open(docs_copy, "w", encoding="utf-8") as f:
            f.write(html_content)
        print(f"[SUCCESS] Documentation copy mirrored to: {docs_copy}")
    except Exception as e:
        print(f"[WARN] Failed to write docs mirror: {e}")

    print("================================================================================")
    return 0


if __name__ == "__main__":
    sys.exit(main())
