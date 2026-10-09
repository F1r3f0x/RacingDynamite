# Windows graphics free dependencies

Authenticated 2026-10-09 from standard IGN_WIN.EXE, 915,968 bytes, SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782.
PE32 i386 image base VA 0x00400000. Local target doctor passes. Bounded live
Ghidra disassembly/xrefs at localhost:8080 agree with independent Capstone
decoding. Authentic active program selection is the user-provided operating
assumption, not an automated identity check. No DOS evidence is used.

| Semantic name | VA / RVA | Extent; FPO tuple | SHA-256 |
| --- | --- | --- | --- |
| Gfx_FreeBytes | 0x0045F8B0 / 0x5F8B0 | 18; (18,0,1,0) | 404196d294b1f0112a432fb80398ae7d09669d883e27df694804678cd1d91d94 |
| Gfx_FreeAlignedBytes | 0x00461A60 / 0x61A60 | 17; (17,0,1,0) | 396fe877c29ad0654d7555f73c4f499cb23e2069f5d64946b6ecb8ad14c6588b |

Both file-backed .text routines end in ordinary RET; 14/15 INT3 bytes follow.
Neither has HIGHLOW operands or a relocated entry pointer. Each takes one
cdecl stack pointer, preserves nonvolatile registers, and uses caller cleanup.
Incidental EAX is not a meaningful C result: null byte free leaves zero; a
nonnull byte free retains the unspecified CRT free return register. Aligned
free propagates it. Callers discard it. Production signatures are void.

Byte free loads its pointer, tests exact zero and skips the CRT for null.
Otherwise it passes the pointer to CRT free VA 0x004693B0 at call VA
0x0045F8B9 and cleans four bytes. It never dereferences the pointer itself.
Aligned free loads the unaligned dword at pointer minus four modulo 2^32,
passes that captured back-pointer to real byte free at VA 0x00461A68,
and cleans four bytes. It does not check null, alignment, provenance or size.
A null back-pointer skips CRT free after the header read; a null aligned
pointer attempts the read at VA 0xFFFFFFFC. No invented guards are added.
This independently agrees with aligned allocation RVA 0x61840's header layout.

Aligned free's sole direct caller is packing release RVA 0x618B0 at VA
0x0046199C. Its full 419-byte extent/FPO (419,1,1,0x1410) has hash
7563059eb087b1961ce5aac8901e5ed29574c2f1a9354e62ed943d5842fb0072.
Release then unlinks/frees the page and frees bucket/container/leaf through
byte free. Ghidra also identifies many other byte-free callers; their whole
contracts remain unreconstructed. No whole release integration is claimed here.

Production C89 is in geputget.c with public prototypes in geputget.h. The
focused builder extracts actual production bodies and uses a separate
nonreturning CRT fixture, intercepted only by the emulator. No production
dependency stub is added. Clang/LLD are provisional behavioral compilers;
original compiler/version/options and linked layout remain unresolved.

Run `uv run python tools/verify_sprite_free.py`. The independent raw-address
oracle compares authentic instructions and compiled C against hand-derived
header/free expectations, exact ordered reads, free arguments and pre-free
snapshots, all arena/global/unrelated-image bytes, normal cdecl caller stack,
nonvolatile registers and DF. CRT free has modeled return/clobbers and explicit
header poisoning; persistent calls must reread the changed header and skip the
CRT when it becomes zero. Tests include arbitrary pointer/back-pointer words,
null byte free, unaligned and mapped-edge headers and unmapped header faults.
Incidental EAX and private stack layout are excluded. Read faults do not imply
native exception or portable unchecked-access parity. General reentry,
concurrency, arbitrary image/stack aliases and native heap effects are unverified.

Compilation and differential emulation are recorded separately. New
coverage is 216 byte-free comparisons (144 persistent follow-ups) and
920 aligned-free comparisons (608 persistent follow-ups, eight read faults).
All existing coverage is retained through workflow complete. Instruction equality, original
compiler/link layout, native graphics/heap/game/fault parity and a playable
rebuilt game remain unverified. Packing release is the next production feature.
