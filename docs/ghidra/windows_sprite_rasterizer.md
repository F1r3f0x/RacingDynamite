# Native sprite rasterizer entry — corrected extent and integration blocker

Follow-up: [triangle execution analysis](windows_triangle.md) now executes the real
triangle through return and both transformed calls through outer return. It checks
312 unclipped expression cases, 60 clipped execution cases and 24 transformed cases
(ten with combined framebuffer expressions). First-call live preparation scratch
preservation is observed in those fixtures. Complete C reconstruction is blocked
on clipped edge splitting at RVA `0x24F270` and its continuation, with the outer ESI
integration still unresolved. The evidence below describes the preceding feature
and its then-current first-call stopping boundary; it is retained as historical
scope, not a current claim that no transformed framebuffer bytes have executed.

Recovered 2026-10-07 from the authenticated standard Windows release
`Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
PE32 x86, preferred image base `0x00400000`. Target doctor passed, tracked
hooks are installed, and workflow preflight declares feature paths and RVAs.
The checkout began at `ba98cf6` with only unrelated `tools/dump_456470.py`;
that file and original binaries/assets are untouched. No concurrent feature
changes were observed before taking database/export ownership.

Ghidra reachability was rechecked outside the sandbox: `methods` and `segments`
returned HTTP 200. Identity queries `program`, `program_info`, `get_program_info`,
`getCurrentProgram`, and `list_programs` returned 404. Loaded-program identity
remains unverified. No bridge disassembly, symbol mutation or synchronization
was used. Evidence comes from fingerprinted PE bytes, Capstone and Unicorn.

## Entry boundary and ABI

Analysis name `Gfx_RasterizeSpritePacket`, preferred VA `0x00465BB5`,
RVA `0x65BB5`. **The observed 437-byte prefix is not the complete body.**
Its last two bytes are POPAD/RET at `0x00465D68/69`, but the conditional branch
at `0x00465CF0` targets `0x00465D6A`. That tail handles narrow spans and jumps
back into the prefix at `0x00465D36`, `0x00465D52` and `0x00465D50`.
All branches are instruction-aligned and inside the corrected 474-byte extent
`[0x00465BB5,0x00465D8F)`. A single INT3 at `0x00465D8F` separates the next
routine at `0x00465D90`. No authentic FPO record covers this entry.

| Extent | Bytes | SHA-256 |
| --- | ---: | --- |
| Historical prefix, RVA 0x65BB5 | 437 | `8670817dfcf88d20b163da537eb3df0b063620fc8e2b4fe7d96aea01a78b2102` |
| Complete entry, RVA 0x65BB5 | 474 | `2cdb543878a2ce01c034f50d8f679a881f1491d088a64746b18ce6273d464fbb` |
| Transform helper, RVA 0x68C70 | 631 | `c239f5eb8a12e6b59eb775b316f45e4d8c37ad81f2c0b66910cda5033f8f96b2` |
| Triangle dependency, RVA 0x5CA50 | 1813 | `4458bb7690590a3289e6ce7d99b3a80338382fe2afc3b67a25891b2af0e5f8bf` |

Input is a packet pointer in **ESI**, with no stack arguments. Entry executes
PUSHAD; transformed return uses POPAD/RET at `0x00465BE8/9`, and untransformed
return uses POPAD/RET at `0x00465D68/9`. General registers, including incidental
EAX, and caller ESP are restored. EFLAGS are not saved/restored by these
instructions; no semantic return value is established. The emulator checks all
seven non-ESP general registers and caller stack on untransformed returns.
Transformed return is corroborated statically, not executed in this feature.

Packet dwords: +0 is unused by this entry, +4 points to two position dwords,
+8 points to eight prepared dwords, +12 is null or points to four coefficients.
The adapter's packet VA `0x004BA778` is file-initialized `(4,0,0x004BA758,0)`;
+8 has HIGHLOW relocation RVA `0xBA780`. The consumed layout is aligned dwords,
independently observed in Windows instructions. It does not establish a full
image descriptor, handle allocation or ownership layout.

## Untransformed rendering effects

Let prepared dwords be `d0..d7`. Arithmetic wraps at 32 bits; SAR interprets
the wrapped value as signed. Initial ordered writes are:

1. VA `0x004BAD28`: replace the low byte of `d3` with byte +9 of the prepared
   descriptor, then add `d6`. Thus source is
   `((d3 & 0xFFFFFF00) | ((d2 >> 8) & 255)) + d6`, modulo 2^32.
2. VA `0x004BAD2C`: `(signed(d4) >> 8) - (signed(d2) >> 8)` (width).
3. VA `0x004BAD30`: `(signed(d5) >> 8) - (signed(d3) >> 8)` (height).

Position is `(signed(wrap(point.x-d0)) >> 8,
signed(wrap(point.y-d1)) >> 8)`. The active clip ordering is:

| Preferred VA | Observed role |
| --- | --- |
| 0x0063F2B4 | exclusive bottom |
| 0x0063F2B8 | inclusive top |
| 0x0063F2BC | exclusive right |
| 0x0063F2C0 | inclusive left |
| 0x0063F2C8 | framebuffer row stride, dword |
| 0x0063F2D4 | framebuffer pointer |

Clipping proceeds top, bottom, left, right, with signed x86 branches. Top
clipping subtracts removed rows from scratch height; if JLE rejects the span,
the source pointer is **not** subsequently adjusted. Otherwise it adds rows<<8
to source and clamps Y. Bottom clipping subtracts overrun from height. Left
clipping subtracts columns from width, rejects before source adjustment, then
adds columns to source and clamps X. Right clipping subtracts width overrun.
Scratch changes persist even on rejection. These ordered effects are checked,
including exact-edge and fully-outside cases.

Destination starts at framebuffer + X + Y*stride, with dword wrapping. Source
rows advance by **256 bytes**, independently of framebuffer stride. The high
16 bits of `d7` supply a 64-KiB lookup table base; the low 16 bits are replaced
with `(source_byte<<8)|existing_destination_byte`. Every rendered byte is the
corresponding table byte. There is no explicit source-zero transparency branch;
any transparency/compositing behavior resides in table contents whose original
initialization is not recovered here. The adapter's packed upper descriptor
word feeds `d6`, and its live render word feeds `d7`; no pointer interpretation
is inferred from the legacy DOS `SpriteDesc` type.

Rows proceed downward. Within each row, writes proceed from right to left:
four-byte stores while at least six pixels remain, followed by descending byte
stores. Source/destination reads are pipelined ahead of writes; a per-pixel loop
with identical final bytes would not alone prove aliasing or ordered-write fidelity.
The original-only tests check actual store addresses, widths, values and order.
Widths 1 and 2–5 use the newly recovered tail. Width zero/negative and invalid
height can bypass ordinary assumptions: there is no unconditional positive-size
guard before the row loop, whose DEC/JNE requires an appropriate height. Invalid
geometry, overflowed clip arithmetic and overlapping source/framebuffer/table
storage remain outside the dynamic validation scope; do not add safety guards
or assume such calls are defined by an eventual C API without further evidence.

Scratch `0x004BAD28–30` is file-backed zero-initialized data. Clipping, stride
and framebuffer storage lie in the loader-zeroed `.data` virtual tail; their
runtime initialization and actual native surfaces remain unverified.

## Transformed path and smallest remaining dependency

With packet +12 non-null the entry writes, in order: point X/Y to
`0x004BAD40/44`, prepared pointer to `0x004BAD38`, coefficients pointer to
`0x004BAD3C`. It passes ESI=`0x004BAD34` to VA `0x00468C70` at call site
`0x00465BE3`. Scratch +0 at `0x004BAD34` is file-initialized zero and unchanged.
This path performs no entry-local clipping or framebuffer write.

Analysis name `Gfx_PrepareTransformedSprite`, VA `0x00468C70` / RVA `0x68C70`,
631 bytes through RET at `0x00468EE6`, followed by one INT3. It has no FPO
record. Its ESI input comprises unused +0, prepared pointer +4, coefficients
pointer +8, position X/Y +12/+16. It saves EDI with PUSH/POP, consumes/clobbers
other general registers, and relies on the outer PUSHAD for restoration.

It copies prepared texture Y0,Y1-16,X0,X1-16, lookup word and source base into
`0x004BAF3C–50`. The subtraction of 16 is observed fixed-point behavior, not
a corrected integer-pixel edge. Signed IMUL followed by MOV AX,DX / ROL EAX,16
extracts the low 32 bits of the signed 64-bit product shifted right by 16.
Each input NEG and ADD wraps at 32 bits.

With `mul16(a,b)=low32(signed(a)*signed(b)>>16)`, local geometry is:

- A.x = mul16(wrap(-d0),c0) + mul16(wrap(-d1),c2).
- A.y = mul16(wrap(-d0),c1) + mul16(wrap(-d1),c3).
- H = (mul16(wrap(d4-16-d2),c0), mul16(wrap(d4-16-d2),c1)).
- V = (mul16(wrap(d5-16-d3),c2), mul16(wrap(d5-16-d3),c3)).
- Corners A, B=A+H, C=A+V, D=A+H+V then each receive position X/Y.

All intermediate scratch writes, including updates later overwritten, are
checked against independently derived expressions in 72 original executions.
Both positive sine coefficients are consumed exactly as observed; no sign fix
or conventional rotation replacement is introduced.

The helper builds a 15-dword triangle packet at `0x004BAF54`: unchanged +0,
three (X,Y) pairs at +4..+24, three (texture X,Y) pairs at +28..+48,
source base +52, lookup word +56. First triangle is A/B/C with texture corners
(X0,Y0)/(X1-16,Y0)/(X0,Y1-16). It calls `0x0045CA50` with one stack argument
`0x004BAF54` at `0x00468E6B`, then removes that argument with POP EAX.
The second packet uses B/C/D and texture corners
(X1-16,Y0)/(X0,Y1-16)/(X1-16,Y1-16), followed by the same caller-cleanup call
at `0x00468EDF`. This second preparation and the helper's epilogue are static
evidence only. They read live shared scratch after the first call: dependency
mutations must be recovered, not assumed absent.

`0x0045CA50` / RVA `0x5CA50` has an authentic 1,813-byte FPO extent, one stack
parameter, complete decode and final preserved EBX/ESI/EDI/EBP epilogue.
Its prologue reads the packet from caller ESP+4, copies geometry/texture fields
into separate scratch and consumes framebuffer state. It remains **named**,
with only this boundary analyzed; its triangle clipping, division, scan conversion
and downstream rendering contracts have not been recovered. Three observed
direct-call targets lacking FPO records, VAs `0x0064EA80`, `0x0064EC00`,
`0x0064EC40` (RVAs `0x24EA80`, `0x24EC00`, `0x24EC40`),
are imported as **unidentified entries with unknown size/hash/ABI**. Existing
FPO call-target candidates are retained unchanged. These targets are file-backed
in the PE `code` section, whose header lacks the execute flag. Import uses explicit
`--allow-nonexecutable` based on the authenticated direct calls; modern NX/native
execution implications are unverified. No transitive renderer migration
or claim of complete triangle behavior accompanies these records.

## Reproduction, results and integration decision

```powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
uv run --offline tools/analyze_sprite_rasterizer.py
```

Pinned pefile 2024.8.26, Capstone 5.0.7 and Unicorn 2.1.4. The script authenticates
the target, pins routine hashes, checks entry control-flow closure and initialized
storage, records HIGHLOW operands and writes diagnostic disassembly only under
ignored `build/decomp/windows/`. Its JSON report also stays under `build/`.
Raw disassembly/decompiler output and binary assets are not committed.

704 original-only cases execute the complete untransformed entry through return:
widths 1–20/33/63, fractional position boundaries, all four clip sides, exact
edges and rejection, seeded source/framebuffer/table bytes, ordered scratch and
byte/dword framebuffer stores, saved registers/stack, unchanged input/table/source,
and no other image/data writes. Positive valid geometry and disjoint storage are
the declared dynamic scope. 72 cases execute entry plus transformed helper to
the **first instruction** of `0x0045CA50`, checking all ordered shared writes,
wrapped products including extreme coefficients, PUSHAD stack layout, ESI and
the cdecl argument/return-address boundary. Execution stops there; no dependency
return is manufactured, no triangle body runs, and the second call/return path
is not dynamically validated.

No new production-C body, compilation result, instruction comparison,
original-versus-C differential result or native runtime result is recorded.
Entry/helper remain analyzed with unknown implementation fidelity. Existing
production validation records are unchanged. Focused candidate-import tooling
has four passing isolated guard tests; this is infrastructure testing, not
renderer fidelity. Fidelity audit and workflow analysis-only completion regenerate
the SQLite-derived exports without promoting test evidence from status labels.

A future internal C function may explicitly accept this packet **only after**
recovering its complete relevant effects, preserving persistent scratch and ordered
framebuffer accesses, and validating the transformed dependency. Replacing the
native ESI call now would leave `0x0045CA50` unreconstructed; retaining that call
would still lack a verified assembly-free C interface. Therefore the immediate
integration objective remains blocked, while the bounded entry analysis is complete.
The next evidence-supported routine is `0x0045CA50` / RVA `0x5CA50`, beginning
with its one-packet cdecl contract and original triangle clipping/rendering effects.
Original compiler identity, instruction equality, native rendering and playable
game reconstruction remain unverified.
