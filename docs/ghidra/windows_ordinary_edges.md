# Windows ordinary corrected and raw edge wrappers

Authenticated standard IGN_WIN.EXE SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Analysis names Gfx_BuildCorrectedTriangleEdge and Gfx_BuildRawTriangleEdge are
not recovered linker symbols. RVA0x24EC00/VA0x64EC00 and RVA0x24EC40/VA0x64EC40
span59bytes through RET0x64EC3A/0x64EC7A. Extent hashes respectively
`3d7cf4d9c890809aee52eff06fe958abf5987150fee36ea62311c57118177390`
and `31d786c6ae0152819d172f452e335972f98e2cf739cfc0821da8743c0ca791d3`.
Local pinned bytes and live bounded Ghidra disassembly corroborate provenance
under the user-provided active-program assumption. Original source files and
compiler remain unknown; renderer ownership and direct compilation are explicit.

ESI supplies the eight-word edge packet and EBX supplies the two-word output
header followed by V/U/X records. Only EBP is preserved. DEV-007 adapts these
inputs to typed C89 pointers; incidental register outputs are not claimed.
Read Y0 once, SAR8 and store start; read Y1 once, SAR8, subtract captured start
and store count. Only zero skips. The real initializer runs for every nonzero
count, including negatives. Header count is reloaded after initialization, then
initial V/X/U are read in that order. The real corrected/raw emitter writes
at header+8. No clipping, split-state changes or advanced-pointer publication
are added. Negative counts retain their original header and mandatory first record.

Before implementation,50 actual original calls included18 pointer faults and
negative/zero counts and packet/header aliases. `uv run tools/verify_ordinary_edges.py`
passes1,282 whole original/C calls:430 persistent followups,422 actual pointer
faults and568 packet/output alias calls. Ordered non-stack reads/writes, all
arena/shared-state bytes, unrelated image, caller arguments, normal ESP/DF,
ABI-preserved registers and fault prefixes compare. Actual initializer/emitter
bodies execute with no modeled callees. Null and truncated output/header buffers,
zero/negative counts, persistent calls and packet aliases are covered.
The whole linked C89 renderer is import-free; all19 direct calls target verified
production helpers. Existing clipped-wrapper and renderer coverage is retained.

Full113 compilation/differential renewal, fidelity audit and synchronized
exports pass at snapshot `3e7c31572473cde694b32ca5732dc00d57dcfe1f773d3a91585d7adc8aff8055`.
All48 guard tests pass; existing43 scoped native records remain separate.
LF/index and staged gates/hooks are required before commit. No native ordinary
wrapper or complete triangle/game result is claimed. Arbitrary image aliases,
flags/fault stack/unwinding, concurrency and original toolchain remain unresolved.
Next reconstruct the authenticated gradient helper, then complete triangle/sprite
integration and native frames. Playable startup/menus/render/input/audio/races
remain the active completion requirement.
