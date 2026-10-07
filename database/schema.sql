-- ==============================================================================
-- Racing Dynamite / Ignition (1997) Decompilation Tracking Database Schema
-- Target: MAINDOS.EXE (DOS/4GW 32-bit LE)
-- ==============================================================================

PRAGMA foreign_keys = ON;

-- Project configuration and metadata
CREATE TABLE IF NOT EXISTS metadata (
    key TEXT PRIMARY KEY,
    value TEXT NOT NULL
);

-- Source code translation units / modules
CREATE TABLE IF NOT EXISTS modules (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,             -- e.g. 'getsurf.c', 'lisa3d.c'
    original_path TEXT,                    -- e.g. 'd:\projects\ignition\getsurf\getsurf.c'
    decomp_path TEXT,                      -- e.g. 'decomp/getsurf.c'
    description TEXT,                      -- Module purpose / subsystem
    notes TEXT
);

-- Functions in the executable
CREATE TABLE IF NOT EXISTS functions (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    dos_address TEXT UNIQUE,               -- Address in MAINDOS.EXE (e.g. '0x00012340')
    win_address TEXT UNIQUE,               -- Legacy cross-ref address (e.g. '0x00412fc0')
    symbol_name TEXT NOT NULL,             -- Reconstructed / authentic C function name
    original_ghidra_name TEXT,             -- Ghidra default label (e.g. 'FUN_00412fc0')
    module_id INTEGER REFERENCES modules(id) ON DELETE SET NULL,
    status TEXT NOT NULL DEFAULT 'unidentified' 
        CHECK (status IN ('unidentified', 'analyzed', 'decompiled', 'matching')),
    calling_convention TEXT DEFAULT 'watcom_reg'
        CHECK (calling_convention IN ('watcom_reg', 'cdecl', 'stdcall', 'fastcall')),
    return_type TEXT DEFAULT 'void',
    parameters TEXT,                       -- Formatted parameter list e.g. '(int x, int z)'
    byte_size INTEGER,                     -- Byte size in binary
    line_count INTEGER,                    -- Decompiled C line count
    fidelity TEXT DEFAULT 'EXACT'
        CHECK (fidelity IN ('EXACT', 'ADAPTED', 'EXTENDED', 'INFRASTRUCTURE', '-')),
    port_location TEXT,                    -- Location in src/ if ported
    purpose TEXT,                          -- High-level description of functionality
    notes TEXT,                            -- Technical notes, registers, formulas
    assembly_hash TEXT                     -- Hash of original disassembly for matching
);

CREATE INDEX IF NOT EXISTS idx_functions_dos_addr ON functions(dos_address);
CREATE INDEX IF NOT EXISTS idx_functions_module ON functions(module_id);
CREATE INDEX IF NOT EXISTS idx_functions_status ON functions(status);

-- Global variables and static buffers
CREATE TABLE IF NOT EXISTS globals (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    dos_address TEXT,                      -- Address in MAINDOS.EXE
    win_address TEXT UNIQUE,               -- Legacy cross-ref address
    name TEXT NOT NULL,                    -- Variable name (e.g. 'g_pActiveSRF')
    module_id INTEGER REFERENCES modules(id) ON DELETE SET NULL,
    type TEXT NOT NULL,                    -- Data type (e.g. 'uint8_t*', 'int32_t')
    size INTEGER,                          -- Size in bytes
    description TEXT,                      -- Description and context
    initial_value TEXT
);

CREATE INDEX IF NOT EXISTS idx_globals_dos_addr ON globals(dos_address);
CREATE INDEX IF NOT EXISTS idx_globals_name ON globals(name);

-- Structures and type definitions
CREATE TABLE IF NOT EXISTS structs (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT UNIQUE NOT NULL,             -- Struct name (e.g. 'SrfHeader')
    size INTEGER,                          -- Total size in bytes
    module_id INTEGER REFERENCES modules(id) ON DELETE SET NULL,
    description TEXT,
    notes TEXT
);

-- Fields within structures
CREATE TABLE IF NOT EXISTS struct_fields (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    struct_id INTEGER NOT NULL REFERENCES structs(id) ON DELETE CASCADE,
    offset INTEGER NOT NULL,               -- Byte offset within struct
    type TEXT NOT NULL,                    -- Field C type
    name TEXT NOT NULL,                    -- Field name
    size INTEGER,                          -- Field size in bytes
    description TEXT,
    UNIQUE(struct_id, offset)
);

CREATE INDEX IF NOT EXISTS idx_struct_fields_struct ON struct_fields(struct_id);

-- Deviations and bug tracking
CREATE TABLE IF NOT EXISTS deviations (
    id TEXT PRIMARY KEY,                   -- e.g. 'DEV-001'
    category TEXT NOT NULL,                -- e.g. 'FIX_CAT_NOCLIP'
    title TEXT NOT NULL,
    description TEXT,
    dos_address TEXT,
    win_address TEXT,
    toggle_key TEXT                        -- Corresponding flag in GameFixOptions
);

-- Source evidence is deliberately separate from historical reconstruction status.
-- A lexical candidate is not a confirmed defect or proof of authentic behavior.
CREATE TABLE IF NOT EXISTS implementation_audits (
    symbol_name TEXT NOT NULL,
    source_path TEXT NOT NULL,
    source_line INTEGER NOT NULL,
    implementation_state TEXT NOT NULL,
    runtime_verification TEXT NOT NULL,
    evidence TEXT NOT NULL,
    PRIMARY KEY(symbol_name, source_path)
);
