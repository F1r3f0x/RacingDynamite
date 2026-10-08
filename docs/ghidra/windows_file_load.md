# File_LoadToMemory: independently inspected Windows dependency scope

This is static analysis from authenticated IGN_WIN.EXE instructions and live
bounded Ghidra queries on 2026-10-07. No production C, compilation, original-only
execution, differential validation or native file I/O result is claimed for these
four routines. Font_Load still models File_LoadToMemory. The current completed
reconstruction is its allocator prerequisite [Mem_Alloc](windows_mem_alloc.md).

The local doctor verifies IGN_WIN.EXE at Ignition/Ignition/IGN_WIN.EXE,
915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782.
Preferred image base VA 0x00400000. The active authentic Ghidra program is a
user-provided operating assumption, not an automated identity check. Local PE
FPO extents, complete instruction decoding, initializers, direct calls and
HIGHLOW operands were independently inspected. DOS code and Ghidra inherited
comments are hypotheses only; no original binary/asset was changed.

| Semantic name | VA / RVA | Bytes / FPO | Routine SHA-256 |
| --- | --- | --- | --- |
| File_LoadToMemory | 0x004574A0 / 0x574A0 | 230 / (230,2,1,0x30A) | 2a23d52a4c2588198a7749db30e5de8c2f5ecd150a65f5940dbc2a1a5db87ec3 |
| File_GetStreamSize | 0x004575F0 / 0x575F0 | 60 / (60,0,1,0x307) | 2524b5e97bba2f220082804901c8494382563e7a8207947bd60b8524424f4647 |
| File_GetSize | 0x00457630 / 0x57630 | 47 / (47,0,1,0x206) | 74b545b88eaa717b908b4d812074e28954eb30ea5fc0560f0810492515a5d19f |
| File_CheckReadable | 0x004576B0 / 0x576B0 | 43 / (43,0,1,0) | 3dde60cb96d06429d62e1e2826c17dc7e2c779dc5e2b39d5608b06f47485d7a3 |

All four use one cdecl stack dword and ordinary RET/caller cleanup.
Loader saves EBX/ESI/EDI and returns a raw pointer. Stream-size helper saves
EBX/ESI/EDI and returns the second ftell's signed 32-bit long (including -1).
Filename-size helper saves ESI/EDI and forwards that raw result. Readability
helper uses no saved nonvolatile registers and returns int 1 or 2031.
Names are semantic; the original source filename/module remains unknown.
Future source placement must follow the recovered native memory/file subsystem.

## Loader contract and calls

For const char *filename:

1. Call File_CheckReadable(filename) at VA 0x004574AB. Require **exactly 1**;
   every other result returns null and writes error **2030** at VA 0x004BAB34.
2. Call File_GetSize(filename) at VA 0x004574CC. Only zero fails: return null,
   error **2040**. The comparison is not a signed positivity test; a -1 result
   is forwarded as requested size 0xFFFFFFFF.
3. Call Mem_Alloc(0,size) at VA 0x004574F0. Null returns error **2050**.
4. Call CRT fopen(filename,"rb") at VA 0x00457517, target VA 0x00469390.
   Initialized mode is VA 0x0047C040 (bytes 72 62 00). Null stream returns
   null with error **2000**, retaining the allocation.
5. Zero two local dwords and call CRT fsetpos(stream,&position) at
   VA 0x00457548, target VA 0x00469D20. Ignore its return.
6. Call CRT fread(buffer,1,size,stream) at VA 0x00457555, target VA 0x00469170.
   Require bitwise equality of its 32-bit return and requested size. Any
   mismatch returns null, error **2010**, without fclose or Mem_Free.
7. Call CRT fclose(stream) at VA 0x00457575, target VA 0x00469100, ignore its
   return, and return the cached buffer pointer. Success preserves the error
   word except for any actual dependency mutation.

No failure frees allocated storage. The short-read branch does not close its
stream. There is no size/filename guard, retry or rollback. Preserve these
behaviors in future reconstruction unless a separately authorized deviation
is tracked. Buffer, size and opened stream stay in saved registers across
later dependencies; callback mutation/reentry effects need explicit future tests.
Font_Load overwrites any null result's error with 1000, returns -1 and makes
no parser/free call, as established by its existing differential verifier.

Loader HIGHLOW operand relocations:
RVA 0x574BD,0x574DF,0x57503,0x5752A,0x57566 -> VA 0x004BAB34;
RVA 0x57512 -> mode VA 0x0047C040. Error word is initialized zero in file-backed
.data. Ghidra reports seven callers, including Font_Load call VA 0x00456427
(already authenticated in the font verifier), surface, sound and track loaders.

## Actual filename checks and size behavior

File_CheckReadable opens filename in **text mode "r"**, initialized at
VA 0x004BA794 (bytes 72 00), through fopen call VA 0x004576BA. Null stream
returns **2031**, without writing the global error word or closing. Nonnull
stream is closed at VA 0x004576CD; close result is ignored and return is 1.
Its sole HIGHLOW operand is RVA 0x576B5 -> VA 0x004BA794.

File_GetSize opens filename with the same text-mode string, call VA 0x0045763C,
then calls File_GetStreamSize at VA 0x00457647 and closes at VA 0x00457652.
It does **not** check fopen's result: even a null stream is forwarded to both
the stream-size helper and fclose. It returns the cached helper result,
ignoring fclose. Sole HIGHLOW operand: RVA 0x57637 -> VA 0x004BA794.

File_GetStreamSize performs four CRT calls:

| Call VA | Target VA | Arguments | Result use |
| --- | --- | --- | --- |
| 0x004575F8 | ftell 0x00469DE0 | stream | cached original position |
| 0x00457607 | fseek 0x00469D40 | stream,0,2 | ignored |
| 0x00457610 | ftell 0x00469DE0 | stream | cached return size |
| 0x0045761E | fseek 0x00469D40 | stream,original_position,0 | ignored |

There are no null, ftell-error or seek-result checks. No HIGHLOW operands occur
in this helper. The restore seek still happens after failed tell/seek outcomes.
The readability, filename-size and payload-open steps use separate opens;
changes between them can produce leaks/errors. This is inspected behavior,
not measured native filesystem behavior.

## CRT boundaries and next bounded implementation

fsetpos RVA 0x69D20 is 27 bytes, FPO (27,0,2,0), SHA-256
3acb37e5ee5cfda2dff629e546014b91c8349504f4bb397c1130740042e6a1c5.
Its instructions read the two position dwords and forward stream, low word,
high word and origin zero to VA 0x0046F770. The live decompiler's guessed
parameter list for that downstream routine is unreliable; use the stack
instructions to establish the 64-bit CRT contract before compiling.
CRT names are consistent with live library recognition and call contracts;
original toolchain identity and full CRT implementation remain unresolved.

Prefer reconstructing File_CheckReadable, File_GetStreamSize and File_GetSize
in the recovered native memory/file subsystem, then File_LoadToMemory using real
Mem_Alloc. Pool
creation/initialization also remain analyzed and unreconstructed. Explicit
fopen/fclose/ftell/fseek/fsetpos/fread and CRT malloc/free fixtures are appropriate
only after independently establishing their invocation contracts. Exercise real
file wrappers and allocator through the loader, preserving text/binary modes,
negative size propagation, ignored dependency failures, leakage, ordered calls,
error precedence, exact position words, full state and ABI. Extend Font_Load
with actual loader execution after this scope is supported. Do not substitute
success-returning game allocator or file-size helpers.

Analysis-stage records represent these observations only; no validation result
is manufactured. Native heap/filesystem/game parity, original linked layout,
instruction equality, invalid streams/buffers and concurrency/reentry are
unverified. Full geputget.c/native game remain blocked by legacy dependencies.
