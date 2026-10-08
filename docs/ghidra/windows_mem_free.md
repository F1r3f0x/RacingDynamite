# Mem_Free: Windows allocation-record release

## Verified provenance

Authentic IGN_WIN.EXE at `Ignition/Ignition/IGN_WIN.EXE`: 915,968 bytes,
SHA-256 `7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Preferred base VA 0x00400000. Mem_Free: VA **0x0045B000**, RVA **0x5B000**,
127 bytes, complete FPO extent [0x0045B000,0x0045B07F), followed by one INT3.
Routine hash `40c455ee2388ee3160f714b07885f4cc9f3fb5401ab86cd2bea5da8749acd471`.
51 decoded instructions; FPO (127,1,2,0x141D), one local dword, two stack
arguments, saved EBX/ESI/EDI/EBP, ordinary RET and caller cleanup.
The sole HIGHLOW operand relocation is RVA 0x5B015 -> VA 0x0063C6A0.
The sole relative CALL is VA 0x0045B062 -> CRT free VA 0x004693B0.

Local target doctor, PE headers/FPO, relocations, imports and Capstone
instructions were checked independently. Live Ghidra MCP disassembly,
decompilation and xrefs corroborate the local routine and hierarchy consumers.
Authentic IGN_WIN.EXE being loaded and active is the user-provided operating
assumption, not an automated Ghidra program fingerprint check. No original
binary/asset was modified; no DOS executable was used as evidence.

Direct callers independently authenticated as CALL instructions: VA
0x004127AC (Surface_FreeSRF), 0x0041FCBB (sound initialization), 0x004208BC
(track binary cache) and 0x0045645A (Font_Load). Names are semantic hypotheses;
original module/source names are inferred. The reconstruction belongs in mem.c.

## Allocation hierarchy and ownership evidence

| Object | Verified native layout |
| --- | --- |
| g_memPools | 256 pointers, VA 0x0063C6A0..0x0063CAA0, loader-zeroed .data virtual tail |
| MemPool | 320 bytes: name bytes 0..63, 64 page pointers at offset 64 |
| MemAllocationPage | 256 bytes: 64 record-block pointers |
| Record block | 128 bytes: 16 records, each pointer at offset 0 and size dword at offset 4, stride 8 |

The independently inspected initializer at RVA 0x5AD50 zeros 256 pointer
words and calls the pool creator at RVA 0x5AD80. Pool creation scans first-null
pool pointers, returns -1 if all 256 are occupied, allocates 0x140 bytes,
stores the root, copies at most 63 name bytes with a terminator (or stores
an empty name), and zeros 64 pointers at root+0x40. Allocation at RVA 0x5AE10
corroborates the 64/64/16 counts, allocates pages of 0x100 bytes and blocks of
0x80 bytes, and stores returned allocation pointers plus the requested size
in the two record dwords. Size zero denotes an available record to allocation.
Block pointer words are not uniformly initialized by all allocation paths;
Mem_Free may inspect stale or otherwise indeterminate record pointers.

Pool destruction at RVA 0x5B080 instead tests size != 0 before freeing record
pointers, then frees blocks, pages and the root and clears the selected pool
pointer. Global pool shutdown at RVA 0x5B140 scans 256 roots. Those routines
are inspection evidence only: no source reconstruction or validation promotion
is claimed for them in this feature. Authenticated extents and routine hashes:

| RVA | Bytes | SHA-256 |
| --- | --- | --- |
| 0x5AD50 | 35 | 4dde71fdc49b5480369eec8adf5b12a9a74ad3aeb97921330f412e22acc3b627 |
| 0x5AD80 | 132 | 8beca914f2291a89e320f0ad75559b562a336f5a0c020f9553de979bba923743 |
| 0x5AE10 | 406 | b272bd31d844358c51a80948c668ed37a98108dceff066c13a67328c9a964ed3 |
| 0x5B080 | 182 | 5d94bbebd22cf4806b9abdee9a916457043581b24f1fe69b907c320596cb7e62 |
| 0x5B140 | 41 | caa0affb689af0a88ea17b0af2829cec7f498ea2613b8ede871badc5afff2444 |

## Recovered behavior and C89 reconstruction

Signature `int Mem_Free(int pool_id, void *pointer)`, cdecl. The routine
loads g_memPools[pool_id] without checking pool range or root validity, then
scans pages, blocks and records in ascending index order, skipping null pages
and blocks. It compares each record pointer literally, **without reading size**.
On the first equality it calls CRT free(pointer), then writes zero to the
matched record size, and returns 1. Otherwise it returns 0 with no writes/calls.
The CRT return/EAX value is ignored. The record-block pointer and index remain
cached across the call, so detaching a page at the boundary does not redirect
the final store. The pointer word, hierarchy allocations, pool name/table and
all other records remain unchanged unless the dependency itself mutates them.

Null target pointers can match a zero record pointer and invoke free(NULL).
A stale pointer with size zero still matches, and a repeated call selects it
again. Duplicate pointers select only the first record. These authentic quirks
are preserved without a behavioral deviation or new protective guards.

Production mem.c now contains the routine and pool table. mem.h has verified
named layout types and the cdecl prototype; compile-time assertions enforce
record size/size offset, page size, pool size/pages offset and x86 pointers.
Volatile fields retain the observed read ordering and post-free write.
32-bit unsigned address arithmetic preserves the raw indexed table calculation
without introducing signed multiplication overflow. Supported fixtures use valid
pool IDs 0..255 and readable nonaliasing hierarchy objects; surrounding invalid
pool indices, null roots and unmapped hierarchy objects are outside validation.
The redundant void-return Mem_Free declaration in geputget.c was removed in
favor of the verified header prototype. No font routine body changed.

CRT free is a separate unreconstructed boundary at RVA 0x693B0 (79 bytes),
hash `0b3a6390807c1232be0e96fec6d2e97c96f0c27607a2cd6685e14467d6f0c1af`.
Its inspected body skips null, calls internal heap helpers for some pointers,
and otherwise calls the HeapFree import at VA 0x0064C318, with heap handle
from VA 0x0064AF64. It is modeled in validation, not replaced in production.
Focused DLL link fixtures loop forever if interception is missing; no invented
success-returning CRT implementation can silently pass.

## Compilation and differential validation

PowerShell, Python through uv, from repository root:

```powershell
uv run python tools/verify_mem_free.py
uv run python tools/verify_font_load.py
uv run python tools/workflow.py complete --rva 0x5b000 --rva 0x56420 --limitation "CRT heap freeing, file loading and sprite creation modeled; invalid pools/hierarchies, instruction equality and native game parity unverified; full geputget.c blocked by legacy dependencies"
uv run python tools/db.py update --check
```

Both verifiers are integrated into verify_matching.py. The mem verifier builds
complete production mem.c using Clang/LLD 19.1.1, i686-pc-windows-msvc, strict
C89, -O2, -ffreestanding, -fno-builtin, scalar/no-SSE flags. The original
compiler remains unknown; this is provisional behavior validation. Pins are
pefile 2024.8.26, Capstone 5.0.7 and Unicorn 2.1.4; standalone project Python
currently uses Capstone 5.0.9. Memory freeing gets a separate focused DLL
(build/decomp/windows/mem_free_validation.dll), avoiding overwriting an earlier
memory verifier's recorded artifact. Existing memory/font builders link an
explicit nonreturning CRT boundary as required by complete mem.c. The aggregate
provenance guard permits exactly one direct helper call, owned by Mem_Free and
targeting that free boundary; all other direct helper dependencies are rejected.
Its former whole-DLL zero-direct-call assertion was caught on the first aggregate
run and updated to account for the newly reconstructed native dependency.

**519 differential executions pass:**

- 256 cases select every pool ID using a distinct selected root and empty
  roots for other IDs; matched sizes include arbitrary raw dwords.
- 128 cases cover every page and block index, with size-zero matches.
- 16 cases cover every record index at the last page/block.
- 32 invocations cover null/1/0x80000000/0xFFFFFFFF pointers with four size
  values, duplicate pointers and repeated release, including size-zero records.
- Three cases cover empty and completely dense missing-pointer scans and a
  last-record match in the full 65,536-record hierarchy.
- 80 cases cover randomized sparse pages/blocks, match positions and pools.
- Four CRT-boundary mutation cases alter the pointer/size words and detach
  the current page, then confirm the cached record's size is still cleared.

An instruction-derived oracle checks each binary independently. Ordered reads
include only the selected table word, page/block words and record pointers;
size words must never be read. The exact event order is CRT free then one size
store, or no events for no match. At CRT entry the complete arena must still
match its pre-call image; the model clobbers EAX/ECX/EDX/condition flags and
applies explicit mutations. Complete hierarchy arena, image bytes, caller stack,
nonvolatile registers, ESP, return value and clear DF are checked. Fixtures
model allocated hierarchy storage, not native pool creation or native heaps.

The loader verifier now executes Mem_Free in both binaries in **333 cases**:
35 null loads, 266 real-parser cases and 32 isolated modeled-parser-return cases.
A registered buffer occupies the last record of the last block/page and reaches
CRT free; six additional cases leave the buffer unregistered and check native
return-zero traversal without a CRT call. It verifies post-call size clearing
and pointer retention, buffer poisoning/read guards at CRT free and complete
tracked state. See [loader evidence](windows_font_load.md).

Compilation and emulation are separately recorded for RVAs 0x5B000 and 0x56420.
Raw-prefix bytes differ; instruction equality, original linked layout, native
heap behavior and native game parity remain unverified. Invalid/aliased
hierarchies, invalid pool IDs, concurrency and reentry are excluded. Bounded
boundary mutations are not general reentry validation. Full geputget.c and
native game remain blocked by legacy dependencies; no playable reconstruction
is produced.

## Mem_Alloc prerequisite follow-up (2026-10-07)

[Mem_Alloc](windows_mem_alloc.md), VA 0x0045AE10 / RVA 0x5AE10, now executes
through 800 differential allocator invocations and three allocation/free/reuse
round trips that execute real Mem_Free bodies. CRT malloc/free remain explicit
modeled heap boundaries. This follow-up leaves Mem_Free's body unchanged and
refreshes its 519-case verifier plus the existing 333 Font_Load cases. The
complete memory DLL now has seven direct helper calls: six owned by Mem_Alloc
targeting malloc and one owned by Mem_Free targeting free. The provenance
guard verifies both ownership and target, superseding the older count of one.
Memory/font builders link nonreturning malloc as well as free fixtures because
they compile complete mem.c. Font_Load still models File_LoadToMemory; it does
not execute Mem_Alloc until that loader dependency is reconstructed.

Compilation and differential emulation are separately refreshed. Instruction
equality, native heap/game parity, invalid/aliased hierarchies, concurrency and
general reentry remain unverified. Allocator validation additionally requires
readable trailing lookahead words; that is not a new Mem_Free requirement.
