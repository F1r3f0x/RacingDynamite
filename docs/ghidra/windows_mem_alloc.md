# Mem_Alloc: Windows pool allocation

## Provenance and bounded scope

Authentic IGN_WIN.EXE: 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782.
Preferred image base VA 0x00400000. Mem_Alloc is VA **0x0045AE10** /
RVA **0x5AE10**, 406 bytes, complete FPO extent [0x0045AE10,0x0045AFA6),
followed by ten INT3 bytes. Routine SHA-256:
b272bd31d844358c51a80948c668ed37a98108dceff066c13a67328c9a964ed3.
FPO (406,2,2,0x141B), two local and two argument dwords, saved EBX/ESI/EDI/EBP,
ordinary RET/caller cleanup. Signature: void *Mem_Alloc(int pool_id, unsigned int size).
The sole HIGHLOW operand relocation is RVA 0x5AE1B -> VA 0x0063C6A0.
Six native calls, at VAs 0x0045AE94, 0x0045AEBF, 0x0045AEF7,
0x0045AF1F, 0x0045AF4F and 0x0045AF84, all target CRT malloc VA 0x00469400.

The doctor, local authenticated PE headers/FPO, imports, initializers,
relocations and complete Capstone instructions were independently checked.
Live Ghidra bounded disassembly, decompilation and xrefs corroborated the
contract. Authentic IGN_WIN.EXE being active is the user-provided operating
assumption; no automated loaded-program fingerprint claim is made. Existing
Ghidra labels/comments and legacy DOS implementation/status were not accepted
as proof. No original binary/asset was modified and no DOS binary was used.

Locally authenticated direct callers include File_LoadToMemory at VA 0x004574F0,
the allocation wrapper call at VA 0x0045AFC0, VA 0x004100C9 and VA 0x0041ACA3.
Ghidra reports 26 call references. Original source/symbol names are unknown;
mem.c and semantic names describe the recovered allocation subsystem.

## Hierarchy and instruction contract

The independently rechecked hierarchy agrees with [Mem_Free](windows_mem_free.md):
256 pool pointers at VA 0x0063C6A0 in loader-zeroed .data; each 320-byte pool
contains 64 name bytes then 64 page pointers; each 256-byte page contains 64
block pointers; each 128-byte block contains sixteen 8-byte pointer/size records.
The existing compile-time layout checks remain in production mem.c.

The allocator takes the selected root without a range or null-root guard.
It traverses pages, blocks and records in ascending order, selecting the first
record whose **size** is zero regardless of its pointer. It tests each page/block
pointer before its index bound, then reloads the chosen pointer. The record scan
tests size before its bound as well. Consequently exhaustion reads pool+320,
page+256 and block+132 (record index 16's size). These are real reads beyond
the declared arrays, not extra usable capacity. Their values do not change the
exhausted result. Tests supply mapped readable trailing words; native allocator
fault behavior for unreadable trailing storage remains unverified.

- Existing free record: malloc(size); failure returns null without native writes;
  success stores pointer then size and returns the pointer.
- First missing block in an existing page: malloc(128); attach the block before
  initialization; clear only sizes 1..15; malloc(size); success writes record zero's
  pointer then size. Payload failure retains the attached block and leaves record
  zero's size at its original malloc-filled value. All pointer words remain untouched
  until explicitly populated.
- First missing page: malloc(256); attach it then zero all 64 block pointers;
  malloc(128); attach block zero then clear all sixteen sizes; malloc(size);
  success writes record zero's pointer then size. Each failure retains previously
  attached hierarchy objects. No failure frees them.
- Full hierarchy: return null with no malloc or writes, after lookahead reads.

A missing earlier page/block takes priority over reusable records later in the
hierarchy. A nonnull malloc result for size zero is recorded with size zero,
so the next allocation reuses that record and overwrites its pointer. This
can orphan the earlier allocation. These native behaviors are preserved;
no deviation or new protective guard is introduced. Pool/page/block destinations
and requested size remain cached across CRT calls; bounded mutations verify
that detaching hierarchy pointers does not redirect stores.

The C89 implementation uses named types, offsetof and sizeof. Volatile fields
preserve observed read/write ordering. Exhausted lookahead addresses use x86
unsigned integer address calculation and pointer conversion, avoiding C array
indexing beyond the declared aggregate. This is a platform-specific flat-address
contract, not portable ISO C or instruction equality.

CRT malloc RVA 0x69400 is a 20-byte cdecl wrapper, FPO (20,0,1,0), SHA-256
7d8692fed5df9efac5f7c0937ec31305b20b064cc084472afa53c39c7333fcfc.
It forwards size and the policy word at VA 0x004BB400 (file initialized zero)
to VA 0x00469420. HeapAlloc is imported at VA 0x0064C314. Ghidra's library
identification is a compiler clue, not proof of the original game compiler.
Native heap/new-handler behavior is unreconstructed. Tests intercept the CRT
entry with explicit request/return fixtures, including null and arbitrary raw
payload pointers. Hierarchy allocations name mapped fixture storage. Linked
malloc/free fixtures loop forever if interception is absent.

## Validation and reproduction

PowerShell with Python through uv, from repository root:

~~~powershell
uv run python tools/verify_mem_alloc.py
uv run python tools/workflow.py complete --rva 0x5ae10 --rva 0x5b000 --rva 0x5b410 --limitation "CRT heap modeled; readable trailing words required; invalid/aliased hierarchies, concurrency, general reentry, instruction equality and native game parity unverified"
uv run python tools/db.py update --check
~~~

The verifier compiles complete production mem.c plus explicit nonreturning CRT
boundaries into build/decomp/windows/mem_alloc_validation.dll, separate from
other recorded DLLs. Clang/LLD 19.1.1 is provisional: i686-pc-windows-msvc,
strict C89, -O2, freestanding/no-builtin, no vectorization/SSE. This is a focused
DLL, not a playable game. The aggregate resolver pins pefile 2024.8.26,
Capstone 5.0.7 and Unicorn 2.1.4; standalone project Capstone is 5.0.9.

The differential suite checks 800 original-instruction versus compiled-C
invocations plus three real Mem_Free round trips:

- All 256 pool IDs and all sixteen record indices; unsigned size/return
  boundaries, size-zero reuse and malloc failure.
- Every page and block insertion/existing-record index, complete preceding scans,
  dense exhaustion and the last of 65,536 records.
- All six native malloc failure sites, retained attachments and subsequent retries;
  the existing-page uninitialized size-zero case and first-hole priority.
- Zero and nonzero lookahead words at every exhausted hierarchy level.
- 64 seeded cases with varied nonzero occupied sizes and pool/size/return words.
- CRT mutations that detach cached hierarchy destinations or change target records.
- Allocation, real free with size clearing/pointer retention, then real record reuse
  for each of the three allocation branches. CRT free remains modeled.

Each binary is checked independently against an instruction-derived oracle:
exact ordered pool/hierarchy reads, writes and malloc calls; complete arena
and image state; full pre-call state at each CRT entry; EAX/ESP, caller stack,
saved registers and clear DF. Models clobber EAX/ECX/EDX and condition flags.
Compilation and emulation are recorded separately. Raw-prefix bytes differ;
instruction equality, original linked layout, native heap behavior and native
game parity remain unverified. Invalid roots/indices/unmapped trailing storage,
aliasing, concurrency and general callback reentry are excluded. Full geputget.c
and the native game remain blocked by legacy dependencies.

The aggregate verifier permits exactly seven direct calls in the focused memory
DLL: six owned by Mem_Alloc targeting malloc and one owned by Mem_Free targeting
free. Font cleanup/parser/load builders also link the explicit malloc boundary
because they compile complete mem.c; their routine bodies are unchanged and
fully revalidated. Existing mem/free/font results are regenerated after these
input changes. The conservative source scanner also treats Mem_Free as affected because its
preceding declarations changed. Its body is unchanged and its real verifier
executes; Mem_ReleaseHandleId is additionally included in the completion scope
as an unchanged regression contract.

## Independently rechecked pool prerequisites

Live Ghidra and authenticated local bytes corroborate these entries during
this feature. They are recorded as analyzed only, with **no compilation,
original execution or differential validation** and no reconstructed C bodies:

| Routine | VA / RVA | Bytes / FPO | Routine SHA-256 |
| --- | --- | --- | --- |
| Mem_InitPools | 0x0045AD50 / 0x5AD50 | 35 / (35,0,0,0x103) | 4dde71fdc49b5480369eec8adf5b12a9a74ad3aeb97921330f412e22acc3b627 |
| Mem_CreatePool | 0x0045AD80 / 0x5AD80 | 132 / (132,0,1,0x209) | 8beca914f2291a89e320f0ad75559b562a336f5a0c020f9553de979bba923743 |

Mem_InitPools clears all 256 roots then calls Mem_CreatePool with initialized
"DEFAULT" at VA 0x00491F6C, ignores its result and returns 1. It does not free
existing roots. Mem_CreatePool scans for the first null root, returns -1 on full
table or malloc(320) failure, attaches before name initialization, copies up to
63 name bytes and writes a terminator (or just an empty name for null input),
then clears all page pointers and returns the pool index. Unwritten name bytes
retain heap contents. It reads the next source byte before the 63-byte bound.
Both use ordinary caller cleanup; initializer saves EDI, creator saves ESI/EDI.
Names/module association are semantic hypotheses. Mem_Alloc validation starts
with independently defined hierarchy fixtures; it does not execute these bodies.

The next loader scope is recorded separately in
[File_LoadToMemory dependency analysis](windows_file_load.md).
