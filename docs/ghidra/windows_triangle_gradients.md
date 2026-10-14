# Windows triangle texture gradients and correction tables

Authenticated standard IGN_WIN.EXE SHA256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Gfx_ComputeTriangleGradients is an analysis name, not a recovered linker symbol.
RVA0x24EA80 / VA0x64EA80 spans380bytes through RET0x64EBFB. Hash:
`83c0abc6600657871b1ca05baeb39fc3977b6e154ebcfe7578ce339495cfb7aa`.
Local pinned bytes and live bounded Ghidra instructions corroborate provenance
under the user-provided active-program assumption. Original source filenames and
compiler remain unknown; renderer owns directly compiled readable strict C89.

No arguments/callees exist. Original clobbers EAX/EBX/EDX, preserving
ECX/ESI/EDI/EBP. DEV-007 gives an internal C89 no-argument function; incidental
register outputs and flags are not claimed. The shared48byte input at VA4BAD74
contains three interleaved signed X/Y pairs followed by three U/V pairs. All
fields, gradients, correction tables and32byte scratch are file-zeroed. Input
population/sorting is owned by the still-pending whole triangle parents.

Store wrapped Y1-Y0 and Y2-Y0. Stored zero second height becomes1, but comparison
uses the originally captured second height against the live first height.
Equal heights choose ratio7FFFFFFF; otherwise unsigned DIV(height01:0,stored
height02) then SHR1. Store ratio and deltaX01, then signed high products of
wrapped doubled X/U/V deltas and ratio. Store denominator interpolatedX-deltaX01;
values-2..2 map via SAR2/OR1/SHL2 to-4 or+4. No conventional area replacement.
Each UV difference is wrapped before constructing highSAR16/lowSHL16 for signed
IDIV. This differs from the initializer's sign-only high-word numerator.
Publish U gradient and its entire correction table before Vdivision. Each table
is0,step,then six wrapped additions; step is signed(wrapped negative gradient)
SAR3. Division overflow faults preserve earlier scratch/gradient/table stores.

Verified scratch VA4BADF4..4BAE10 contains heights01/02,denominator,deltaX01,
interpolatedX/U/V andratio. Existing gradient and table ownership is reused;
no duplicate backing or invented runtime initializer is introduced. Halfword
signed high products and paired-word restoring DIV/IDIV retain CPU arithmetic
without wide types, runtime helpers or assembly. Invalid divisions reach actual
i386 faults under the verified provisional compiler; arbitrary compiler C
exception semantics, fault-stack/SEH/unwinding are not certified.

Before production application,256 normal,108 boundary and1,024 full-word
original-only executions checked independent arithmetic, all scratch/table words
and exact ordered store/fault prefixes; boundary/wide cases include54/462 actual
DIV/IDIV faults. A disposable complete strict-C89 draft passed before application.
`uv run tools/verify_triangle_gradients.py` now passes2,260 whole original versus
production C calls over1,388 input packets:872 persistent followups and516 actual
divide faults. Exact ordered non-stack reads/writes, complete arena and nine
shared-state objects, unchanged unrelated image/caller bytes, normal ESP/DF,
original ECX/ESI/EDI/EBP and compiled cdecl preservation compare. Original inputs
include zero/equal/reversed heights, tinydenominators and arbitrary full32bit
coordinates/UVs. Actual DIV/IDIV instructions are required on faults, with all
partial stores compared. The complete renderer is import-free and its19 actual
calls remain verified production helper targets; gradients add no callees.

Full114 compilation/differential renewal, fidelity audit and synchronized exports
passed at committed snapshot `168c99a0c897682abc28550c414ce3c829e2205406c16c4f9e1d2b28df3a03a2`,
commit `a1b6590`. The committed inventory records114 fresh compilation/emulation
and43 scoped native passes; no native gradient result was added. No native gradient/parent
or playable game result is claimed. Existing43 scoped native records remain
separate. Original NX behavior, original toolchain, concurrency and complete
startup/menu/render/input/audio/races remain unresolved. Next integrate real
triangle parents, viewport setters and directly migrated workspace ownership,
then validate native triangle frames and sprite consumers. Goal remains active.
