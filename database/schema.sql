-- Active tracking for the fingerprinted Windows binary. Addresses are numeric RVAs.
PRAGMA foreign_keys = ON;
CREATE TABLE metadata (key TEXT PRIMARY KEY, value TEXT NOT NULL);
CREATE TABLE modules (id INTEGER PRIMARY KEY, name TEXT UNIQUE NOT NULL, source_path TEXT);
CREATE TABLE functions (
    id INTEGER PRIMARY KEY,
    rva INTEGER UNIQUE NOT NULL CHECK(rva >= 0),
    byte_size INTEGER CHECK(byte_size IS NULL OR byte_size > 0),
    symbol_name TEXT NOT NULL,
    module_id INTEGER REFERENCES modules(id),
    extent_origin TEXT NOT NULL,
    extent_confidence TEXT NOT NULL DEFAULT 'medium',
    classification TEXT NOT NULL DEFAULT 'unknown' CHECK(classification IN ('unknown','game','crt','thunk')),
    analysis_stage TEXT NOT NULL DEFAULT 'unidentified' CHECK(analysis_stage IN ('unidentified','named','analyzed','reconstructed')),
    source_path TEXT,
    routine_sha256 TEXT,
    abi TEXT,
    fidelity TEXT NOT NULL DEFAULT 'unknown' CHECK(fidelity IN ('unknown','EXACT','ADAPTED','EXTENDED','INFRASTRUCTURE')),
    evidence_path TEXT,
    notes TEXT NOT NULL DEFAULT '',
    imported_evidence_json TEXT
);
CREATE TABLE verification_runs (
    id INTEGER PRIMARY KEY,
    function_id INTEGER NOT NULL REFERENCES functions(id),
    kind TEXT NOT NULL CHECK(kind IN ('compilation','raw_bytes','instructions','linked','emulation','native')),
    outcome TEXT NOT NULL CHECK(outcome IN ('pass','fail','different','unverified')),
    case_count INTEGER NOT NULL DEFAULT 0 CHECK(case_count >= 0),
    recorded_at TEXT NOT NULL,
    target_sha256 TEXT NOT NULL,
    routine_sha256 TEXT NOT NULL,
    input_paths_json TEXT NOT NULL,
    input_sha256 TEXT NOT NULL,
    artifact_path TEXT,
    artifact_sha256 TEXT,
    command TEXT NOT NULL,
    details_json TEXT NOT NULL
);
CREATE INDEX verification_function ON verification_runs(function_id,kind,id);
CREATE TABLE milestones (
    key TEXT PRIMARY KEY,
    title TEXT NOT NULL,
    state TEXT NOT NULL DEFAULT 'unverified' CHECK(state IN ('unverified','blocked','observed','validated')),
    evidence_path TEXT,
    notes TEXT NOT NULL DEFAULT ''
);
