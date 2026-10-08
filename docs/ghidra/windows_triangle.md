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

## Recovered edge clipping and splitting contract (RVA 0x24F270)

Analysis name `Gfx_ClipAndSplitTriangleEdge`, VA `0x0064F270`, RVA `0x24F270`, 537 bytes through RET `0x0064F488`.
Verified direct callers: VA `0x0064EFDD` (in `0x24EFA0`) and VA `0x0064F0FD` (in `0x24F0C0`).
Input: ESI points to a mutable 32-byte edge packet `(x0, y0, x1, y1, u0, v0, u1, v1)` in fixed-point 24.8 / UV units; no stack arguments.
Registers EAX, ECX, EDX are scratch; general registers EBX, ESI, EDI, EBP and caller stack are preserved.

The routine performs sequential top, bottom, and right clipping:

1. **Top clipping (VA 0x0064F270–0x0064F2BD):**
   Compares fixed-point clip top at `0x0063F2B0` with `y0`. If `y0 < clip_top`:
   `dy = y1 - y0`, `dist = clip_top - y0`. Unsigned DIV divides `(dist << 32)` by `dy`, then `shr eax, 1` yields fixed-point ratio `0..0x7FFFFFFF`.
   Interpolates `v0`, `u0`, `x0` in order via signed 64-bit product of doubled delta: `start + (s32(2*(end - start)) * ratio >> 32)`.
   Sets `y0 = clip_top`.
2. **Bottom clipping (VA 0x0064F2BE–0x0064F30A):**
   Compares fixed-point clip bottom at `0x0063F2C4` with `y1`. If `y1 > clip_bottom`:
   `dy = y1 - y0`, `dist = y1 - clip_bottom`. Unsigned DIV divides `(dist << 32)` by `dy`, then `shr eax, 1`.
   Interpolates backwards: `v1`, `u1`, `x1` in order via `end - (s32(2*(end - start)) * ratio >> 32)`.
   Sets `y1 = clip_bottom`.
3. **Split state initialization (VA 0x0064F30B):**
   Clears split flag dword `0x004BAEC8` to 0.
4. **Right boundary branches and ratio special case (VA 0x0064F315–0x0064F488):**
   Tests `x0` against fixed-point clip right at `0x0063F2D0`:
   - If `x0 > clip_right`:
     - If `x1 < clip_right` (outside-to-inside crossing): edge splits.
       Sets split flag `0x004BAEC8 = 1`. Saves original endpoint 1 `(x1, y1, u1, v1)` into `0x004BAECC–D8`.
       Intersection ratio is `((clip_right - x1) << 32 / (x0 - x1)) >> 1`.
       Updates endpoint 1 to the crossing intersection `(clip_right, y_interp, u_interp, v_interp)`.
       Then clamps endpoint 0 to `clip_right` using texture gradients `dU` (`0x004BADA4`) and `dV` (`0x004BADA8`):
       `u0 -= mul16(x0 - clip_right, dU)`, `v0 -= mul16(x0 - clip_right, dV)`, `x0 = clip_right`.
     - If `x1 >= clip_right` (both endpoints outside right): no split (`0x004BAEC8` remains 0).
       Clamps endpoint 1 using `mul16(x1 - clip_right, dU/dV)`, sets `x1 = clip_right`.
       Clamps endpoint 0 using `mul16(x0 - clip_right, dU/dV)`, sets `x0 = clip_right`.
   - If `x0 <= clip_right`:
     - If `x1 <= clip_right`: entire edge is inside or on the boundary; returns immediately (`0x004BAEC8 = 0`).
     - If `x1 > clip_right` (inside-to-outside crossing): edge splits.
       Sets split flag `0x004BAEC8 = 1`. Saves original endpoint 1 `(x1, y1, u1, v1)` into `0x004BAECC–D8`.
       Intersection ratio DIV computes `((x1 - clip_right) << 32 / (x1 - x0)) >> 1`.
       **Ratio special case at VA 0x0064F44C:** if `x1 - clip_right == x1 - x0` (i.e. `x0 == clip_right`), `DIV` is skipped and ratio raw quotient is set to `0xFFFFFFFF` (`SHR 1` yields `0x7FFFFFFF`). This explicitly prevents an x86 divide error (#DE) when dividing equal 32-bit operands shifted into EDX:EAX.
       Updates endpoint 1 to the crossing intersection `(clip_right, y_interp, u_interp, v_interp)`.
       Endpoint 0 is untouched. Returns at `0x0064F488`.

## Recovered wrappers and right-endpoint clamp helper (RVAs 0x24EFA0, 0x24F0C0, 0x24F489)

- `Gfx_ClampTriangleEdgeRightEndpoint`, VA `0x0064F489` / RVA `0x24F489`, 63 bytes through RET at `0x0064F4C7`.
  ESI mutable edge packet. If `x1 > clip_right`: `u1 -= mul16(x1 - clip_right, dU)`, `v1 -= mul16(x1 - clip_right, dV)`, `x1 = clip_right`.
- `Gfx_BuildClippedCorrectedTriangleEdge`, VA `0x0064EFA0` / RVA `0x24EFA0`, 282 bytes through RET at `0x0064F0B9`.
- `Gfx_BuildClippedRawTriangleEdge`, VA `0x0064F0C0` / RVA `0x24F0C0`, 282 bytes through RET at `0x0064F1D9`.

Both wrappers take ESI=mutable edge packet, EBX=output buffer pointer; saves EBP; clobbers working registers.
1. Trivial rejection: if `y1 <= clip_top`, stores `[ebx] = clip_top_int`, `[ebx+4] = 0`, returns. If `y0 >= clip_bottom`, stores `[ebx] = clip_bottom_int`, `[ebx+4] = 0`, returns.
2. Segment 1: calls splitter `0x64F270`. Sets `[ebx] = startY = y0 >> 8` and `[ebx+4] = count1 = (y1 >> 8) - startY`.
   If `count1 > 0`: calls step initializer `0x64F4D0` and edge emitter (`0x64F220` for corrected, `0x64F1E0` for raw).
   Stores advanced record pointer at `0x004BAEB8` (corrected) or `0x004BAEE4` (raw).
3. Segment 2 (if split flag `0x004BAEC8 == 1`): replaces endpoint 0 with segment 1 endpoint 1; restores endpoint 1 from saved quartet `0x004BAECC–D8`. Calls clamp helper `0x64F489`.
   Computes `count2 = (y1 >> 8) - (y0 >> 8)`. If `count2 > 0`: adds `count2` to header count `[ebx+4]`, calls step initializer `0x64F4D0`, and calls edge emitter appending `count2` records contiguously after segment 1.

## Left-clipped spans and access order (RVAs 0x5C9D0, 0x67F97)

When `lx < clip_left_int` (at VAs `0x0045D3A0`, `0x0045D466`, `0x0045D620`, `0x0045D6E6` inside RVA `0x5D170`), the rasterizer calls the four-argument span wrapper at `0x0045C9D0` (`Gfx_DrawClippedTexturedSpanWrapper`), which invokes `0x00467F97` (`Gfx_DrawClippedTexturedSpan`).
- Four arguments: `destination = cursor + clip_left`, `count = rx - clip_left`, `packedUV = (r.V & 0xffff) | (r.U << 24)`, `Uinteger = r.U >> 8`.
- In `0x00467F97`: loops right to left from `count - 1` down to 0. **Access order per pixel is: texture read, then framebuffer read, then table lookup and store.**
- Unlike the unclipped span `0x00467F42`, it **does not sample or composite a cached left pixel** outside the viewport.

## Unified clipped rendering oracle and results

An independent analytical oracle in `tools/analyze_triangle.py` integrates all recovered clipping, splitting, wrapper, slope, gradient, and span contracts.

```powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
uv run --offline tools/analyze_triangle.py
uv run --offline tools/analyze_sprite_rasterizer.py
New-Item -ItemType Directory -Force build/tmp | Out-Null
$env:TMP = Join-Path (Get-Location) 'build/tmp'
$env:TEMP = $env:TMP
uv run python -m unittest discover -s tests -p test_db_candidates.py
uv run python -m unittest discover -s tests -p test_workflow.py
```

- **Direct splitter checks (62 cases):** authentic RVA `0x24F270` executed directly across both right-crossing directions, both endpoints outside right, exact-right equality, top then bottom clipping, fractional crossings in 24.8, degenerate edges, narrow intervals, and the ratio special case at `0x0064F44C`. Every single memory write address, length, and value matches the independent analytical derivation.
- **Direct wrapper checks (16 cases):** authentic RVAs `0x24EFA0` and `0x24F0C0` executed directly across inside, top rejection, bottom rejection, top clip, bottom clip, both split directions, and double-outside clamping. Output buffer headers, counts, 12-byte record sequences, two-segment continuation, and clamp helper `0x24F489` match identically.
- **Original-only unclipped triangles (312 cases):** 19,486 framebuffer byte stores match independent expressions; rendering read order, emitted edge records, and key persistent scratch verified.
- **Original-only clipped triangles (60 cases):** all 60 cases across four clip sides, combined clipping, and complete outside rejection in six vertex orders now match the independent clipped rendering oracle identically in final framebuffer bytes, ordered framebuffer stores (2,382 stores), and ordered memory accesses.
- **Original-only transformed sprites (24 cases):** all 24 outer-return transformed cases now match the unified rendering oracle through both triangle calls and outer return, carrying the framebuffer forward and comparing combined ordered framebuffer byte stores and memory accesses. Twelve cases produce stores in both triangles.
- **Fault/degenerate observations:** narrow collinear packet with U1=`0x7FFFFFFF` raises `UC_ERR_EXCEPTION` at IDIV VA `0x0064EB44`; coincident vertices with zero UV return normally. Total framebuffer stores across all oracle cases: 21,868.

## Integration boundary status and limitations

The clipping and splitting contracts are completely recovered and validated by independent analytical expressions. However, C89 reconstruction remains bounded:
- The outer ESI packet interface at `0x00465BB5` and the register ABIs across `0x24...` edge routines (ESI edge packet, EBX output buffer, EAX/ESI/EDX/EBX/ECX record emitter) are native register calling conventions lacking portable C89 representations without assembly.
- AGENTS.md strictly forbids handwritten assembly or inline `__asm`.
- Therefore this milestone completes as a bounded analysis feature. No production C, instruction comparison, or differential emulation is claimed for these routines.
