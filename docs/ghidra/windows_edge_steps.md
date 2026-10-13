# Windows edge increments and first samples

Authenticated standard IGN_WIN.EXE, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Gfx_InitializeTriangleEdgeSteps is an analysis name, not a recovered linker symbol.
RVA 0x24F4D0 / VA 0x64F4D0, 199 bytes through RET at 0x64F596; no FPO.
SHA-256 `26ecf3f443aeac09a0baa2abe2104e6a883bf3b7d9c45d1fa3c20272dc944383`.
Fingerprint-pinned local instructions and bounded Ghidra disassembly establish
provenance under the user-provided active authentic-program assumption. The PE
code section lacks the execute flag; native NX implications remain unverified.

Original ESI points to the eight-word edge packet; no stack arguments or semantic
return. EBX/ESI/EDI/EBP are preserved; EAX/ECX/EDX and flags are scratch. The helper
captures wrapped dy=y1-y0. For unsigned dy<=64 it reads wrapped dx=x1-x0; unsigned
absolute dx<=65536 changes dy to65, otherwise dy<=3 becomes3. NEG of signed32
minimum retains its high bit; no corrected mathematical absolute value is inserted.

V and U quotients use an unusual signed paired numerator: high word is the sign
of the wrapped texture delta, while low word is delta<<16. The high word is not
delta>>16. X uses high=SAR(dx,16), low=dx<<16. Each signed IDIV returns a signed32
quotient or raises actual #DE on overflow. Persistent store order is V increment
at 0x4BAEB0, U increment at 0x4BAEAC, X increment at 0x4BAEC0. Earlier stores
remain when a later division faults.

Fraction=256-(y0&255). Initial U at 0x4BAEA4 is wrapped (u0<<8)+SAR(low32(fraction*
Uincrement),8); initial V at 0x4BAEA8 follows the corresponding expression.
Initial X at 0x4BAEBC is wrapped (x0<<8)+fraction*SAR(Xincrement,8). Packet fields
are reread at their actual points, so aliases with these stores can affect later
calculations. All six outputs persist; the normal packet is not explicitly written.

The verified state now covers 32 bytes beginning 0x4BAEA4: initial U/V, U/V
increments, preserved intervening word, advanced corrected record pointer,
initial X and X increment. All begin as authenticated file-backed zeros. The
pointer is established by the already analyzed corrected edge wrappers and is
retained by this initializer. Expanding the existing object preserves physical
field distances and removes anonymous storage for newly recovered fields.

Before implementation, 512 independent original-only expression cases passed,
including 318 real IDIV faults, 57 dy65 adjustments, 150 dy3 adjustments and
1,733 ordered stores. Installed GCC32 wide operators require unresolved runtime
helpers; Clang's alternative helpers were unavailable in the inspected CRT.
Rather than link a fabricated callee, an import-free C89 paired-word restoring
division prototype was compared against the authentic IDIV instruction in 2,200
cases with 102 faults over the initializer numerator forms. An out-of-domain
signed64 MIN/-1 direct probe terminated its interpreter process; it is unreachable
in this initializer and excluded explicitly. Fresh engines and actual original
instruction advancement are required. These diagnostics are not production evidence.

Complete production C89 uses captured words and restoring division directly,
without wide types, external arithmetic callees or assembly. Invalid quotients
reach an actual i386 IDIV exception under the verified compiler; this is not a
failure-return model or portable C exception guarantee. DEV-007 records the typed
packet interface and internal lowering. The peculiar numerator, height adjustments,
wrapping, ordered stores and live packet reloads are retained without a bug fix.

`uv run tools/verify_edge_steps.py` passes 1,314 whole original-versus-C calls,
740 actual divide/pointer faults, 605 shared-state packet aliases, 287 persistent
followups and 4,784 ordered stores. Both full bodies execute. Ordered non-stack
reads/writes, every arena/state byte, unchanged unrelated image and arguments,
return/ESP/preserved registers/DF on success and actual fault prefixes are compared.
Exceptions must be real IDIV instructions; no synthetic failure return is used.
The full renderer links import-free with exactly the three real span-to-core calls;
the initializer introduces no callee. Native initializer/parent, flags, fault stack
and unwinding equality remain unverified. Existing 896 span comparisons/64 paired
native cases, 1,025 slope comparisons/84 paired native cases, 1,028 clamp comparisons
and renewed 416 emitter comparisons pass separately after the state expansion.

Full 108-routine renewal, fidelity audit and synchronized exports pass at snapshot
`0bfc0061763a74a9ebfafb5c9fb949b0a08d2a0f8bc4d8f30bd9a9d042953ac3`, with 108 fresh compilation/emulation and 43 existing scoped native records.
All 48 tracking/ownership/workflow regression tests pass. LF/index gates and
tracked hooks are required before commit; no new native initializer record is manufactured.
Existing emitter bodies and setup core are renewed for layout/source context;
no new native initializer result is claimed. Continue real clipped-edge wrappers,
splitter and gradient/triangle/sprite integration. Original toolchain and playable
Windows startup/menu/render/input/audio/race validation remain unresolved.
