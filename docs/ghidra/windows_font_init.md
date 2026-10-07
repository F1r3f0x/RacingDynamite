# Windows font initializer (2026-10-07)

## Identity and provenance

Semantic name `Font_InitSystem`, inferred module `geputget.c`. Authentic
`Ignition/Ignition/IGN_WIN.EXE`: 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Preferred image base `0x00400000`; VA `0x00456180`, RVA `0x00056180`.
Complete extent `[0x00456180,0x00456206)`: 134 bytes / 33 decoded instructions,
followed by ten INT3 bytes up to `0x00456210`. FPO: zero locals and parameters,
flags `0x0209`. Routine SHA-256:
`32c69eb459a5bc9c8e74254334262750ef7db85caf4f5a70437ebc28738f2c3d`.

The user confirmed authentic IGN_WIN.EXE was open in Ghidra. This is the
user-provided operating assumption, not an automated identity check. HTTP
inspection was retried but the sandbox denied localhost (Node reported EACCES
at 127.0.0.1:8080; PowerShell could not connect). No live Ghidra corroboration
or symbol synchronization is claimed. The local PE fingerprint, FPO, relocations,
initialized data and Capstone disassembly were independently inspected.
No DOS executable was used as evidence.

## Recovered contract

`__cdecl int Font_InitSystem(void)`: EAX result, caller cleanup, preserved EBX,
ESI, EDI, EBP and clear direction flag. Only flag equality to 1 returns 1010
(`0x3F2`) without any calls or state writes. Other flag values initialize:

1. Store 1 to `g_fontSystemInitialized` (VA `0x004BA6C4`) before calling
   `Mem_NextHandleId` (VA `0x0045B1B0`).
2. Store its signed result as a raw dword to `g_fontSubsystemHandle`
   (VA `0x0050E680`); pass the identical dword to `Mem_RegisterHandle`
   (VA `0x0045B360`).
3. Before registration, stage `g_memPendingContext` at VA `0x0063C690` as
   VA `0x004BA6C8` (string `fontExit()` with NUL terminator),
   `g_memPendingCallback` at VA `0x0063C694` as VA `0x00456210`
   (`Font_Shutdown`), and `g_memPendingParameter` at VA `0x0063C698` as zero.
4. Reset six dwords in each of 30 slots, base VA `0x0063F2E0`, stride
   `0x640` / 1600 bytes, end VA `0x0064AE60`. Offsets 0/4/8/12 become zero;
   offsets 16/20 become one. Preserve all bytes 24..1599, including metrics,
   presence tags, padding, glyph handles and widths.
5. Return 1 regardless of allocation or registration failure. No rollback.

This is a partial slot-header reset, not glyph unloading. The existing C89
definition in `decomp/src/geputget.c` already matches the recovered contract,
including Windows `@original` and `@fidelity EXACT`; no source edit was needed.
EXACT describes behavior, not instruction equality. Prototypes in geputget.h
and mem.h match the caller-cleanup ABI. Compile-time checks in the focused unit
enforce FontSlot size 1600 and offsets 24, 30 and 256 independently of DOS packing.

Flag and string are file-backed .data, with initial flag zero. Handle, pending
words and the complete 48,000-byte font array lie in its loader-zeroed virtual
tail. See [handle-ID evidence](windows_handles.md),
[registration](windows_handle_bookkeeping.md), [unload](windows_font_unload.md)
and [shutdown](windows_font_shutdown.md) for downstream contracts.

## Disassembly and relocations

Preferred VAs below summarize all instruction groups. The verifier asserts
routine hash, padding, FPO, complete decoded coverage and both relative CALLs.

| VA | Decoded operation |
| --- | --- |
| 0x00456180 | CMP [0x004BA6C4],1; PUSH ESI; PUSH EDI; JNE 0x00456193 |
| 0x0045618B | MOV EAX,0x3F2; POP EDI; POP ESI; RET at 0x00456192 |
| 0x00456193 | MOV ESI,1; XOR EDI,EDI; MOV [0x004BA6C4],ESI |
| 0x004561A0 | CALL 0x0045B1B0; PUSH EAX; MOV [0x0050E680],EAX |
| 0x004561AB | MOV [0x0063C690],0x004BA6C8 |
| 0x004561B5 | MOV [0x0063C694],0x00456210 |
| 0x004561BF | MOV [0x0063C698],EDI |
| 0x004561C5 | CALL 0x0045B360; ADD ESP,4 |
| 0x004561CD | MOV EAX,0x0063F2E0 |
| 0x004561D2 | MOV [EAX],EDI; ADD EAX,0x640; CMP EAX,0x0064AE60 |
| 0x004561DE | MOV [EAX-0x63C],EDI; MOV [EAX-0x638],EDI; MOV [EAX-0x634],EDI |
| 0x004561F0 | MOV [EAX-0x630],ESI; MOV [EAX-0x62C],ESI |
| 0x004561FC | JB 0x004561D2, using flags preserved from CMP |
| 0x004561FE | MOV EAX,1; POP EDI; POP ESI; RET at 0x00456205 |

All ten HIGHLOW operand relocations (RVA locations, preferred VA values):

| Operand RVA | Value VA |
| --- | --- |
| 0x56182 | 0x004BA6C4 |
| 0x5619C | 0x004BA6C4 |
| 0x561A7 | 0x0050E680 |
| 0x561AD | 0x0063C690 |
| 0x561B1 | 0x004BA6C8 |
| 0x561B7 | 0x0063C694 |
| 0x561BB | 0x00456210 |
| 0x561C1 | 0x0063C698 |
| 0x561CE | 0x0063F2E0 |
| 0x561DA | 0x0064AE60 |

Both CALLs use relative displacements, not HIGHLOW operands.

## Reproduction and evidence

PowerShell at repository root, using already cached script-pinned dependencies:

```powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
$env:UV_OFFLINE = 1
uv run tools/verify_font_cleanup.py
uv run python tools/workflow.py complete --rva 0x56180 --limitation "Full geputget.c and native game blocked by legacy dependencies; instruction equality unclaimed; live Ghidra transport denied by sandbox"
uv run python tools/db.py update --check
```

Pins: pefile 2024.8.26, Capstone 5.0.7, Unicorn 2.1.4. Clang/LLD 19.1.1
compile strict C89 using `--target=i686-pc-windows-msvc -std=c89
-pedantic-errors -Wall -Wextra -Werror -O2 -ffreestanding -fno-builtin
-fno-inline -mno-sse -mno-sse2`. Inlining is disabled to observe calls.
The original compiler remains unknown; this compiler is provisional.

`build/font_cleanup_validation.dll` extracts initializer/shutdown/unload and
the context declaration from production geputget.c, and compiles complete
production mem.c. Exports include the font routines, memory allocation,
registration, release and all required data. A validation-only const pointer
exposes the address of the static context; production linkage is unchanged.
Only the Gfx_SpriteOp release boundary is modeled and records arguments.
Handle allocation, registration, release and Font_Unload bodies execute fully.

**510 initializer differential executions pass:**

- 200 cases reach every registration slot and check first-zero selection.
- 300 combinations cover font flags 0/1/2/0x80000000/0xFFFFFFFF, memory flags
  0/1/2/0xFFFFFFFF, cursors 0/198/199/200/32767, and first/last/full registration.
  Deterministic signed ID patterns include 0, 1, 32767, -1 and -32768.
- Ten initializer calls form five persistent round trips with empty, first,
  last, sparse and all-active fonts: actual `Mem_InitHandles -> Font_InitSystem
  -> populate slots -> Font_Shutdown -> Font_InitSystem`. There are 20 total
  differential routine invocations. Shutdown releases ID 1, clears the flag
  and unloads active slots; reinitialization allocates ID 2 and reuses the slot.
  Population is a fixture; Font_Parse/native allocation is outside this scope.

An independent instruction-derived oracle checks full state and ordered calls.
Each execution checks all font bytes, handle/pending state, return value, actual
call arguments, nonvolatile registers, stack bytes/ESP, DF, and unchanged image
bytes outside lifecycle fields. Hooks observe flag=1 before allocation and all
pending registration fields and the stored handle before registration. Only
verified context/callback pointer fields and addresses receive normalization;
glyph handles and all other values compare literally. Persistent CPUs retain
actual state across invocations. All 123 unload and 71 shutdown cases also pass.

Compilation and emulation are separate recorded evidence for RVA 0x56180.
Raw compiled-prefix bytes differ; no proven compiled extent or relocation-aware
instruction comparison is claimed. Full geputget.c/native game are blocked by
legacy dependencies. Native linked layout, real sprite/resource destruction,
negative-cursor surrounding memory, concurrent mutation and native game parity
remain unverified. Live Ghidra inspection is blocked by sandbox networking.
