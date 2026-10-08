# Native sprite backend — authentic Windows recovery

Recovered 2026-10-07 from `Ignition/Ignition/IGN_WIN.EXE`, 915,968 bytes,
SHA-256 `7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
PE32 x86, preferred base `0x00400000`, CRT entry RVA `0x69950` / VA
`0x00469950`, application entry VA `0x004120A0`. The existing PE inspector
rechecks sections, imports, relocation directory and startup edges. No DOS
binary or original asset was changed or used as reconstruction evidence.

The localhost:8080 Ghidra bridge was available outside the restricted sandbox.
The user confirmed the selected authentic Windows program; this confirmation
supersedes the skill's independent loaded-program authentication requirement.
Read-only bridge disassembly, decompilation and xrefs informed bounded searches;
file disassembly, hashes, FPO and relocations corroborated the results. Ghidra
had no function defined at `0x004571B0`; its downstream `undefined8` return and
fastcall rasterizer prototype are hypotheses contradicted by the instructions.
No bulk synchronization or symbol mutation was performed.

## Extent, ABI and dispatch

The reconstruction is named `Gfx_DrawSpriteNative` for its observed role, not a
recovered PDB symbol. Preferred VA `0x004571B0`, RVA `0x000571B0`, 145 bytes,
extent `[0x004571B0,0x00457241)`. Authentic FPO tuple `(145,1,3,289)` agrees
with one local dword, three argument dwords and saved ESI. All 145 bytes decode;
the single epilogue ends in plain RET and 15 INT3 bytes precede `0x00457250`.
Routine SHA-256:
`f2fe1b07e3a37fe080d3cfcaeadad8e3bae3d2ca326e16c4a99e6ac563e01dac`.

```c
int Gfx_DrawSpriteNative(GfxSpriteHandle *handle, Point2D *position,
    const GfxSpriteTransform *transform);
```

Three-argument x86 cdecl: entry ESP+4/+8/+12 are handle, point and transform
pointers. EAX is always set to 1 after the downstream call returns. There are
no validation/error branches for handle or point, nor an allocation/ownership
operation. A non-null transform requires two readable float words.

The 25-byte dispatch wrapper at VA `0x00456D20` / RVA `0x56D20` forwards
all three words, calls through VA `0x0050EBBC`, cleans 12 bytes and returns EAX.
Hash `e04def2288d23041976ef0a043cea2b296558f74cf82cce58476bf765e804623`.
The zero-mode branch of `0x00456AF0` calls `0x00456E60` at `0x00456AFD`.
That dispatch initializer installs sixteen backend pointers, including
`[0x0050EBBC] = 0x004571B0` at `0x00456EA6`, before calls to `0x0045D840`,
`0x0045C9F0`, `0x0045C7F0`, and return 1. The installation's destination and
immediate have independent HIGHLOW relocations at RVAs `0x56EA8` and `0x56EAC`;
the wrapper's dispatch operand has one at RVA `0x56D31`.

A direct FPO-code scan found 53 wrapper call sites; it is not a complete
non-FPO inventory. Ghidra xrefs corroborated font, menu and HUD clients.
`Font_DrawText` calls at `0x0045681F`/`0x004569AE` pass NULL and ignore EAX.
The independently inspected non-null client at `[0x0040B7D0,0x0040B812)`
packs two raw argument words into a local two-float transform, shifts its X/Y
integer arguments left eight, and calls the wrapper at `0x0040B809`, cleaning
its local frame plus three call arguments. Scale is its fourth argument and
angle its fifth; there is no degrees conversion in either this caller or backend.
The x87 trig operands establish radians. Module placement in the existing
`geputget.c` follows its 2D sprite/font role; an original source filename is not
independently recovered from debug information.

## State layout and ordered behavior

All fields below are aligned dwords. Request storage and the rendering packet
are file-backed `.data`; the request begins as twelve zero bytes. The dispatch
pointer, active pointer and coefficient block lie in `.data`'s loader-zeroed
virtual tail: raw-backed data ends at RVA `0xBCE00`, whereas these fields start
at RVA `0x10EBBC` or higher. The float multiplier at VA `0x0047AED8` is file-backed binary32
65536.0 (`00 00 80 47`). No Watcom packing or register ABI was reused.

| Preferred VA | Layout / role |
| --- | --- |
| 0x004BA708 | request +0: borrowed point pointer |
| 0x004BA70C | request +4: borrowed sprite handle pointer |
| 0x004BA710 | request +8: coefficient pointer, NULL on untransformed path |
| 0x0050EC10–0x0050EC1C | four signed dwords, in the order cosine/sine/sine/cosine |
| 0x0050EBFC | active request pointer |

The handle's **consumed prefix** is image ID at +0, origin X/Y at +4/+8.
Its full allocation/size/ownership is not recovered. The point is two signed
dwords interpreted as coordinates with eight fractional bits, confirmed by
callers and the downstream arithmetic-shift instructions. The transform has
binary32 scale at +0 and radians at +4. Only these eight bytes are consumed;
there is no integer flags field. Scale becomes signed 16-fractional-bit state.

The backend stores position, clears the coefficient pointer, then stores handle.
For a non-null transform it computes:

1. `scale_dword = low32(_ftol(scale * 65536.0f))`.
2. `cosine = low32(_ftol(FCOS(angle) * signed(scale_dword)))`; write EC10.
3. `sine = low32(_ftol(FSIN(angle) * signed(scale_dword)))`; write EC14,
   then EC18, then publish EC10 at request +8, then write cosine at EC1C.
4. Publish the request address at EBFC, call `0x00457370` with **zero** stack
   arguments, overwrite EAX with 1 and return.

No coefficient bytes change on the null path. All four coefficients persist
after return, as do both published pointers. There is no state restoration or
post-call store. The point may be a caller's stack object: a persisted pointer
does not imply ownership or extend the object's lifetime. This system shares
mutable scratch state and is not intrinsically reentrant or thread safe.

The two positive sine entries are verified, including their consumption by
`0x00468C70`: signed IMULs read coefficients +0/+8 for X and +4/+12 for Y.
No conventional rotation sign correction is inserted.

`_ftol` executes at `[0x0046950C,0x00469533)`, saves the caller control word,
sets truncation, performs FISTP **qword**, returns its low/high dwords in EAX/EDX,
and restores the control word. The backend uses EAX only and subsequently FILDs
it as a signed dword. This wraps conversions beyond signed 32-bit range. Masked
invalid conversions produce signed-64 indefinite, whose low dword is zero.
The original FCOS and FSIN do no range reduction when |angle| >= 2^63: each
leaves its operand unchanged and sets C2. That behavior is preserved under the
validation compiler's separate instructions, rather than a corrected math library.

Caller precision and rounding apply to x87 multiply/trig results; `_ftol` changes
rounding only during integer conversion. The compiled backend uses long-double
intermediates and GCC x87 builtins, with an exact modulo-2^32 conversion macro
to avoid undefined C integer overflow. It preserves tested output/state/ABI,
but not the exact FP exception/status flag sequence: additional comparisons and
FPREM, and the volatile angle copy, can change flags. Unmasked traps and initial
nonempty x87 stacks are not validated. EXACT records behavioral fidelity in
the stated scope; it is not instruction equality or a universal native FP claim.

## Relocation operands

Exactly twelve backend HIGHLOW entries, asserted in full by the verifier.
Operand sites are **RVAs**; targets are preferred **VAs**.

| Operand RVA | Target VA |
| --- | --- |
| 0x571BC | 0x004BA708 |
| 0x571C2, 0x5721A | 0x004BA710 |
| 0x571CD | 0x004BA70C |
| 0x571DD | 0x0047AED8 |
| 0x571FD, 0x5721E | 0x0050EC10 |
| 0x5720F | 0x0050EC14 |
| 0x57214 | 0x0050EC18 |
| 0x57224 | 0x0050EC1C |
| 0x5722A | 0x0050EBFC |
| 0x5722E | 0x004BA708 |

## Independently recovered downstream contract

VA `0x00457370` / RVA `0x57370`: 167 bytes,
`[0x00457370,0x00457417)`, FPO `(167,16,0,267)`; hash
`203cc8648be172994ad229e423e9f05c83ff3a5d541277de7e9864ac47ae79fe`.
It takes no stack parameters, reserves 64 local bytes and preserves ESI.
Its direct body does the following, in this order:

- Read EBFC; copy request +0/+8 to VA `0x004BA77C`/`0x004BA784`.
- Cache handle=request +4 in ESI. Read handle +0 and call `0x004612E0`
  as cdecl `(local_descriptor_64_bytes, image_id)`, cleaning eight bytes.
- Read cached handle +4/+8 **after** the lookup, store at `0x004BA758`/`75C`.
- Read local descriptor dword +16. Write `(word & 255)<<8` at `760`,
  `word & 0xFF00` at `764`, `word & 0xFFFF0000` at `770`.
- Write `(descriptor[1]<<8)+[760]` at `768`, then
  `(descriptor[2]<<8)+[764]` at `76C`, with dword wrap.
- Copy VA `0x0063F2D8` to `774`, load ESI=`0x004BA778`, call `0x00465BB5`.

The packet at `0x004BA778` starts with file-initialized dwords
`(4, NULL, 0x004BA758, NULL)`. Its descriptor-pointer word has a HIGHLOW
relocation at RVA `0xBA780`; +4 and +12 are replaced with point/coefficient
pointers. This is an **ESI input contract**, not a two-argument fastcall.
The zero-argument C dependency declaration reflects only the backend-to-adapter
boundary; it does not invent a C ABI for the ESI rasterizer.

The lookup at VA `0x004612E0` / RVA `0x612E0` has a 114-byte FPO extent,
tuple `(114,0,2,520)`, hash
`75f04bc300b71fce757262cbb88c22e365cd785b8fc1a87578179dba1c442858`.
It copies sixteen dwords from fallback VA `0x0051FB88` when ID is zero,
negative, >= signed count at `0x00520388`, or table slot is null. Otherwise it
uses the pointer table at `0x0052038C`, copies 64 bytes, then clears local-copy
+24/+28. It changes no persistent descriptor/handle state. Fallback/table/count
are loader-zeroed storage whose initialization is outside this feature.
The adapter consumes descriptor +4/+8/+16; no full semantic descriptor type is
claimed from this small read set.

`0x00465BB5` starts with PUSHAD and restores registers with POPAD on both
observed dispatch paths. Consequently the adapter returns incidental EAX from
`0x0063F2D8` and EDX from handle +8, not a semantic 64-bit success result.
The backend ignores both. The untransformed rasterizer branch reads point
coordinates, prepared descriptor, clipping state at `0x0063F2B4–0x0063F2C0`,
stride at `0x0063F2C8` and framebuffer pointer `0x0063F2D4`, writes scratch
`0x004BAD28–0x004BAD30`, and performs table-based framebuffer writes. The
transformed branch writes request scratch `0x004BAD38–0x004BAD44`, calls
`0x00468C70` via ESI, builds transformed polygon scratch and calls `0x0045CA50`
twice. These transitive rasterizer bodies, clipping/data contracts and framebuffer
effects are **not** reconstructed or emulated here. Modeling the recovered
`57370` boundary does not claim those effects are absent.

The 28 original-only adapter experiments execute real `57370` and `612E0`
instructions, stopping before `465BB5`. They check every ordered persistent
adapter store, all fallback/valid/null-slot lookup paths, local-copy clearing,
ESI packet pointer, incidental return registers, stack, preserved registers,
and unchanged request/handle/point/descriptor storage. This corroborates the
contract before the backend verifier intercepts it. Neither downstream routine
is promoted to reconstructed tracking status.

## Production compilation and validation scope

Run `uv run tools/verify_sprite_backend.py`, or the integrated
`uv run tools/verify_matching.py`. The harness extracts the actual production
globals and backend from `decomp/src/geputget.c`, includes its production header
and checks structure sizes/offsets. It does not keep a duplicate implementation.
Full `geputget.c` compilation remains blocked by unrelated legacy includes and
dependencies. The unrelated `globals.c` sprite stub is never linked.

GCC 16.2.0 compiles strict C89 32-bit Windows COFF; LLD 19.1.1 links a focused
DLL with `/noentry /nodefaultlib /safeseh:no`. This provisional behavior compiler
does not establish the original Microsoft-compatible compiler/version. x87
flags include `-mfpmath=387 -mno-sse -mno-sse2 -fno-math-errno` and unsafe-math
lowering with reassociation, reciprocal transformations and finite-only assumptions
disabled, and signed zeros enabled. The verifier rejects external imports,
unexpected calls and FSINCOS contraction; compiled FCOS/FSIN execute directly.
No handwritten assembly is present. A separately compiled infinite-loop trap
marks the downstream external symbol; it is intercepted and never executes.

2,588 original-versus-C cases passed. Null cases include arbitrary/null
handle/point words (the backend publishes without dereferencing; actual downstream
consumption requires readable pointers). Non-null cases cover all twelve valid
masked precision/rounding combinations, negative/zero/unit/fractional/extreme
scales, signed-32 wrap, signed-64 invalid conversion, adjacent float boundaries,
subnormals, NaNs/infinities, tiny/quadrant/large/out-of-range angles and 160 seeded
raw-float pairs. Original wrapper execution is included for representative null
and non-null calls. Each case compares ordered dword stores, published snapshots,
modeled downstream mutations, return value, caller arguments/stack, saved
registers, DF, x87 control word/stack balance and unchanged input/image regions.

Original `_ftol` executes in differential cases. The real original dispatch
wrapper executes in wrapper cases. Only the separate original-only experiments
execute `57370`/`612E0`; backend differential cases intercept `57370` and vary
its EAX/ECX/EDX and explicit shared-state mutations. No rasterizer body executes.
Compilation and differential emulation pass separately; raw bytes differ.
Relocation-aware instruction equality, native DLL/game execution, framebuffer
output, visual parity and the playable reconstruction remain unverified.

The next evidence-supported candidate is VA `0x00457370` / RVA `0x57370`,
whose direct adapter contract is recovered here. Reconstruct it with its native
ESI dependency interface and independently validate further rasterizer contracts
before claiming rendering output. No broader renderer migration was performed.
