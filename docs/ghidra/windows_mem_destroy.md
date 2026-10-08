# Windows pool destruction and shutdown

## Authenticated provenance

Authentic Ignition/Ignition/IGN_WIN.EXE: 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred image base VA 0x00400000. No original asset changed.
No DOS executable was used as evidence.

| Semantic name | VA / RVA | Complete extent | FPO (bytes, local dwords, argument dwords, bits) | SHA-256 |
| --- | --- | --- | --- | --- |
| Mem_DestroyPool | 0x0045B080 / 0x5B080 | 182 bytes, end 0x0045B136 | (182,5,1,0x141D) | 5d94bbebd22cf4806b9abdee9a916457043581b24f1fe69b907c320596cb7e62 |
| Mem_ShutdownPools | 0x0045B140 / 0x5B140 | 41 bytes, end 0x0045B169 | (41,0,0,0x202) | caa0affb689af0a88ea17b0af2829cec7f498ea2613b8ede871badc5afff2444 |

Local doctor fingerprint, PE sections/imports, FPO, complete decoding (58/16
instructions), returns, INT3 padding (10/7 bytes), HIGHLOW operands and direct
calls were reauthenticated independently. Live Ghidra MCP bounded disassembly,
decompilation and xrefs corroborate these entries. Authentic IGN_WIN.EXE being
active is a user-provided operating assumption, separate from the local binary
fingerprint; no automated loaded-program identity is claimed. Names and mem.c
association are semantic, not recovered original symbols/source names.

Destroy relocations: RVA 0x5B08B and 0x5B121 -> VA 0x0063C6A0.
Shutdown relocation: RVA 0x5B149 -> VA 0x0063C6A0. This is the 256-pointer
loader-zeroed .data table independently authenticated by the pool verifier.
The 320-byte pool has 64 name bytes and 64 page pointers; 256-byte pages
contain 64 block pointers; 128-byte blocks contain sixteen pointer/size pairs
of eight bytes. Production layout assertions and volatile fields are retained.

All four destroy calls (VAs 0x0045B0D5, 0x0045B0E4, 0x0045B0FA,
0x0045B112) target CRT free VA 0x004693B0, not 0x004691E0.
The free body/HeapFree import and malloc policy/HeapAlloc import are independently
authenticated by the prerequisite verifiers. Shutdown's only call at
VA 0x0045B150 targets destruction. Ghidra's seven destruction call sites at
VAs 0x00410926, 0x00420853, 0x00420977, 0x00420BE8, 0x00422513,
0x0044616E and 0x0045B150, plus shutdown caller VA 0x0045B1A5, are
asserted as direct CALL instructions against the local PE.

## Recovered behavior

Mem_DestroyPool takes one stack pool-ID dword, ordinary RET/caller cleanup,
returns EAX=1, saves EBX/ESI/EDI/EBP and has five local dwords.
It loads the root without a bounds or null guard. Pages, blocks and records
are traversed in ascending order with fixed 64/64/16 counts, skipping null
pages/blocks. Every record size is read; its pointer is read and forwarded to
free only when size is nonzero. Zero-size records may contain stale/nonzero
pointers and are skipped. Nonzero sizes forward even null or duplicate
pointers. No size or pointer is cleared. A block is freed after its records,
a page after its blocks, and the cached root after all pages. Only after the
root free does the routine clear the selected root-table word and return 1.
It does not clear other roots, names, page/block slots or record words.

Current pool/page/block bases remain cached across CRT calls. Later record
sizes/pointers, block slots and page slots are loaded live. Detaching or replacing
the selected root/current linkage during free does not redirect traversal or
object frees. A later-slot mutation does affect its later visit. The final
selected-table store overwrites a root replacement. CRT returns are ignored.

Mem_ShutdownPools takes no arguments, ordinary RET, saves ESI/EDI, returns
EAX=1. Its 256-root ascending scan reads each root live, skips zero and invokes
the real destruction body with the index for nonzero roots. Destruction reloads
that root. The result is ignored; there is no initialized flag or retry. Later
insertions/removals at modeled free boundaries affect the scan; earlier
insertions are not revisited. Empty/repeated shutdown returns 1 without frees.
A direct repeated destroy of a cleared pool has no protective guard and requires
invalid storage; this fault behavior is outside the supported mapped hierarchy
contract. No invented guards, rollback, bug fixes or deviations were introduced.

Production C89 definitions/prototypes reside in decomp/src/mem.c and
decomp/include/mem.h. Root indexing uses unsigned x86 address arithmetic.
No assembly or invented production dependency was added.

## Real validation and limitations

PowerShell, Python through uv, repository root:

~~~powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
$env:UV_OFFLINE = 1
uv run python tools/verify_mem_destroy.py
uv run python tools/decomp_doctor.py --probe-ghidra
uv run python tools/workflow.py complete --rva 0x5B080 --rva 0x5B140 --limitation "CRT heap modeled; instruction equality, original link layout, native heap/game parity, invalid or aliased storage, concurrency and reentry unverified"
uv run python tools/db.py update
uv run python tools/db.py update --check
~~~

The focused builder compiles complete production mem.c with strict C89,
Clang/LLD 19.1.1, i686-pc-windows-msvc, -O2, freestanding/no-builtin,
no inlining/loop unrolling/vectorization/SSE. This provisional behavior compiler does not
establish the original compiler. Fresh separate output is
build/decomp/windows/mem_destroy_validation.dll, with no imports. Explicit
nonreturning malloc/free fixtures are intercepted by the emulator; they cannot
silently manufacture success. This is not a playable game build.

504 destruction and 265 shutdown differential invocations pass against an
independent instruction-derived oracle. Both binaries independently satisfy
exact ordered hierarchy/size/pointer reads, frees and final root stores;
complete 1 MiB heap/root-table state and CRT-entry snapshots; unchanged unrelated
image bytes; EAX/ESP, preserved registers, caller stack and clear DF. Modeled
CRT calls clobber EAX/ECX/EDX and flags; indexed mutations apply at exact calls.

- Every root index, with unrelated unreadable roots in direct destruction; each
  shutdown index; every page/block/record index, nonzero high-bit sizes and raw
  null/1/0x80000000/0xFFFFFFFF payload pointers.
- Completely dense hierarchy with all 65,536 sizes zero, then a last-record
  nonzero-size null payload; stale pointer words are not read when sizes are zero.
- Empty shutdown, sparse seeded layouts, cached current-root/page/block mutations,
  next-record insertion, later block/page removal and live future-root insertion/
  removal. An insertion behind the shutdown cursor persists without a revisit.
- 37 persistent invocations execute all six real bodies: initialization, creation,
  allocation, individual free, destruction and shutdown. Thirty calls execute
  prerequisite routines. No root/heap reset occurs between these calls.
  Successful allocation/free/reuse, zero-size overwrite leakage, multiple pools,
  repeated shutdown, every new-page allocation failure, and an existing-page
  block/payload failure are checked. The latter retains record zero's nonzero
  allocator-pattern size; destruction forwards its stale pointer as authentic
  instructions do. The heap boundary models this invalid payload free rather
  than claiming native CRT fault/heap behavior.

Compilation and emulation records are separate for both affected RVAs. The
aggregate verifier additionally refreshes existing contracts; its dependency
check requires thirteen calls with exact owners/targets: four destroy->free,
one shutdown->destroy and eight prerequisite sites (free: one; allocator: five;
creator: one; initializer: one). The first
aggregate run caught Clang unrolling the sixteen-record loop into sixteen
payload-free call sites. Focused memory builds now disable loop unrolling,
retaining four static destruction call sites and the strict symbolic guard.
With this flag Clang also merges two allocator malloc sites, yielding five
compiled sites versus six native sites. Ownership and exact targets are still
checked for every generated direct call; behavioral allocation contracts are
fully rerun. Static call counts are compiler-specific, not instruction equality.
Raw compiled prefixes differ; instruction equality and relocation-aware matching,
original linked layout, native heap/game parity and playable rebuilding remain
unverified. Valid roots and readable nonaliasing hierarchy storage, clear DF are
required. Invalid indices/null roots/unmapped storage, arbitrary aliasing,
concurrency and reentry are excluded; bounded dependency mutations are not general
reentry validation. CRT heap remains modeled. Font_Load retains its pool fixture,
and full geputget.c/native game rebuilding retains legacy blockers.
