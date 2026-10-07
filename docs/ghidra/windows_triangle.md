# Windows triangle execution and bounded contract recovery

Recovered 2026-10-07 from the authentic `Ignition/Ignition/IGN_WIN.EXE`:
915,968 bytes, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 x86, preferred base `0x00400000`. All addresses below are preferred VAs
unless explicitly labeled RVA. The checkout began at
`95d7c01fd303c59dc50044ba419f07ac973b3e3d`, snapshot `95be4cd5df1b`.
Only unrelated untracked `tools/dump_456470.py` existed; it is untouched.
Other chats sharing this checkout were idle when this feature took sole ownership
of its new analysis harness, SQLite updates, exports and commit. Existing production
verifiers and original binaries/assets are unchanged. Tracked hooks were installed;
preflight declares the affected RVAs and explicit feature paths, extended before
changing the candidate extent CLI.

Ghidra `methods`/`segments` returned HTTP 200 after a read-only localhost probe
outside the restricted sandbox. `program`, `program_info`, `get_program_info`,
`getCurrentProgram` and `list_programs` returned 404. The available bridge has
no byte-read or fingerprint operation. Loaded-program identity remains
unauthenticated; no Ghidra analysis or symbol mutation was used. Evidence is from
fingerprinted PE bytes decoded with Capstone and executed with Unicorn.

## Packet, ordering, clipping dispatch and returns

Analysis name `Gfx_RasterizeTexturedTriangle`, VA `0x0045CA50`, RVA `0x5CA50`.
Authentic FPO extent: 1,813 bytes, `[0x0045CA50,0x0045D165)`;
SHA-256 `4458bb7690590a3289e6ce7d99b3a80338382fe2afc3b67a25891b2af0e5f8bf`.
One cdecl stack argument is a 60-byte packet. Entry saves EBX/ESI/EDI/EBP and
reads the argument at entry ESP+4. All return paths restore those registers and
consume only the return address. EAX/ECX/EDX and flags are scratch; incidental
EAX is not an established semantic return. The input packet is read, not sorted
in place. Aliases with internal writable scratch remain outside the fixture scope.

| Packet offset | Observed role |
| --- | --- |
| +0 | Not consumed |
| +4,+8 / +12,+16 / +20,+24 | Three signed X,Y pairs with eight fractional bits |
| +28,+32 / +36,+40 / +44,+48 | Corresponding texture U,V pairs |
| +52 | Texture base |
| +56 | Lookup/compositing word |

The first twelve field stores copy geometry/texture into `0x004BAD74–A0`, in
packet order. Texture base is stored at `0x0051A9C4`, and the active framebuffer
pointer from `0x0063F2D4` at `0x0051A9D0`. X minima/maxima at
`0x0051A9D4/F8` are computed **before** sorting. Signed comparisons swap complete
vertices and their UVs: compare vertex 0 against 2 in Y, then 0 against 1,
otherwise compare 2 against 1. Equal-Y vertices retain this instruction-defined
tie ordering. Original packet bytes remain unchanged.

Fast-path tests are signed, in order: minimum Y >= `0x0063F2B0`, maximum Y <=
`0x0063F2C4`, maximum X <= `0x0063F2D0`, minimum X >= `0x0063F2CC`.
These are **fixed-point bounds**, distinct from integer clip bounds used by spans.
Failures call RVA `0x5D170` at VAs `0x0045CC66`, `0x0045CC81`, `0x0045CC9C`,
or `0x0045CCB7`, passing the original packet. They return immediately afterward,
at `0x0045CC72/8D/A8/C3`. Normal branches return at `0x0045CF7D` or
`0x0045D164`. All six return sites executed in the new cases.

`0x0045D170` has a complete authentic FPO extent of 1,569 bytes. It assumes
already populated/sorted scratch, rejects wholly outside geometry with strict
signed comparisons, and otherwise uses clipped edge generators and either the
ordinary span wrapper or a four-argument span wrapper. This is not a general
standalone packet decoder. Its clipping continuation contract is still partial.

## Recovered fast-path dependencies

These are analysis names, not recovered linker symbols. All extents fully decode,
end in RET, have closed instruction-aligned conditional/jump targets, and are
hash-pinned by `tools/analyze_triangle.py`. Call closure is also exercised by a
hook that permits only instruction starts decoded from the authenticated bodies.
No helper return or rendering effect is synthesized.

| RVA | Bytes | Analysis role and ABI |
| --- | ---: | --- |
| 0x24EA80 | 380 | Texture gradients; no args, shared triangle scratch, clobbers EAX/EBX/EDX; ECX/ESI/EDI/EBP unchanged |
| 0x24EC00 | 59 | Corrected right edge; ESI=edge packet, EBX=output, saves EBP, clobbers other working regs |
| 0x24EC40 | 59 | Raw left edge; same register interface |
| 0x24F4D0 | 199 | Edge increments/first sample from ESI, shared output scratch, no stack args |
| 0x24F1E0 | 51 | Raw records: EAX=V, ESI=U, EDX=X, EBX=output, ECX=count |
| 0x24F220 | 70 | Corrected records: same interface, dword count |
| 0x5D7A0 | 53 | cdecl (x0,y0,x1,y1) slope; preserves ESI, returns EAX |
| 0x5C990 | 24 | Four-argument cdecl setup wrapper, preserves EBX |
| 0x67F1C | 38 | Setup: EAX=dV, EBX=dU, ECX=texture, EDX=lookup |
| 0x5C9B0 | 32 | cdecl (destination,count,packedUV,Uinteger,leftRecord), preserves EBX/ESI/EDI |
| 0x67F42 | 85 | Span: EDI=destination, ESI=count, ECX=packedUV, EBX=Uinteger, EAX=leftRecord; saves EBP |
| 0x5C9D0 | 28 | cdecl (destination,count,packedUV,Uinteger), preserves EBX/ESI/EDI |
| 0x67F97 | 53 | Span without the separately sampled left pixel; saves EBP |

The `code` PE section containing RVAs `0x24...` is file-backed but lacks the
execute flag. Direct calls and real emulated execution authenticate these entries;
native NX behavior is not validated. PE imports are not called by this render path.
Arithmetic and memory access are x86 instructions, not CRT/compiler guesses.

For sorted vertices A/B/C, define wrapped dword differences h1=B.y-A.y and
h2=C.y-A.y. The gradient helper stores h1 at `0x004BADF4`, stores h2 at
`0x004BADF8`, replacing stored zero h2 with 1. It uses ratio `0x7FFFFFFF` when
the original h2 equals h1; otherwise unsigned DIV of `(h1:0)` by stored h2,
then SHR 1. For X/U/V it takes the signed high dword of
`signed(wrap(2*(C.field-A.field)))*signed(ratio)`. The signed denominator is
the interpolated X minus (B.x-A.x). Values -2 through 2 become -4 or +4 via
SAR 2, OR 1, SHL 2. Each UV difference from the interpolated point to B is
shifted into a signed 64-bit numerator by 16 and IDIV uses that denominator,
truncating toward zero. Quotient overflow remains a CPU exception.

Gradients dU/dV persist at `0x004BADA4/A8`. Eight U and V correction dwords at
`0x004BADB4–D0` and `0x004BADD4–F0` are multiples 0..7 of
`signed(wrap(-gradient)) >> 3`, with wrapping additions. Intermediate values
and ratio/denominator persist at `0x004BADF4–0x004BAE10`.

Slope RVA `0x5D7A0` returns signed IDIV of
`signed(wrap((x0-x1)<<8)) / signed(wrap(y0-y1))`. Equal Y returns
`0x80010000` if signed x1<x0, otherwise `0x7FFF0000`. If slope AC <= slope AB,
AB+BC forms the corrected right side and AC forms the raw left side. Otherwise
AC is corrected right and AB+BC is raw left. No area/culling return is present.

Edge setup uses dy=y1-y0. If unsigned dy<=64, abs(wrapped dx)<=65536 changes
dy to 65; otherwise dy<=3 changes it to 3. Texture increment IDIV constructs
its high dword as only the **sign of wrapped delta**, and low dword delta<<16;
it is not a general mathematical signed delta<<16 for arbitrary large deltas.
X increment uses the true signed dx<<16 numerator. With f=256-(y0&255),
U/V accumulators start at wrap((UV0<<8)+SAR(wrap(f*increment),8)); X starts
at wrap((x0<<8)+f*SAR(Xincrement,8)). Even exact integral Y starts at the
next sample, since f=256. This must not be replaced by a conventional triangle
coverage formula.

Output buffers are pointers at `0x0051A9D8/DC/E0`. The real initializer
RVA `0x5C9F0` allocates **0x20D8 bytes each**, through three calls to
`0x00469400`, and sets `0x004BACE4`; allocator/startup execution is not part of
this feature. Fixtures supply disjoint buffers of that capacity rather than
fabricating allocator returns. Headers contain SAR(y0,8) and
SAR(y1,8)-SAR(y0,8), followed by 12-byte (V,U,X) records. Zero count skips edge
setup/emission. Raw records use SAR(accumulator,16); their loop decrements **CX**
and tests signed JG. Corrected records select bucket `(X>>13)&7`, add the relevant
UV correction then SAR 8, while X remains SAR 16; this loop decrements full ECX.
Large counts/overflows and buffer overruns are not claimed supported by fixtures.

## Scan spans and persistent effects

Row order is ascending Y, upper half then lower half. The long-side record index
continues across the half boundary. Shared framebuffer cursor `0x0051A9D0`
advances by `0x0063F2C8` every processed row, including empty/reversed spans.
Normal span arguments are cursor+leftX+1, rightX-leftX-1, a packed UV word
from the corrected right record, its SAR(U,8), and the raw left record pointer.

Setup stores lookup at `0x004BAD6C`, texture at `0x004BAD68`, byte step
`(wrap(-dU)>>16)&255` at `0x004BAD60`, and packed step
`((wrap(-dU)&65535)<<16) | ((wrap(-dV)>>8)&65535)` at `0x004BAD64`.
The low 16 bits of texture and lookup register addresses are overwritten by
coordinates/indices; a general unaligned-base pointer interpretation is wrong.
Fixtures deliberately use 64-KiB-aligned texture and lookup regions.

Span entry first reads a texture byte at (left V low byte,left U low byte),
stores it at `0x004BAD70`, **before testing span count**. Main pixels go right
to left: add packed step to ECX, put CH into texture address BH, ADC byte step
into BL using the carry from the dword add; read destination, texture, lookup;
store the lookup byte. The table index is `(textureByte<<8)|destinationByte`.
There is no texture-zero bypass. After ordinary pixels, if the original count
was nonnegative, it also composites the cached left sample at destination-1.
Negative count performs the initial texture read but no framebuffer write.
The clipped four-argument span omits that cached left pixel. Store order and
the rendering read order are independently checked on the unclipped path,
including empty/narrow spans; final framebuffer equality alone is insufficient.

Shared triangle/gradient/edge scratch persists; input vertices and texture/table
fixtures do not change. `0x004BAD60–0x004BAEEF` is verified file-zeroed data.
`0x0051A9A0`/`0x0051AA00` working edge packets, buffer pointers, cursor and
viewport fields are in loader-zeroed virtual `.data`. Actual surface/lookup
initialization remains outside scope. All observed non-stack writes are confined
to these declared scratch regions, supplied edge buffers and framebuffer.
Stack/register restoration and unchanged inputs/table/texture are checked.
Aliasing between texture/table/framebuffer/scratch, flags as an external contract,
arbitrary clip-state inconsistency and full arithmetic fault space are unverified.

## Both transformed calls and remaining clipping blocker

The outer ESI packet routine RVA `0x65BB5` executes its real PUSHAD, preparation
RVA `0x68C70`, both real triangle calls, POPAD and outer RET in 24 new cases.
All seven non-ESP general registers and outer ESP are restored. Second packet
geometry/UVs are checked independently from wrapped mul16 corner expressions.
In these fixtures the first triangle leaves **all 116 bytes** at
`0x004BAF1C–0x004BAF8F` unchanged between triangle entry and helper resumption
at `0x00468E70`. Thus its persistent writes occur in separate triangle/edge/span
scratch, and do not alter the live corner/UV scratch used to prepare B/C/D.
This preservation is now observed, not assumed; it is limited to disjoint valid
fixture storage and the executed paths. The original packet supplied externally
is also unchanged. Twelve transformed cases produce stores in both triangles.

Ten wholly inside transformed cases compare both calls' combined ordered rendering
accesses, stores and final framebuffer to independently derived expressions,
carrying the first framebuffer into the second. Remaining transformed cases
execute the clipped/reject/degenerate paths and check calls, packet preparation,
scratch preservation, unchanged inputs and ABI, without a full clipped pixel oracle.

Exact blocker: VA `0x0064F270`, RVA `0x24F270`, 537 bytes through RET
`0x0064F488`. Clipped corrected/raw wrappers, RVAs `0x24EFA0` and `0x24F0C0`
(282 bytes each), call it at **0x0064EFDD** and **0x0064F0FD**. ESI points to
the mutable 32-byte (x0,y0,x1,y1,u0,v0,u1,v1) edge packet; no stack arguments.
It clobbers arithmetic registers and writes packet endpoints, split flag
`0x004BAEC8`, and saved endpoint quartet `0x004BAECC–D8`. The wrappers can emit
one segment, replace packet start with the clamped end, restore a saved end,
call RVA `0x24F489` (63 bytes) at `0x0064F072/0x0064F192`, and append a second
segment while changing output counts and shared output pointers.

Static evidence shows top/bottom endpoint interpolation using unsigned DIV of
`(removedDistance:0)` by edge distance, SHR 1, signed high products of doubled
deltas. Right-boundary branches clamp UV via the previously computed gradients,
or save a crossing endpoint and adjust Y/UV for a split. Exact-right equality
includes a `0xFFFFFFFF` ratio special case at `0x0064F44C`. **The entire split
continuation, its rounding at combined clip boundaries, and ordered mutations
have not been independently recovered/tested as a complete implementation
contract.** Original execution proves the path runs, not that a new clipping
implementation would be faithful. These routines and the main triangle remain
analyzed, fidelity unknown; no production C has been added.

Next executable experiment: invoke authentic RVA `0x24F270` directly with
ESI pointing at a disjoint edge packet and real gradient scratch. Test each
right-crossing direction, both endpoints outside right, exact-right equality,
top then bottom clipping, and fractional crossings. Derive and compare every
ordered packet/split-scratch write and IDIV/DIV fault path; then execute each
real clipped wrapper through its optional second segment, checking headers,
records, pointer increments and left-clipped span access order. Only after those
contracts are complete should production C and its compilation/differential
verifier be added. The outer ESI ABI also still needs a verified assembly-free
integration solution; a conventional C wrapper does not prove that contract.

## Reproduction and precise results

```powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
uv run --offline tools/analyze_triangle.py
uv run --offline tools/analyze_sprite_rasterizer.py
New-Item -ItemType Directory -Force build/tmp | Out-Null
$env:TMP = Join-Path (Get-Location) 'build/tmp'
$env:TEMP = $env:TMP
uv run python -m unittest discover -s tests -p test_db_candidates.py
```

Pinned pefile 2024.8.26, Capstone 5.0.7, Unicorn 2.1.4. JSON report with target,
routine and analysis-input hashes and diagnostic disassembly are written only
under ignored `build/decomp/windows/`. Neither raw disassembly nor binary assets
are committed. Candidate CLI now authenticates a previously unknown extent with
`describe --size`, explicit evidence/confidence and non-executable opt-in; seven
isolated tests check hashes, fingerprint, capacity, atomic rejection and stage
preservation. This is infrastructure validation, not game compilation.

- **Original-only:** 312 unclipped comparisons, nine geometry families across all
  six vertex orders and four fractional offsets, plus 96 seeded cases with
  independently varying fractional vertices and signed UVs. Flat-top/bottom,
  both side orientations, narrow spans, horizontal/coincident/collinear geometry
  are included. 19,486 framebuffer byte stores match independent expressions;
  rendering read order, complete emitted edge records and key persistent scratch
  are checked. This is not original-versus-production-C differential emulation.
- **Original-only clipping:** 60 cases, each run twice, cover four sides, combined
  clipping and wholly outside rejection in six vertex orders. Full observed
  write/read/path sequences repeat; framebuffer stores stay inside integer clip
  bounds. This does not independently verify clipped pixel values or every
  scratch mutation against a semantic oracle.
- **Original-only transformed:** 24 outer-return cases; both calls execute,
  live preparation scratch and independently derived second packets match;
  ten cases have combined framebuffer/access expressions. Twelve cases render
  nonempty output from both triangles. Zero transform cases return without stores.
- **Fault/degenerate observations:** a narrow collinear packet with U1=`0x7FFFFFFF`
  raises `UC_ERR_EXCEPTION` at IDIV VA `0x0064EB44`; identical vertices with zero
  UV return normally. The harness asserts these observations and does not fix them.
- **Existing original-only scope reproduced:** 704 untransformed framebuffer and
  72 transformed first-call-boundary cases pass unchanged.
- **Compilation:** no new game C compilation. **Instruction comparison:** no
  comparison to compiled C or instruction-equality result. **Differential
  emulation:** none against production C. **Native validation:** none. Existing
  production verification records are unchanged; no playable/native rendering
  or original compiler identity is claimed.

Workflow analysis-only completion audits and regenerates SQLite exports. No
verification result is manufactured from these analysis labels. Transformed
framebuffer behavior is now checked within the stated original-only scope;
complete clipped rendering and C integration remain blocked.
