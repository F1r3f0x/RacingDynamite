# Windows Lisa version banner

## Provenance, strings and ABI

Input: authentic Ignition/Ignition/IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred image base VA 0x00400000. Originals remain untouched;
no DOS executable supplied evidence. Lisa_PrintVersion is a semantic name;
original source names are unknown. Production placement follows native lisa3d.c.

VA 0x0045B4F0 / RVA 0x5B4F0 has complete FPO extent 34 bytes, tuple
(34,0 local dwords,0 argument dwords,0 bits), ending at VA 0x0045B512.
Fourteen INT3 bytes precede the next entry. Complete body SHA-256:
719ae03224b3f4a4ab08fa52985cd65454730bd8de870f8afe632d9216951802.
Nine instructions push version then format, call VA 0x0046A500, clean eight
bytes, push copyright format, call that same target, clean four bytes, zero
EAX and RET. HIGHLOW relocations at RVAs 0x5B4F1/0x5B4F6/0x5B503 target
VAs 0x004BAB20/0x004BAB5C/0x004BAB3C respectively.

All three strings are NUL-terminated and independently recovered from the
file-backed .data section, including exact punctuation and newline bytes:

| VA / RVA | Bytes before NUL (C escaping) | Role |
| --- | --- | --- |
| 0x004BAB20 / 0xBAB20 | `Compilation 0.91.0` | First call's sole vararg |
| 0x004BAB5C / 0xBAB5C | `\nLisa 2 Development System, %s\n` | First format |
| 0x004BAB3C / 0xBAB3C | `Copyright (c) UDS, 1995-1996\n\n` | Second format, no vararg |

Both printf results are ignored; even negative/error returns do not skip the
second call or alter the final zero. No globals are read or written by the banner
itself. Zero arguments, balanced caller stack, preserved nonvolatile registers,
and EAX=0 establish the int(void) production interface. No-argument RET alone
cannot distinguish cdecl/stdcall, so ordinary x86 C ABI is used. The variadic
printf boundary is caller-cleanup cdecl, independently established by pushes and
ADD ESP,8/4 rather than an inherited DOS ABI.

The printf target is a statically linked CRT routine, not a PE import. Its FPO
extent is 61 bytes, tuple (61,0,1,0x202), SHA-256
ed5882e648efe6314e582da142973e428c0c42ba90e311b3da568581151fc447.
It loads the format from the stack, forms the following varargs pointer, invokes
an output helper with stdout at VA 0x004BB190, preserves that result across
buffer cleanup, restores ESI/EDI and RETs. Ghidra identifies _printf with a
Visual Studio 1998 Release library signature. This is CRT identification evidence;
it does not establish the game's compiler, build flags or linked layout.

Live localhost:8080 Ghidra disassembly/decompilation/xrefs corroborate local
instructions and strings. Authentic IGN_WIN.EXE being active is the user's
operating assumption; no automated loaded-program fingerprint is claimed.
The sole banner caller in a complete bounded FPO instruction scan is
Mem_InitSystem's CALL at VA 0x0045B170, and its return is ignored. Ghidra lists
the same caller and one direct data reference for each string. This is not proof
against arbitrary indirect references. No Ghidra mutation or raw decompiler
output is committed.

## Production reconstruction and execution

The former DOS-annotated body had a colon, omitted the leading newline and one
trailing copyright newline. It is replaced in place with the Windows comma and
exact newline sequences, retaining the compilation text and unconditional zero.
EXACT records the recovered behavior, not instruction equality. No deviation
from Windows behavior is introduced; unrelated legacy lisa3d.c routines remain
untrusted and are not migrated by this feature.

PowerShell from repository root:

```powershell
$env:UV_CACHE_DIR = 'build/uv-cache'
$env:UV_OFFLINE = '1'
uv run python tools/verify_mem_lifecycle.py
uv run python tools/workflow.py complete --rva 0x5B4F0 --rva 0x5B170 --rva 0x5B1A0 --rva 0x55AB0 --limitation "Two graphics startup dependencies, CRT printf/heap and callbacks modeled; instruction equality and native console/input/graphics/heap/game parity unverified"
```

The shared focused builders extract the exact production body and existing
public prototype into a separate translation unit, linked with complete mem.c
and the real Input_ResetCallbacks. A separate TU is necessary: compiling the
banner alongside the nonreturning printf fixture permits dead-argument removal
and truncates the second call. All shared-builder consumers now link this body
and fingerprint lisa3d.c/lisa3d.h as inputs. Full lisa3d.c and geputget.c remain
unvalidated legacy modules; only the bounded extracted bodies are compiled.

Clang/LLD 19.1.1, i686-pc-windows-msvc, strict C89, -O2, freestanding/no-builtin,
no inlining/unrolling/vectorization/SSE are provisional behavior-validation flags.
The output is an import-free focused PE32 validation DLL, not a playable game.

306 standalone differential invocations pass: all 25 pairs of representative
printf returns (0,1,2,high-bit,-1), 128 seeded random pairs, and each pair followed
by another call without resetting state. Actual stack pointers are decoded to
exact NUL-terminated format/vararg bytes. The first printf has one string vararg;
the second has none. Both original PE and compiled production instructions are
compared with an independent instruction/data-derived oracle. CRT models mutate
callback words at both call boundaries and clobber volatile registers/flags;
EAX still returns zero. Ordered calls, boundary snapshots, complete tracked
state, nonvolatile registers, stack, DF and unrelated image bytes are checked.

The real banner and reset execute through 18 integrated startups (14 fresh,
four persistent). Mutations at both printf boundaries precede real pool/handle
initialization and callback clearing. Retained lifecycle coverage is 112 startup /
519 shutdown comparisons, including 94/74 isolated wrapper contracts and ten
persistent allocation/registration/lifecycle calls. The isolated wrapper tests
intentionally model all six dependencies, including banner/reset, to test ignored
helper results. Standalone reset coverage remains 266 comparisons.

Compilation, raw-prefix diagnostics and differential emulation have separate
tracking records. Raw prefixes differ; relocation-aware instruction equality
is unverified. CRT printf/heap, handle callbacks, Gfx_InitPrimitiveState and
Gfx_SelectBackend remain modeled. Format/argument forwarding is verified, while
CRT formatting, console bytes, stream errors and native console/runtime behavior
are not executed. Native input/graphics/heap/game parity, original compiler/link
layout, concurrency, general reentry and invalid/aliased storage remain unverified.

Next bounded candidate: Gfx_SelectBackend, VA 0x00456AF0 / RVA 0x56AF0, 30 bytes.
Independently recover its selector/global pointers, initialization and downstream
call ABI before replacing legacy C or promoting reconstruction status.
