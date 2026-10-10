# Windows graphics lifecycle entry points

Target: authentic `IGN_WIN.EXE`, SHA-256
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`,
PE32 i386 image base VA `0x00400000`. Evidence below is from local file-backed
PE instructions and FPO. Ghidra currently refuses connections; the request to
reopen the target with its MCP plugin is pending. No MCP response is claimed.

| Routine | RVA | VA | FPO extent | Routine SHA-256 |
| --- | --- | --- | --- | --- |
| Gfx_Open | 0x56BC0 | 0x00456BC0 | 19 | 52be6802ced83d2ed0f4d075befb6cb8a27c422283385d5948efbf9807fee826 |
| Gfx_Rebuild | 0x56BE0 | 0x00456BE0 | 6 | 38113c56bd5b8bad3c416585d5886f408e4c6e8aa5712adc29541c69099eaac4 |
| Gfx_Shutdown | 0x56BF0 | 0x00456BF0 | 19 | 12f76bae9f55a6539d23baf465bf52e0b2450cf8cc41121eb16f100448679412 |
| Gfx_SpriteOpenNative | 0x56F20 | 0x00456F20 | 6 | 2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67 |
| Gfx_SpriteShutdownNative | 0x56F30 | 0x00456F30 | 6 | 2db31f4e09597946e56e859813bfdad7046d457c32332c3a882654d1e3ffdf67 |

All five routines are cdecl, no arguments, full EAX result; FPO records have
zero locals, parameters and flags. Open calls the pointer at VA `0x0050EB6C`;
zero returns zero, any nonzero value tail-calls the live pointer at `0x0050EBA0`.
Shutdown does the same with `0x0050EB74` then `0x0050EBA4`. Rebuild jumps through
`0x0050EB70`. Both sprite leaves are exactly `B8 01 00 00 00 C3`, returning one.
The historical sprite Reset name is corrected to Shutdown by caller behavior.
The already authenticated installer RVA `0x56E60` binds these two sprite slots;
its source binding names change, but their addresses and stored targets do not.

The production bodies use the actual volatile dispatch slots and reconstructed
surface constructor/rebuild/shutdown dependencies. The builder removes the two
nonreturning sprite fixtures. C89 compilation passed after implementation.
`uv run tools/verify_graphics_entry.py` passes 525 original-versus-C comparisons:
309 standalone and 216 real dependency executions. These include 47 faults and
140 persistent follow-ups. All five candidates are reconstructed after these
actual results. The full workflow renews all 70 reconstructed routines, with
70 fresh compilation/emulation and nine scoped native records. Audit and
synchronized exports pass at snapshot `5ed35d7ce096`; all 45 verifier inputs
have identical LF working/index bytes. No raw/instruction/linked equality result
is recorded.

Disposable preparation under `build/workflow/` contains the bounded capture,
`graphics-entry-original-probes.json` and the probe script. All 120 original-only
callback cases pass, including full-width replies, zero short circuit, live
tail-slot replacement and cdecl/nonvolatile/DF preservation. Callback replies
are explicit test boundaries, not production substitutes. The committed verifier
independently authenticates hashes/FPO and compares arbitrary EAX results, null
and invalid target fetches, live tail replacements, zero short circuit, ordered
effects, all banks/globals/palette buffers, unrelated image bytes, caller bytes,
nonvolatile registers, direction flag and cdecl/SDK stack cleanup. Real dependency
sequences span modes 0/1/2/FFFFFFFF and counts 0/1/2/4/5/FFFFFFFF, constructor
API failures, repeated rebuild/shutdown and invalid DirectDraw pointers.

The native host entry variant passes four paired scenarios: missing class and
registered windowed 0/1/4 backbuffers. It executes the actual surface installer,
eight Open calls, twelve Rebuild calls, six Shutdown calls and six calls of each
sprite leaf through real original/rebuilt entry points. The host binds only the
two sprite lifecycle slots to their verified real bodies; the allocating sprite
installer is outside this native scope and remains independently covered by the
existing differential contract. The native checks retain type2 boundary releases,
repeated clearing and actual SDK queries proving primary/type1 interface retention.

Core verification audits the new code boundaries and orchestrates this contract.
The provisional compiler remains strict C89 Clang/LLD with GCC for SDK/x87 units.
Full raw/relocation-aware instruction equality is not claimed. Target function
hashes certify provenance, not rebuilt code equality.

Native full game behavior and the original compiler/link configuration remain
unverified. The native surface host still uses original WndProc as a reference
boundary. Startup inspection also finds an unresolved input-init stack-argument
anomaly on WndProc's Alt-Enter path; that must be recovered independently.
