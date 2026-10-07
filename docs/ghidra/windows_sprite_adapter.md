# Native sprite preparation adapter — analysis and integration blocker

Recovered 2026-10-07 from the authenticated standard Windows release,
`Ignition/Ignition/IGN_WIN.EXE`, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
915,968 bytes, PE32 x86, preferred image base `0x00400000`.
The target doctor passed in this session. Hooks were installed and workflow
preflight declared the affected candidates and explicit feature paths.
Original binaries/assets and `tools/dump_456470.py` remain untouched.

The localhost Ghidra bridge is reachable outside the sandbox. The bundled bridge
API has no program-fingerprint operation; bounded read-only identity queries
(`program`, `program_info`, `get_program_info`, `getCurrentProgram`,
`list_programs`) all returned HTTP 404. Loaded-program identity could not be
established in this session. No Ghidra disassembly, guessed program identity,
bulk synchronization or symbol edits are claimed. Direct fingerprinted PE bytes,
Capstone, authentic FPO records, HIGHLOW relocations and Unicorn executions
corroborate the inherited [backend contract](windows_sprite_backend.md).

## Adapter contract

Analysis name `Gfx_SubmitSpriteRequest` describes its role, not a recovered symbol.
VA `0x00457370` / RVA `0x57370`, extent `[0x00457370,0x00457417)`,
167 bytes; routine SHA-256
`203cc8648be172994ad229e423e9f05c83ff3a5d541277de7e9864ac47ae79fe`.
FPO `(167,16,0,267)`, complete decoding through RET and nine following INT3 bytes
corroborate its boundary. It reserves 64 stack bytes and saves/restores ESI;
there are no stack arguments and no semantic return contract. The backend
overwrites EAX with 1 after this call.

The packet is four aligned dwords at VA `0x004BA778`, initialized from file-backed
`.data` as `(4,0,0x004BA758,0)`. The pointer at packet +8 has a HIGHLOW relocation
at RVA `0xBA780`. Prepared storage at `0x004BA758` comprises eight dwords;
no larger semantic descriptor layout is claimed. Active request, descriptor
fallback/table/count, and render word `0x0063F2D8` are in loader-zeroed `.data`
tails; later initialization remains unverified. Packing is independently observed
as aligned dwords, without assuming DOS layouts or the legacy `SpriteDesc` type.

Ordered effects recovered from the Windows instructions:

1. Read active request `0x0050EBFC`; copy request +0 to packet +4 and request
   +8 to packet +12. Cache request +4 (handle) in ESI.
2. Read cached handle +0 (image ID); call `0x004612E0` with cdecl arguments
   `(local_64_byte_destination,image_id)` and clean eight bytes.
3. Read the cached handle origins +4/+8 **after** lookup; store them at
   `0x004BA758`/`75C`. Changing request/active pointers during lookup does not
   replace the cached handle or already copied packet pointers.
4. For descriptor dword `d` at local +16, store `(d & 255)<<8` at `760`,
   `d & 0xFF00` at `764`, then `d & 0xFFFF0000` at `770`.
5. Store `(descriptor_dword_1<<8)+[760]` at `768`, then
   `(descriptor_dword_2<<8)+[764]` at `76C`, with unsigned dword wrapping.
6. Copy live `0x0063F2D8` to `774`; load ESI=`0x004BA778` and call
   `0x00465BB5`. Restore ESI, release locals and return only after that call.

The reproduction asserts all fifteen adapter HIGHLOW operand RVAs and targets:
`57374→50EBFC`, `5737D→4BA77C`, `57386→4BA784`, `573A2→4BA758`,
`573B6→4BA75C`, `573BE→4BA760`, `573CA→4BA764`, `573DD→4BA760`,
`573E3→4BA770`, `573E8→4BA768`, `573F5→4BA764`, `573FA→4BA76C`,
`573FF→63F2D8`, `57404→4BA774`, `57409→4BA778`.
Left sides are RVAs; right sides are preferred VAs.

## Descriptor dependency

Analysis name `Gfx_CopyImageDescriptor`, VA `0x004612E0` / RVA `0x612E0`,
114 bytes, `[0x004612E0,0x00461352)`, FPO `(114,0,2,520)`;
SHA-256 `75f04bc300b71fce757262cbb88c22e365cd785b8fc1a87578179dba1c442858`.
Two-argument x86 cdecl, preserves ESI/EDI, no local frame. Signed ID zero/negative
or ID >= signed count at `0x00520388` selects fallback `0x0051FB88`.
Otherwise it indexes the table pointer at `0x0052038C`; null slot also selects
fallback. It copies sixteen dwords using REP MOVSD. Only the non-null table path
clears destination +24/+28 after the copy. It never clears those fallback words,
changes persistent descriptors, or mutates a handle. It assumes the ordinary
x86 ABI's cleared direction flag; DF-set behavior is outside these experiments.
Return EAX is incidental (zero on valid path, input ID on fallback paths).
Aliasing destination with source/table is not tested or promoted to C fidelity.

## Reproduction and actual scope

```powershell
$env:UV_CACHE_DIR = Join-Path (Get-Location) 'build/uv-cache'
uv run --offline tools/analyze_sprite_adapter.py
```

The script pins the same pefile/Capstone/Unicorn versions as the existing verifier.
It reuses target authentication, image mapping and original static assertions;
it does not change production source or the existing differential verifier.
168 original-only cases execute real adapter and descriptor lookup instructions.
Six signed counts, seven IDs, null/non-null table slots and two mutation modes
cover all lookup paths, local-copy clearing, all ten ordered persistent stores,
wrapped extent arithmetic, ESI packet pointer, saved ESI stack slot, callee saves
at the boundary, call return addresses and stack depth. Random descriptor bytes
exercise packed fields and wrapping; point/coefficient pointers are copied only.
Mutation hooks after real lookup change cached-handle origins and request/active
pointers: origins change, while the earlier packet pointers remain captured.
These hooks are diagnostic perturbations, not claimed original lookup behavior.

Execution stops **before** the first rasterizer instruction. There is no
success-returning substitute, synthetic rasterizer return, adapter epilogue test
or assertion that rasterizer effects are absent. Image writes outside the declared
adapter stores and deliberate mutations are rejected. Persistent source descriptor
bytes and the complete local copy are checked. The ignored build report is
`build/decomp/windows/sprite_adapter_analysis.json`; disassembly/probe artifacts
also remain under `build/`. No adapter compilation/differential test result is
inserted into the database.

Three pure-C ABI diagnostics compile with provisional GCC 16.2.0, `-m32
-std=c89 -pedantic-errors -O0 -fno-optimize-sibling-calls`, linked by LLD as a
focused diagnostic DLL. Unicorn observes cdecl packet at ESP+4, fastcall packet
in ECX, and GCC regparm(1) packet in EAX. In each case ESI retains the unrelated
sentinel `0x13579BDF`. Generated C uses no assembly. These are compilation and
calling-convention diagnostics, not original-versus-reconstructed adapter tests.
The report records full commands and actual compiler version.

## Concrete blocker and next scope

The existing C89 toolchain/declarations have no verified way to place the packet
in ESI at the native rasterizer boundary. A conventional pointer parameter passes
it elsewhere; a zero-argument call does not establish ESI. Compiler allocation by
chance is not an ABI guarantee. Register-variable bindings/assembly shims would
violate the no-handwritten-assembly requirement. The diagnostics cover the three
candidate conventions above, not every possible compiler extension or compiler.
There is no claim that a faithful C reconstruction is intrinsically impossible.

Replacing this call with a model would exclude real rasterizer scratch/framebuffer
effects and cannot establish complete adapter fidelity. Reconstructing lookup
alone does not resolve the call. Therefore both records remain **analyzed**, with
unknown implementation fidelity and no reconstruction promotion. A production C
body and extension of the production differential verifier are blocked here.

The next evidence-supported recovery is rasterizer entry VA `0x00465BB5` /
RVA `0x65BB5`, observed 437-byte body with PUSHAD/POPAD and ESI input. Its
transformed branch reaches VA `0x00468C70` (631 bytes), which calls
`0x0045CA50`; clipping, scratch, framebuffer and those transitive contracts need
independent recovery before selecting a C integration boundary. Those routines
are outside this bounded feature. Original compiler identity, instruction equality,
rasterizer/framebuffer output and native game runtime remain unverified.
