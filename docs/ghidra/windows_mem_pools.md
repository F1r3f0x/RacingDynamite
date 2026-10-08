# Windows pool initialization and creation

## Independently authenticated provenance

Authentic input: Ignition/Ignition/IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred image base VA 0x00400000. Original binaries/assets are
untouched. No DOS executable was used as evidence.

| Semantic name | VA / RVA | Complete extent | FPO (bytes, local dwords, parameter dwords, bits) | SHA-256 |
| --- | --- | --- | --- | --- |
| Mem_InitPools | 0x0045AD50 / 0x5AD50 | 35 bytes; ends at 0x0045AD73 | (35,0,0,0x103) | 4dde71fdc49b5480369eec8adf5b12a9a74ad3aeb97921330f412e22acc3b627 |
| Mem_CreatePool | 0x0045AD80 / 0x5AD80 | 132 bytes; ends at 0x0045AE04 | (132,0,1,0x209) | 8beca914f2291a89e320f0ad75559b562a336f5a0c020f9553de979bba923743 |

Local PE headers, complete instruction decoding (11/50 instructions), FPO,
return paths, INT3 padding (13/12 bytes), relative calls, HIGHLOW operands,
initializers and imported heap dependencies were independently rechecked.
Live Ghidra disassembly, decompilation and xrefs at localhost:8080 corroborate
both entries. Authentic IGN_WIN.EXE being loaded and active is a user-provided
operating assumption, separate from the local fingerprint check; no automated
Ghidra loaded-program identity is claimed. A bounded read-only second review
independently corroborated both hashes, extents and contracts. Original source
and symbol names are unknown; mem.c and the routine names are semantic names.

Initializer HIGHLOW operands: RVA 0x5AD54 -> VA 0x0063C6A0,
RVA 0x5AD60 -> VA 0x00491F6C. Creator operands: RVA 0x5AD82 and
0x5ADC9 -> VA 0x0063C6A0; RVA 0x5AD93 -> one-past-table VA 0x0063CAA0.
The 256 pool pointers occupy loader-zeroed .data virtual storage, outside its
file-backed raw bytes. DEFAULT is the file-initialized eight-byte string
at VA 0x00491F6C. Neither routine has an initialization flag.

Initializer's only CALL, VA 0x0045AD64, targets creator VA 0x0045AD80.
Creator's only CALL, VA 0x0045ADAE, targets CRT malloc VA 0x00469400.
That 20-byte cdecl wrapper and its file-initialized policy word are authenticated
by the allocator verifier; it forwards size and policy to VA 0x00469420.
HeapAlloc/HeapFree imports do not establish native runtime parity or the
original game compiler. CRT malloc/free remain modeled boundaries.
Actual free through production Mem_Free is VA 0x004693B0.

Live Ghidra reports creator calls at VAs 0x0040FFEB, 0x00417EEF,
0x00418B7C, 0x00420249, 0x004223F1, 0x00446049 and 0x0045AD64;
all seven direct CALL displacements are asserted against authenticated PE bytes.
Initializer has a call at VA 0x0045B175, also authenticated. Caller inspection
finds Script_Player creation checks -1, while five null-name callers pass the
result onwards without a failure check. This is static caller evidence, not
native execution of those callers; no compensating guard was introduced.

## Recovered contracts and production C89

Mem_CreatePool takes one stack pointer dword, ordinary RET, caller cleanup,
EAX signed result, saved ESI/EDI. Mem_InitPools takes no arguments, ordinary
RET, EAX=1, saves EDI. A no-argument RET does not distinguish cdecl/stdcall;
the focused build uses its ordinary x86 C ABI and clear DF.

Creator scans roots 0..255 for the first literal zero. It does not dereference
occupied roots, read root 256, or read the name on a full table. Full returns
-1 without a malloc. Otherwise it requests exactly 320 bytes. A null return
produces -1 with no routine stores or name reads. On success it uses the
cached selected index, attaches the root before initializing it, initializes
its name, zeros all 64 page pointers, and returns the index. There is no free.

For a null name it writes only name[0]=0 before clearing pages. For a nonnull
name it compares source byte i before the i<63 bound, reads it again when
copying, writes the destination byte, increments i and compares the next byte.
It copies up to 63 bytes and writes destination[i]=0. Thus an overlong source
still requires readable source[63]; source[64] is not read. All name bytes
after the terminator retain heap contents. Byte values 0x80..0xFF copy literally.
A name with an earlier embedded NUL stops at that NUL. There is no string-library
dependency, padding of the name tail, allocation retry or rollback.

Initializer unconditionally writes zero to all 256 roots, calls the real
creator with DEFAULT, ignores its result and returns 1. Repeated initialization
orphans previous pool allocations without freeing them. Failure creating the
default leaves root zero null but still reports success. These authentic quirks
are preserved, with no deviation or invented dependency.

Production definitions/prototypes reside in decomp/src/mem.c and
decomp/include/mem.h. Existing 32-bit layout assertions enforce the 320-byte
pool with 64 name bytes and page-pointer offset 64. Volatile table, name and
page accesses preserve observed ordering and duplicate source reads. No
assembly was introduced. The implementation uses the verified Windows types
as layout descriptions after independent instruction recovery, rather than
inferring behavior from the types.

## Real compilation and differential validation

From the repository root in PowerShell:

~~~powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
$env:UV_OFFLINE = 1
uv run python tools/verify_mem_pools.py
uv run python tools/decomp_doctor.py --probe-ghidra
uv run python tools/workflow.py complete --rva 0x5AD50 --rva 0x5AD80 --limitation "CRT malloc/free modeled; instruction equality, original link layout, native heap/game parity, invalid or aliased storage, concurrency and reentry unverified"
uv run python tools/db.py update
uv run python tools/db.py update --check
~~~

The focused build compiles complete production mem.c under strict C89,
Clang/LLD 19.1.1, i686-pc-windows-msvc, -O2, freestanding/no-builtin,
no function inlining/vectorization/SSE. Original compiler identity remains unknown. Nonreturning
malloc/free link fixtures are intercepted explicitly; they cannot manufacture
success. Fresh output is build/decomp/windows/mem_pools_validation.dll, a
focused PE32 validation DLL with no import table, not a playable rebuilt game.
Separate DLL paths preserve other verifiers' recorded artifacts.

75 Mem_InitPools and 745 Mem_CreatePool differential invocations pass. Each
binary independently satisfies an instruction-derived oracle, including EAX,
ESP, nonvolatile registers, clear DF, caller stack, complete root table and
one-megabyte heap arena, unchanged unrelated image bytes, exact ordered
reads/writes/calls, and complete table/heap snapshots at CRT entry. Modeled
CRT calls clobber EAX/ECX/EDX and condition flags.

- Every first-hole index 0..255, success and malloc failure; later holes cannot
  win. Occupied roots may be arbitrary nonzero dwords. Full table with an
  unreadable name makes no name read or malloc. Failed malloc also skips it.
- Every name length 0..80 for bytes 0x41 and 0xFF, null names, truncation,
  terminator and untouched tails, and 64 seeded first-hole/name cases.
- CRT changes to selected and neighboring roots check the cached index;
  successful attachment overwrites a changed selected root, while failure
  preserves the dependency's table mutations.
- Empty, occupied and seeded initial tables; repeated resets, successful
  nested DEFAULT creation and default malloc failure, including full table
  clearing before the CRT entry and unconditionally returning 1.
- Twelve persistent invocations without table/heap resets between calls:
  three initializer calls including default failure, six real Mem_Alloc/
  Mem_Free calls in three allocate/free rounds with reuse and size zero, then
  three additional creator calls including failure and subsequent success.
  These execute actual production allocation/free and authenticated original
  bodies, using the pool created by actual initialization. Only CRT heap
  operations are modeled. Imported allocator/free oracles retain their
  ordered-access and full-state checks; no synthetic pool root is substituted
  between calls.

Compilation and emulation are recorded separately for both new RVAs. The
aggregate verifier runs this suite and refreshes existing memory/font/file
contracts. Its dependency guard checks nine direct calls by owning routine
and exact symbolic target: initializer->creator, creator->malloc, six
allocator->malloc calls and free->CRT free. The first aggregate attempt
caught Clang inlining the creator into the initializer; focused memory builds
now disable function inlining so the initializer executes the separate real
creator body and the guard checks that symbolic dependency.

Raw compiled prefixes differ. Instruction equality, relocation-aware matching,
original linked-global layout, native filesystem/heap/game behavior and a
playable reconstruction remain unverified. Readable nonaliasing source/heap
storage and DF clear are required. General aliasing, invalid/unmapped storage,
concurrency/reentry and native allocator faults are excluded. The existing
Font_Load verifier continues using a pool fixture; this feature adds real pool
creation through allocator/free rather than changing its file/sprite boundary
scope. Full geputget.c and native game rebuilding retain legacy blockers.
