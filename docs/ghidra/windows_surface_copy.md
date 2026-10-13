# Windows byte-copy body and forwarding entry

Both routines are authenticated against the configured standard `IGN_WIN.EXE`.
VA is image base0x400000 plus RVA. Original source filenames are unknown;
reconstruction belongs to the directly compiled `surface` module.

| Routine | RVA / VA | Extent | SHA-256 |
| --- | --- | --- | --- |
| Gfx_SurfaceCopyPixelsNative | 0x5C1D0 / 0x45C1D0 | 497 bytes, no FPO; complete flow/RET0x45C3C0 and15 INT3 bytes before Clear0x45C3D0 | `ab0e1a366b4876c8807819b388b91435c556b4546a92764e400e2eee3ecd3c39` |
| Gfx_SurfaceCopyPixelsDispatch | 0x56B70 / 0x456B70 | 55 bytes; FPO(55,0,9,0) | `8bfc4e41d08ca864cab508cc3db8631f6f1dacef871033cbdfc54d9c23e8996a` |

The entry forwards nine cdecl stack words through live slot VA0x50EB80 and
caller-cleans36 bytes. Arguments are source bytes, stride, source x/y,
width/height, destination record and destination x/y. Width, stride and x offsets
are bytes; the body never reads bit depth. The real installer binds the body.

The body returns0 for destination.active0. It captures lock_mode; mode0 locks3,
mode1 unlocks then locks3, and any failed prerequisite returns0. Other modes
use the cached write pointer. Source is pixels+stride*source_y+source_x; destination
is write_pixels+captured pitch_bytes*destination_y+destination_x, with32-bit wrapping.
Pitch is read before the write pointer. After copying it rereads mode; if changed,
it unlocks and, for nonzero saved mode, relocks that mode, returning0 on failure.
Otherwise it returns1. Real production Lock/Unlock bodies supply those dependencies.

Aligned source and stride plus width divisible by160 choose the fast branch.
Each160-byte group first reads source DWORDs at0/32/64/96/128, then copies40
forward DWORDs. Group and row counters are unchecked do/while counts; zero and
negative values can copy and fault. Generic copying captures destination-leading
bytes, trailing=(destination+width)&3 and signed truncated middle DWORD count.
It uses those same counts on every positive-height row. Small widths/alignment
can produce negative middle counts and unbounded REP behavior in the original.
Generic nonpositive height skips copying but still restores access. Forward overlap
propagates already-written data; replacing it with memmove would change behavior.

Before implementation,320 successful original-only normal copies executed actual
access helpers with modeled SDK replies, preserving prefetch/copy order and ABI.
Five invalid-size originals established bounded fault prefixes. The earlier ignored
source+0x5000 fixture was outside its0x4000 arena; its claimed successful-copy scope
was withdrawn before this feature. Corrected source+0x3000 cases asserted result1
and no fault. Twenty-four actual original native copies independently checked both
branches and all four real lock modes,5,568 copied bytes and393,216 readback bytes.
Entry FPO/hash/argument cleanup were rechecked before production implementation.

Readable production C89 uses named surface fields, real SDK access bodies,
volatile byte/DWORD transfers and five explicit volatile prefetch reads. Wrapped
address/count arithmetic retains the original invalid-count behavior. DWORD access
depends on the verified i386 ABI, including ordinary unaligned loads/stores; this
is not a portable C or native fault-atomicity claim. No assembly or invented guards
are introduced. The focused builder removes the nonreturning copy boundary and
exports both reconstructed routines; remaining unresolved targets stay separate.

`uv run tools/verify_surface_copy.py` passes1,334 focused differential comparisons:
667 per routine,434 persistent followups,18 faults,72 fast and1,218 generic paths,
1,096 asserted normal copies,44 executed SDK-failure cases,16 buffer aliases and
18 callback mutation paths. All26 record banks and cached modes0/1/2/3/4/-1 are
tested. Independent source/destination alignment, negative/odd pitch, real helper
failure/restoration, inactive and unmapped inputs, caller stack/return/preserved
registers and unrelated image bytes are compared against original instructions.
SDK replies/output fields/mutations are explicitly modeled. Ten invalid-size tests
require an actual fault after a copied prefix; they do not manufacture native proof.

Boundary development corrected two verifier details: data hooks now include the
negative-pitch window below PIXELS, and an unaligned cross-page DWORD read reports
the first unmapped page. Original and C executions independently faulted at the
same address with the same286-byte prefix and full state. The oracle models that
reported address; no production behavior or trace assertion was relaxed.

Actual paired native helpers use owned64x64 32-bit SDK surfaces, supported pixel
mask seeds, actual body/installer/entry and real Lock/Unlock. Fifty-four copies
check11,328 copied bytes and884,736 destination/outside readback bytes, including
forward overlap. Both branches cover all four modes. Twelve inactive guards and
twelve generic zero-height calls pass with a null source and restored access.
Interfaces are disposed before authentic shutdown; original WndProc remains a
reference boundary. Native invalid geometry, lost surfaces, arbitrary record/stack
aliases, DF-set non-ABI callers, concurrent reentry and fault atomicity are unverified.

Full94-routine renewal, fidelity audit and exports pass at snapshot
`0d2549f92439de263c8fadc2b4a8c9c4a01b07d9db1db141361878ac64e38d44`, with94 fresh
compilation/emulation and33 scoped native records. LF working/index checks, staged
gates and tracked hooks are required before commit. Original compiler/link configuration remains unresolved;
the validation toolchain is provisional. Production frame integration, native
startup, menus, rendering, input integration, audio and representative races remain
open. Focused DLLs/SDK pixel tests are not a playable rebuilt game. Continue useful
unblocked rendering/startup dependencies immediately after the separate feature commit.


The first full attempt stopped in the strict direct-call audit because the newly
reconstructed copy dependency was absent from its contract. The contract now
checks the actual four ordered GCC symbolic call sites (Unlock, Unlock, Lock,
Lock), with both copy/blit code entries declared. The original has six sites;
GCC merges equivalent branches. Differential/native behavior passes; instruction
equality is not claimed. The failed attempt remains in `build/workflow/surface-copy-complete.log`;
the successful renewal is `build/workflow/surface-copy-renewal.log`.
