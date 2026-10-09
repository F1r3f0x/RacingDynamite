# Windows native sprite image operation and coherent control

Authentic standard IGN_WIN.EXE fingerprint remains
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
915,968 bytes, PE32 i386, image base VA 0x00400000. ImageOp VA 0x00461360 /
RVA 0x61360: 329 file-backed .text bytes, FPO (329,0,2,0x140C), SHA-256
81b591def47aa754c8a41dd4255ba978ad5ff82514ca2533537dd607077a2020.
Bounded live Ghidra full instructions/decompilation/xrefs agree with local
Capstone decoding under the user-provided active authentic program assumption.
No automated Ghidra identity check is claimed. No DOS evidence is used.

Two cdecl arguments: source descriptor pointer, signed incoming image ID. EBX/
ESI/EDI/EBP saved and ordinary RET with caller cleanup. Returns zero for delete/
invalid explicit requests, selected image ID for creation/replacement. Control
at VA 0x00520380 is a real 16-byte block: raw next-ID word +0, opaque word +4,
signed capacity +8, pointer-table base +12. Growth receives the block's base.
Primitive startup copies four zero words into it; default initializer then sets
+0 to one. Loader storage is the .data virtual tail. Existing independent C
compiler globals must be replaced by one coherent object before real integration;
source-order declarations alone cannot establish adjacency or this pointer ABI.

Source pointer is captured once and retained through retries and heap calls.
Null source selects deletion: reject ID <=0 before capacity read; reject ID >=
live signed capacity. Load table pointer then the slot. If nonnull, read record
name word +0 and call real byte free, then reread table pointer/slot and call
real packing release with that current record. Do not cache the pre-free record.
Next compare live raw next-ID as signed against selected ID; lower it only when
greater. Reread table/slot, free the current record through byte free (including
null helper call), then reread table again and zero the selected slot. Return zero.
No post-name-free or post-release pointer/ID guards or ownership repair exist.

Nonnull source with incoming ID zero selects automatic allocation. Read raw
next-ID first, then signed capacity. If capacity is not greater than signed
next-ID, call real growth on the coherent block and retry from the source test.
There is no new error, capacity or retry guard. Once admitted, capture next-ID
as selected ID. Compute selected+1 modulo 2^32 and compare it to live signed
capacity. Only when less, capture the table pointer and scan increasing dwords
from that candidate. A zero slot ends scanning immediately; occupied slots
advance address/index and reread signed capacity. Table pointer is captured
for the scan, not reloaded each step. Store the resulting candidate to next-ID.
Negative/noncanonical automatic cursors are not validated; pointer/index math
wraps. An explicitly supplied nonzero ID instead rejects negative or ID >=
capacity and leaves next-ID unchanged. Automatic selection bypasses that explicit
ID validation after assigning its possibly noncanonical cursor.

Load table pointer and capture selected slot address. If null record, call real
Gfx_AllocBytes(64) and publish its pointer into the captured slot after allocation,
even if allocation changes the global table. If nonnull, read its name word,
free name, then reread live table/slot and call real packing release. There is
no record allocation or slot clearing on this replacement branch.

After either branch, reread live table and capture its selected record pointer.
Copy sixteen ascending source dwords to that record with alternating reads/
stores, preserving forward alias effects. Source is not snapshot-copied before
heap calls. Read current destination record name word after copying; call real
Gfx_CopyAllocatedString(name,record). Its pointer destination is record word +0.
Then call real Gfx_AssignSpritePackingStorage(record) and return the captured
selected image ID. Descriptor ownership words, palette/flags and other fields
are copied as raw bits; no invented sanitization or source free is present.

A null allocation/slot, invalid table/record/source/name/pixel pointer and
malformed packing state can fault after earlier frees/publication/copies without
rollback. Heap mutations can retarget subsequent live lookups. Name/record free
calls are separate; they must not be merged into an invented destructor. Exact
and partial aliases follow ordered accesses. Fault-time registers/frames, native
heap exceptions, arbitrary concurrent/reentrant mutation and nontermination are
unverified. Production C89 follows these instruction-derived contracts with all six real dependencies.

## Production and differential validation (2026-10-09)

The complete ImageOp body now executes in the focused PE32 DLL. One volatile
GfxPointerTable object represents the authentic 16-byte control block; header
aliases bind next-ID/capacity/records at offsets 0/8/12. Harness validation_symbols
binds those members from the actual exported object, without linker adjacency
assumptions. The former nonreturning ImageOp fixture is removed. Default descriptor
initialization (RVA 0x611D0) and descriptor copying (RVA 0x612E0) retain their bodies
and use this coherent backing; their existing differential suites remain required.

`uv run python tools/verify_sprite_imageop.py` authenticates the full binary,
parent extent, nine calls, 19 global relocations, dispatch operand, seven direct
callers and related control users. It compiles strict C89 with provisional
Clang/LLD and runs 361 original-versus-C comparisons: 204 persistent follow-ups,
16 fault cases, one bounded observation prefix, eight alias calls, six CRT
boundary mutation calls and 16 packed calls. An independent instruction-derived
parent oracle composes independently authenticated helper contracts. Both x86
images execute real growth, string, storage, release, allocation, pixel and
packing bodies; only CRT malloc/free are modeled. Ordered reads/writes and real
helper entries/arguments/snapshots, full arena/control/packing/unrelated image
bytes, persistent lifecycle and owned strings, normal return/stack/nonvolatile
registers and direction flag are checked. Caller stack bytes are checked on faults;
fault-time register/frame parity is not claimed. Allocation/free mutations test
captured slot publication and later live table/record lookups.

Thirteen earlier original-only probes were preparatory evidence, superseded for
production validation by the permanent differential suite. `verify_matching.py`
runs this contract after all existing suites and checks the production call graph.
The workflow must renew all 50 reconstructed routines, audit provenance, export
SQLite and pass staged gates/hooks before committing. Native game/heap/graphics,
whole primitive/handle/renderer integration, arbitrary reentry/concurrency,
exhaustive aliases, fault frames/atomicity and instruction equality remain
unverified. Cross-page dword stores are excluded because of Unicorn's partial
write behavior. No playable rebuilt executable is certified; the original
compiler and link layout remain unresolved.
