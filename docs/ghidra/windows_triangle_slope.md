# Windows triangle-edge slope

Authenticated standard IGN_WIN.EXE, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Gfx_ComputeTriangleEdgeSlope is an analysis name, not a recovered linker symbol.
RVA 0x5D7A0 / VA 0x45D7A0, 53 bytes through RET at 0x45D7D4, followed
by eleven INT3 bytes. FPO tuple (53,0,4,261); routine SHA-256
`fee5ce3932998c6905816c67a42acfbc80612edfa7abc7d724cc90fec87f3bb2`.
Bounded Ghidra disassembly corroborates fingerprinted local PE bytes; callers
are 0x45CD04/29 in the ordinary triangle and 0x45D1FC/221 in its clipped path.
The active authentic Ghidra program is the user-provided operating assumption.

Four-argument i386 cdecl (x0,y0,x1,y1), return EAX, preserves EBX/ESI/EDI/EBP.
For equal y words it returns 0x80010000 when signed x1<x0, otherwise 0x7FFF0000.
Otherwise it wraps x0-x1, shifts its low32 result left eight, wraps y0-y1 and
performs signed32 IDIV with truncation toward zero. It does not compute a
mathematical wide numerator. Quotient overflow is an actual CPU divide exception;
no saturation or safe substitute is added. There are no callees or persistent writes.

Before implementation, 1,025 original-only executions checked independently derived
quotients/sentinels, unchanged image and arguments, ordinary ABI and one actual
overflow at IDIV VA 0x45D7D1. Eighty-four original native cdecl calls checked
20 equal-Y sentinels and 64 signed/wrapped normal quotients. Original assets were
untouched; checked disposable PE copies were mapped under build/runtime.

Readable production C89 in the directly compiled renderer module uses unsigned
subtraction/shift before verified i386 signed casts/division. No shared state,
assembly, new dependency or behavior correction is introduced. Original filenames
and compiler/link configuration remain unknown. Cast interpretation and signed
division overflow are validated for the provisional i386 compiler, rather than
claimed as portable C semantics or a guarantee for arbitrary optimization settings.

`uv run tools/verify_triangle_slope.py` passes 1,025 original-versus-C calls,
20 sentinels and one genuine IDIV CPU exception in each binary. Each successful
call also matches an independent mathematical oracle. Image/state/arguments are
unchanged; return, ESP/EIP, preserved registers and clear direction flag are checked.
Actual exception instructions are decoded and required to be IDIV, rather than
conflating memory faults or synthesizing failure returns. Fault stack layout,
unwinding and flags are not claimed equal. The whole renderer links import-free.

Actual paired native validation passes 84 cases / 168 cdecl calls: real original
instructions and the complete C89 DLL both match independently derived expected
words. It reuses the checked existing HIGHLOW mapper in an explicit generated
test host, invokes no original imports and validates input copies/artifact hashes.
Native exceptions/SEH, complete triangle rendering, startup, menus, input, audio
and races remain unverified. These focused DLLs are not a playable rebuilt game.

Full 106-routine renewal, fidelity audit and synchronized exports pass at snapshot
`ba3d21e1a6d13b167c538639cee657db57eb69d53b564c8760adf1fdb5012e0f`, with 106 fresh compilation/emulation and 43 scoped native records. All 48 tracking/
ownership/workflow tests pass. LF/index gates and tracked hooks are required
before commit.
After its separate commit, continue real edge setup, gradients and clipping,
carrying original arithmetic faults and actual shared-state dependencies.

Next-dependency investigation: the current GCC32 wide-integer diagnostic links
fail on unresolved `___divdi3`/`___udivdi3` against the installed x64 libgcc
archive. Clang32 instead needs `__alldiv`/`__aulldiv`; those are absent from
inspected SysWOW64 msvcrt exports. This blocks the current wide-division lowering
route for edge initializer RVA 0x24F4D0, without proving C89 reconstruction
impossible. Original toolchain remains unresolved. A separate signed32-by32
product/shift diagnostic compiles and links import-free, so next pursue the real
right-endpoint clamp RVA 0x24F489, then return to verified division/runtime recovery.
Ignored diagnostics remain under build/workflow; no fake arithmetic callee is added.
