# Windows graphics allocation dependencies

Authentic IGN_WIN.EXE (915,968 bytes), SHA-256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386, preferred base VA 0x00400000. Local target doctor verifies the
fingerprint. Read-only localhost:8080 disassembly and xrefs corroborate local
instructions. The active Ghidra target is a user-provided operating assumption,
not an automated identity check. Original assets are untouched. Names and
geputget.c placement are semantic, not recovered original symbols.

## Independently recovered dependencies

| Routine | VA / RVA | Bytes; FPO | SHA-256 |
| --- | --- | --- | --- |
| Gfx_AllocBytes | 0x0045F4A0 / 0x5F4A0 | 21; (21,0,1,0) | 2a7819df61b5e4eddd3b8f22a5a85e18f39ff17301f5ab6db1733e69913da9f6 |
| Gfx_AllocAlignedBytes | 0x00461840 / 0x61840 | 47; (47,0,2,0x20A) | 02150c922745d67bfe77182db1c615fb172490b6f730092ffb3e6c460b3f3b34 |

Complete instruction decoding ends in RET; 11/1 INT3 padding bytes follow.
Neither has HIGHLOW operands or a relocated pointer to its entry. Both use
cdecl stack arguments and preserve EBX/ESI/EDI/EBP, caller stack and clear DF.
The wrapper reads one raw size word: zero returns null without calling CRT;
nonzero calls authentic CRT malloc VA 0x00469400 and returns its EAX unchanged.
It adds no initialization, ownership registration, error handling or cleanup.
The FPO direct-call scan finds 49 wrapper call sites; Ghidra reports a subset.
No whole-program absence-of-indirect-reference claim is made.

The aligned helper takes size then alignment as unsigned 32-bit words. Its
only observed caller is page allocation VA 0x0046180F, passing two 0x10000
words and cleaning eight bytes. It calls the real wrapper with the wrapping
sum size+alignment+4. With allocation address A, the original zero-EDX DIV
computes R=((A+4) modulo 2^32) modulo alignment. It writes A as a dword at
H=(A-R+alignment) modulo 2^32 and returns (H+4) modulo 2^32. This is not a
power-of-two-only mask. An already aligned A+4 advances by a full alignment.
The preceding word is a back-pointer; no payload bytes are initialized.
All arithmetic is explicit uint32_t in production, preserving wrapping sums.

Neither routine checks malloc failure. Alignment zero faults at DIV after
allocation; a null allocation can attempt a low-address write. A wrapping
zero request bypasses malloc and still reaches the aligned helper's arithmetic
and store. There is no rollback/free. These cases are not converted to clean
null returns. Native exception behavior and portable C definedness for invalid
accesses/division by zero are not asserted: compiled x86 fault effects are
compared in the stated provisional build only.

## Production and validation

Readable production C89 bodies live in geputget.c with public prototypes.
The focused builder extracts these exact bodies, using provisional Clang/LLD
19.1.1, i686-pc-windows-msvc, -O2, strict C89, freestanding/no-builtin,
disabled inlining/unrolling/vectorization/SSE. CRT malloc is a nonreturning
validation-only fixture intercepted by the verifier; no production substitute
returns invented success. The aligned helper executes the real wrapper.
The aggregate symbolic dependency guard retains the prior 31 calls, adds one
aligned-to-wrapper call, and the new suite independently guards Clang's
conditional tail jump from the wrapper to malloc.

`tools/verify_sprite_packing.py` is part of the real aggregate verifier. It
passes 48 wrapper and 512 aligned-helper comparisons per binary. These include
24/252 persistent follow-ups without CPU/image reset and eight aligned-helper
fault cases. Sizes include zero, 1, 24, 65536, high-bit and all-one words;
alignments include 1/2/3/4/7/16/255/256/65536; seven allocator offsets exercise
residue boundaries and already aligned results. Zero alignment, wrapping sums,
null and high/wrapped allocator results exercise division and write faults.
Fault cases use fresh CPUs, as Unicorn retains exception-delivery state.

An independent raw-address oracle checks ordered reads/writes and CRT calls,
full one-MiB arena, template/head state, complete snapshots at CRT entry,
untouched image bytes, normal EAX/stack/nonvolatile-register/DF behavior, and
fault kind/address with preceding effects. CRT returns clobber volatile
registers/flags. Malloc/native heap behavior is modeled. No full packing,
native graphics/heap/game parity, instruction equality or original compiler/
link layout is certified. EXACT denotes recovered bounded behavior.

Use UV_CACHE_DIR=build/uv-cache and UV_OFFLINE=1 with Python through uv.
`uv run python tools/workflow.py complete --rva 0x5F4A0 --rva 0x61840`
renews all applicable compilation/emulation evidence, audits and exports.
The page allocator RVA 0x617E0 remains a separate feature until reconstructed
and independently verified. Existing lifecycle and memory/font/file/backend
suites remain intact, including 864 link comparisons and 810 packing resets.


## Real page caller integration follow-up

[Page allocation](windows_gfx_sprite_packing_pages.md) now executes these real
helpers in 134 page cases, including six failure cases, alongside its isolated
boundary cases. Standalone helper counts stay 48/512. The preceding separate
feature description is historical; page publication is now reconstructed.
CRT remains modeled; native heap/runtime and instruction matching are unverified.
