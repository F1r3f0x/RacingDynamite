# Font_Load (VA 0x00456420)

## Contract
- **Routine**: `Font_Load` (font resource loader)
- **Address**: `IGN_WIN.EXE` @ VA `0x00456420` / RVA `0x00056420`
- **Size**: 71 bytes (`0x47`), FPO extent `[0x00456420, 0x00456467)`
- **Padding**: 9 bytes of `0xCC` (`INT3`) ending at `0x00456470` (`Font_Unload`)
- **Routine SHA-256**: `f4c269fd2b0d4fbc491b4f4211b5dd22622c8c20d3d7e3f220eaa94093887441`
- **Signature**: `int Font_Load(const char *filename, int unused)`
- **ABI**: `cdecl` (caller cleanup; callee preserves `esi`, `edi`; frame pointer omitted; FPO record: `(71, 0, 2, 518)`)
- **Parameters**:
  - `filename`: pointer to null-terminated file path string (`const char *`)
  - `unused`: secondary parameter (`int`) forwarded directly to `Font_Parse`
- **Return Value**:
  - `-1` (`0xFFFFFFFF`) on error:
    - If `File_LoadToMemory(filename)` returns `NULL`: sets `g_fileErrorLine` (`0x004BAB34`) to `1000` (`0x3E8`) and returns `-1`.
    - If `Font_Parse(buffer, unused)` fails: frees buffer via `Mem_Free(0, buffer)` and returns `-1`.
  - Allocated font slot ID (`0..29`) upon successful parsing and registration.
- **Dependencies**:
  - `File_LoadToMemory` @ VA `0x004574A0` (RVA `0x000574A0`)
  - `Font_Parse` @ VA `0x00456270` (RVA `0x00056270`)
  - `Mem_Free` @ VA `0x0045B000` (RVA `0x0005B000`)
  - `g_fileErrorLine` @ VA `0x004BAB34` (RVA `0x000BAB34`)

## Binary Evidence

Disassembly and operand relocations from authentic `IGN_WIN.EXE`:
```assembly
0x00456420: 8b 44 24 04          mov      eax, dword ptr [esp + 4]   ; eax = filename (arg 0)
0x00456424: 56                   push     esi                        ; preserve esi
0x00456425: 57                   push     edi                        ; preserve edi
0x00456426: 50                   push     eax                        ; push filename
0x00456427: e8 74 10 00 00       call     0x004574a0                 ; call File_LoadToMemory(filename)
0x0045642c: 83 c4 04             add      esp, 4                     ; caller cleanup (1 arg)
0x0045642f: 8b f8                mov      edi, eax                   ; edi = buffer (loaded file pointer)
0x00456431: 85 ff                test     edi, edi                   ; check if buffer == NULL
0x00456433: 75 12                jne      0x00456447                 ; if non-NULL, proceed to parse
0x00456435: b8 ff ff ff ff       mov      eax, 0xffffffff            ; eax = -1 (error return)
0x0045643a: 5f                   pop      edi                        ; restore edi
0x0045643b: c7 05 34 ab 4b 00 e8 03 00 00 mov dword ptr [0x004bab34], 0x3e8 ; g_fileErrorLine = 1000 (reloc @ 0x0045643d)
0x00456445: 5e                   pop      esi                        ; restore esi
0x00456446: c3                   ret                                 ; return -1
0x00456447: 8b 44 24 10          mov      eax, dword ptr [esp + 0x10]; eax = unused (arg 1, offset past saved regs + ret)
0x0045644b: 50                   push     eax                        ; push unused
0x0045644c: 57                   push     edi                        ; push buffer
0x0045644d: e8 1e fe ff ff       call     0x00456270                 ; call Font_Parse(buffer, unused)
0x00456452: 83 c4 08             add      esp, 8                     ; caller cleanup (2 args)
0x00456455: 8b f0                mov      esi, eax                   ; esi = slot (return value from Font_Parse)
0x00456457: 57                   push     edi                        ; push buffer (arg 1 for Mem_Free)
0x00456458: 6a 00                push     0                          ; push 0 (pool index 0 for Mem_Free)
0x0045645a: e8 a1 4b 00 00       call     0x0045b000                 ; call Mem_Free(0, buffer)
0x0045645f: 83 c4 08             add      esp, 8                     ; caller cleanup (2 args)
0x00456462: 8b c6                mov      eax, esi                   ; eax = slot
0x00456464: 5f                   pop      edi                        ; restore edi
0x00456465: 5e                   pop      esi                        ; restore esi
0x00456466: c3                   ret                                 ; return slot
```

## Relocations & Operands
- Exactly one base relocation exists in the routine:
  - VA `0x0045643D` (RVA `0x0005643D`) -> `0x004BAB34`: `g_fileErrorLine` (set to `1000` / `0x3E8` on file load failure).
- Direct call targets (relative 32-bit displacements):
  - `0x00456427` + 5 + `0x00001074` = `0x004574A0`: `File_LoadToMemory`
  - `0x0045644D` + 5 - `0x000001E2` = `0x00456270`: `Font_Parse`
  - `0x0045645A` + 5 + `0x00004BA1` = `0x0045B000`: `Mem_Free`

## Stack Layout and FPO Data
- FPO record: `(71, 0, 2, 518)`
  - Function length: 71 bytes
  - Local frame dwords: 0
  - Parameters: 2 dwords (8 bytes: `filename` and `unused`)
  - Callee-saved registers: 2 (`esi`, `edi`)
  - Frame pointer omitted (ESP-based addressing)
- At routine entry (`0x00456420`):
  - `[esp + 0x00]`: Return address
  - `[esp + 0x04]`: `filename`
  - `[esp + 0x08]`: `unused`
- After pushing `esi` and `edi` (`0x00456425`):
  - `[esp + 0x00]`: Saved `edi`
  - `[esp + 0x04]`: Saved `esi`
  - `[esp + 0x08]`: Return address
  - `[esp + 0x0C]`: `filename`
  - `[esp + 0x10]`: `unused`

## Callers and Usage
Direct calls to `Font_Load` (`0x00456420`) in `IGN_WIN.EXE`:
1. System font initialization (`0x004181D5` .. `0x00418268`):
   - Loads standard font resources into `g_SystemFonts[0..7]` including `"baltazar\data\red_dark.lft"`, `"baltazar\data\red_lite.lft"`, etc.
   - Example call site:
     ```assembly
     0x004181ce: push 0
     0x004181d0: push 0x498930        ; "baltazar\\data\\red_dark.lft"
     0x004181d5: call 0x456420        ; Font_Load
     0x004181da: add  esp, 8
     0x004181dd: mov  dword ptr [0x552fd0], eax ; g_SystemFonts[0]
     ```
2. HUD font initialization (`0x0041ACFB` .. `0x0041AF33`):
   - Loads in-game HUD font resources for player position, speed, and lap times.

## Independently inspected provenance (2026-10-07)

The target doctor passes the authentic 915,968-byte PE fingerprint, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Local Capstone decoding covers all 71 bytes / 28 instructions. FPO, all three
relative calls, the sole HIGHLOW relocation and nine-byte padding agree.
The error dword at VA `0x004BAB34` is file-backed .data, initially zero.
No DOS executable or original asset was modified or used as evidence.

Ghidra MCP at localhost:8080 responded to bounded disassembly, decompilation
and xref requests. Authentic IGN_WIN.EXE being loaded and active is the
user-provided operating assumption, not an automated program fingerprint check.
Its loader disassembly agrees with the authenticated local bytes. Ghidra's
parser pseudocode omits the second parameter, but the instructions explicitly
push both buffer and the forwarded dword; that instruction evidence controls.
The live xref list contains all 17 calls listed above: eight from the routine
starting at VA 0x00418130 and nine from VA 0x0041AC40. Each direct CALL and its
following ADD ESP,8 are independently asserted against the PE by the verifier.
These callers pass zero as the second argument; tests also forward arbitrary
raw dwords. Caller names are semantic names; original source names are inferred.

The existing production C89 wrapper matches this instruction-derived contract.
No production source change was required. The EXACT annotation describes the
recovered wrapper behavior within its stated dependency contracts; it establishes
neither instruction equality nor complete native resource-management fidelity.
The older unrecorded three-case emulation and 100% fidelity claims in this
file have been replaced by the fresh, reproducible evidence below.

## Resource boundaries and error handling

Native dependency bodies were inspected locally and through Ghidra, without
promoting their tracking stages or claiming their production-C validation:

| Boundary | RVA / size | Routine SHA-256 |
| --- | --- | --- |
| File_LoadToMemory | 0x574A0 / 230 | 2a23d52a4c2588198a7749db30e5de8c2f5ecd150a65f5940dbc2a1a5db87ec3 |
| Mem_Free | 0x5B000 / 127 | 40c455ee2388ee3160f714b07885f4cc9f3fb5401ab86cd2bea5da8749acd471 |

File_LoadToMemory consumes one cdecl stack dword. It calls VA 0x004576B0
and requires result exactly 1 (otherwise error 2030), calls VA 0x00457630
and requires a nonzero size result (otherwise 2040), then Mem_Alloc at
VA 0x0045AE10 with pool zero and that size (allocation failure 2050).
It calls the fopen entry at VA 0x00469390 with the filename and mode string
at VA 0x0047C040. A null stream sets 2000. It calls VA 0x00469D20 with
that stream and a zeroed local structure, then fread at VA 0x00469170 with
(buffer, 1, size, stream). A short read sets 2010 and returns null; a full
read calls fclose at VA 0x00469100 and returns the allocated buffer.
The early null paths contain no compensating free; the short-read path
contains no close. Recovering these transitive bodies and native failure
cleanup is future work. Font_Load overwrites any loader failure error with
1000 and does not call Font_Parse or Mem_Free when the returned pointer is zero.

Mem_Free consumes two cdecl stack dwords. It selects the pool through the
pointer table at VA 0x0063C6A0, scans a 64 by 64 by 16 pointer hierarchy, and
compares record pointers against the supplied buffer. A found record calls
VA 0x004693B0 with the buffer, then zeros the record's second dword and
returns 1; no match returns 0. Font_Load calls it with pool zero and the
identical loaded pointer after parsing, on both parser success and failure.
Its return is ignored. Font_Load does not restore an error changed by a
dependency, so any freeing-side error mutation persists. Modeled error mutation
is a wrapper robustness fixture, not a claim that the native free writes errors.

## Previous loader-only validation (commit e0640b4)

This section records the prior 327-case scope. The follow-up below supersedes
its modeled Mem_Free boundary and current validation count.

PowerShell at the repository root:

```powershell
uv run python tools/verify_font_load.py
uv run python tools/workflow.py complete --rva 0x56420 --limitation "Native file I/O, freeing and sprite creation modeled; instruction equality and native game parity unverified; full geputget.c blocked by legacy dependencies"
uv run python tools/db.py update --check
```

The aggregate verifier also invokes the new contract. Script pins are pefile
2024.8.26, Capstone 5.0.7 and Unicorn 2.1.4; standalone project Python currently
uses Capstone 5.0.9. Clang/LLD 19.1.1 remain provisional behavior-validation
tools, not the identified original compiler. Compilation uses strict C89,
-O2, -ffreestanding, -fno-builtin, -fno-inline, -mno-sse and -mno-sse2 for
an i686-pc-windows-msvc target, with production headers and layout assertions.

The separate focused DLL at `build/decomp/windows/font_load_validation.dll`
contains extracted production Font_Load, Font_Parse, Font_InitSystem,
Font_Shutdown and Font_Unload, plus complete production mem.c. The wrapper is
compiled in a separate translation unit so Clang cannot omit the parser's
unused second argument. Compiling them together did omit that argument;
the forwarding assertion caught this and no production edit was made to hide it.
File_LoadToMemory and Mem_Free link boundaries cannot return without explicit
emulator interception; their fixture link bodies loop forever. The parser
executes fully in 260 cases, including real lazy initialization, allocation
of handle IDs and registration. Sprite creation supplies explicit fixture
handles, including null. A C immutable-byte memcmp models the parser's inline
REPE CMPSB comparison, as in the existing parser verifier.

**327 original-versus-production-C comparisons pass:**

- 35 null-load cases combine five initialization flags with preserved or
  overwritten loader errors. They check error 1000, return -1 and no parse,
  initialization or freeing calls.
- 90 real-parser cases cover all 30 slots with empty, sparse and full glyph
  sets and noncanonical occupied status dwords.
- 50 real-parser cases cover five initialization flags, each magic-byte
  failure, invalid versions, full tables and error precedence.
- 60 real-parser cases cover lazy initialization, disabled/noncanonical
  memory flags, allocation exhaustion and first/last/full registration tables.
- 60 randomized real-parser cases cover signed metrics, raw offset dwords,
  arbitrary/null sprite handles and load/free error mutations.
- 32 isolated wrapper cases model the parser boundary with eight raw return
  dwords and four free-error fixtures. These distinguish preserving the parser
  result from returning the free result, independently of parser success slots.
  They are not real-parser executions.

Each execution checks full font/handle/pending/error state, ordered calls,
exact filename and parser-argument forwarding, parser and free boundary state,
all sprite descriptor fields at bytes 4..31 and sprite-boundary state. An
instruction-derived oracle checks each binary separately before differential
comparison. File/free models clobber EAX, ECX, EDX and condition flags while
preserving the recovered cdecl nonvolatile contract. Freeing poisons the input
buffer; emulated reads after that boundary fail. Unrelated image bytes, buffer
guards and caller stack bytes must remain unchanged. EAX, preserved EBX/ESI/
EDI/EBP, ESP and clear DF are checked. Raw filename dwords exercise forwarding;
the fixture loader never dereferences them and does not validate paths.

Compilation and differential emulation have separate fresh SQLite records.
Raw-prefix bytes differ; compiled extent, instruction equality, original linked
layout and native game parity remain unverified. Full geputget.c/native game
remain blocked by legacy dependencies. No native file I/O, allocator/freeing,
sprite creation/destruction, dependency mutation beyond modeled error writes,
reentry, aliased/invalid/unmapped buffers or negative memory-handle cursors are
covered. Descriptor word zero is uninitialized in the authentic parser and C,
and remains excluded. No playable reconstruction is produced.

## Mem_Free integration follow-up (2026-10-07)

[Mem_Free](windows_mem_free.md), VA 0x0045B000 / RVA 0x5B000, is now reconstructed
in production mem.c and executes fully in the loader differential harness.
The CRT free at VA 0x004693B0 replaces the former Mem_Free model boundary.
File_LoadToMemory and sprite creation remain modeled. The linked CRT/file
fixtures cannot return without emulator interception. Production Font_Load
still requires no body edit; geputget.c now uses the verified header prototype
instead of its former redundant void-return Mem_Free declaration.

**333 differential cases pass**: the prior 327 cases now execute actual
Mem_Free plus six missing-registration cases (three slot positions, valid and
invalid versions). There are 35 null-load, 266 real-parser and 32 isolated
modeled-parser-return cases. Registered loaded buffers reside at record 15 of
block 63/page 63, forcing native hierarchy traversal. Mem_Free calls CRT free
before clearing the size word and preserves the pointer. Unregistered buffers
return zero from Mem_Free with no CRT call or buffer poisoning; Font_Load still
returns its saved parser result. Any CRT fixture error mutation persists only
when the native traversal reaches the dependency. The arbitrary fixture EAX
value from CRT free is ignored by native Mem_Free and then by the wrapper.

The harness checks full pool table/hierarchy state alongside prior font/handle/
error state, ordered Mem_Free and CRT calls, parser/free/CRT boundary snapshots,
post-free buffer read guards, unrelated bytes and cdecl ABI. The Mem_Free
standalone contract adds 519 differential executions with exact hierarchy read
and write ordering. Fresh compilation and emulation records remain separate.
CRT heap freeing, native file I/O and sprite creation are unverified. No native
resource management, instruction equality, original linked layout or game
parity is claimed. Full geputget.c remains blocked by legacy dependencies.
