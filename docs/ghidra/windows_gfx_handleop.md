# Native Windows sprite handle operation

Fingerprint verified against IGN_WIN.EXE. VA 0x0045C830 / RVA 0x5C830,
207 bytes; FPO (207,0,2,0x202), SHA256
77a47f976fb496d9826b327cdf81fa38f527ee1a1fb3ce7085124b6ab39a0ec7.
Two caller-cleanup arguments (descriptor, handle), handle pointer return or zero.
RET at 0x45C84A, 0x45C85B, 0x45C8C7, 0x45C8D2, 0x45C8FE; one INT3 padding.
13 HIGHLOW operands and entry pointer at RVA 0x56EB6. No direct FPO callers;
indirect dispatch consumers require their independent ABI evidence. Two direct
calls at 0x45C8AD/0x45C8DE to real ImageOp 0x461360. Ghidra responding;
xrefs shows dispatch 0x456EB0 but function definition absent. No target conflict.

No handle: reject null descriptor or cursor == freelist base. Otherwise subtract
4 from cursor, reread cursor, pop pointer, zero image ID. Existing handle: read
image ID once; zero rejects regardless of descriptor. Null descriptor deletes
image through real ImageOp using captured ID, zeros handle ID, captures cursor,
stores handle there, then rereads/increments cursor 4; returns zero. No range,
ownership, overflow, heap failure or repeated-free protection is invented.

Creation/update zeros scratch word 0, copies only source words 1..5 in order,
preserves scratch words 6..15, rereads selected image ID and calls real ImageOp
(scratch, ID). Stores result in handle ID, then live reads source words 6/7 to
origins +4/+8. Returns selected handle even if ImageOp returns zero. Scratch or
handle aliases and child/heap mutation can change later reads. Creation already
pops the handle before any later fault and does not roll back. Scratch tail can
carry ownership state; do not sanitize it from DOS assumptions.

## Production and differential validation

Production C89 executes with real ImageOp and every resource/packing descendant;
the nonreturning HandleOp fixture is removed. Seven original-only probes were
preparatory evidence. `uv run python tools/verify_sprite_handleop.py` authenticates
fingerprint, FPO/hash, calls, relocations and dispatch consumer before compiling
the focused PE32 DLL. It runs 150 comparisons, including 75 persistent follow-ups,
15 packed calls, six aliases, four CRT mutation cases and 14 faults. Only CRT
malloc/free are modeled. A separate instruction-derived parent oracle composes
the independently authenticated ImageOp and resource contracts.

The suite compares ordered external reads/writes, actual child entries and
arguments, heap-boundary snapshots, full arena/static freelist/pool/scratch/
cursor/control/packing state, unrelated image bytes, normal pointer result,
caller stack, nonvolatile registers and DF. Cases include representative pool
positions at both bounds, multiple simultaneous handles and middle-ID reuse,
existing rejected signed IDs, cold/wrapping cursor behavior, scratch tail values,
source origins changed during allocation, cursor/handle mutations during cleanup,
whole scratch/pool/freelist aliases, real packed pixels and cleanup, and faults
after earlier pop/clear/publication. No rollback or invented ownership check is
added. Cross-page dword fault atomicity and fault-time frames/registers are excluded.

Harness static pointer words are relocated through independently verified object
bindings and normalized for comparison. Untouched random arena bytes are retained,
including numeric address collisions; newly written declared pointers normalize.
Scalar/address-collision cases and exhaustive aliases are outside this contract.
An alias fixture that extended beyond the known pool into unrelated workspace
storage was replaced by an alias wholly within the verified pool. This limits
harness coverage and does not change the production contract.

`verify_matching.py` checks the two production ImageOp calls and runs this suite
after all prior contracts. Full workflow completion renews 51 reconstructed
routines and updates SQLite/exports, LF working/index bytes, gates and hooks.
Native playable game, original compiler/link layout, instruction equality,
primitive/renderer integration, arbitrary reentry/concurrency and native heap/
fault behavior remain unverified. The focused DLL is not a playable executable.
