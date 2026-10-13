# Windows surface BltFast body and forwarding entry

The authentic standard PE fingerprint is the configured
`7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`.
Ghidra MCP responds under the user-provided loaded-target assumption; the entry
leaf is not defined there. Local FPO, complete instruction decoding and hashes
authenticate both routines independently. Original source filenames are unknown.

| Routine | RVA / VA | Bytes / FPO tuple | SHA-256 |
| --- | --- | --- | --- |
| Gfx_SurfaceBlitNative | 0x5C160 / 0x45C160 | 108 / (108,4,8,524) | `e716f58cbe637634909a5de39555b22a4f99b391b14673f17879ecf036ccdde2` |
| Gfx_SurfaceBlitDispatch | 0x56B30 / 0x456B30 | 50 / (50,0,8,0) | `299c846485b18251d0a57631f64aba15d902a277fc877bf5465eb08e5efa2689` |

Both take eight ordinary C stack arguments: source record, source x/y,
width/height, destination record, destination x/y. The forwarding entry copies
all eight arguments to the live dispatch target at VA0x50EB7C and caller-cleans
32 bytes. The verified installer0x5B690 stores the body in that slot. Direct
calls were not found in FPO-decoded callers; indirect/non-FPO consumers are not
excluded, and production frame integration is not certified by that search.

The body first reads source.active, then destination.active, returning0 for either
zero. Other bit patterns, including2 and-1, proceed. It builds a source RECT with
32-bit wrapped sums, reads destination.surface at offset44 before source.surface,
and calls the real IDirectDrawSurface::BltFast vtable slot0x1C with destination
x/y, source interface, source RECT and flags0x10 (DDBLTFAST_WAIT). It makes one
call without a wrapper retry, returning1 only for HRESULT0. Source and destination
records can alias. No invented null-pointer, coordinate or dimension guard is added.

Before production implementation,9,990 original-only raw-offset comparisons and
160 body/forwarder cases authenticated API arguments, RECT/flags, callback state
and ABI;72 mapped API paths were asserted. Twelve actual original SDK copies in
three windowed constructor scenarios checked49,152 destination pixels. This proved
argument7 is horizontal and argument8 vertical, correcting earlier declaration
names without changing their types. The source seed follows actual SDK format
masks; an initial unsupported high-byte expectation on32-bit pixels failed and
was corrected before recording paired validation. All destination bytes, including
the area outside the rectangle, remain checked.

Readable production C89 resides in the declared direct `surface` module. It uses
verified record fields and actual SDK COM declarations. Unsigned sums preserve
the original wrapped RECT coordinates; volatile record views retain ordered
reads. The forwarding entry uses the real volatile dispatch slot. The shared
builder removes its nonreturning Blt fixture and exports both bodies. A production
SDK offset assertion binds BltFast to0x1C; original translation-unit names remain
null in the ownership manifest. The helper builder now reads both C inputs with
explicit UTF-8. No original binary or asset is modified.

`uv run tools/verify_surface_blit.py` runs an independent raw-offset oracle against
original and freshly compiled C89 execution. Focused20,002 comparisons pass:
10,001 per routine,9,986 persistent followups,12 unmapped record/interface faults,
12,596 asserted API calls,7,168 source/destination aliases and two actual callback
mutation paths. It checks all26 source banks, three destination choices, nonzero
active patterns, success/nonzero/failure/busy HRESULTs, wrapped/reversed/empty
rectangles, preserved registers, stack/return and unrelated image bytes. Both
record pointers are relocated for C execution. Real installer execution and
normalized target identity are checked; only those verified relocations normalize.
SDK replies and mutations are explicitly modeled in differential execution.

The separate native helper creates test-owned64x64 SDK surfaces through actual
paired windowed0/1/4 constructors, observes format/pitch/masks, seeds RGB bytes and
invokes original/C body and real installed entry with RECT(3,5,6,4) at(13,7).
Twenty-four copies and98,304 destination-pixel readbacks pass, checking every byte
inside and outside the copy. Actual active2/-1 patterns proceed;96 zero-source or
zero-destination guards pass, including null destination behind inactive source.
Owned interfaces are released before real shutdown. Original WndProc is an
explicit reference boundary; SDK seeds are not a production renderer.

Full92-routine renewal, fidelity audit and exports pass at snapshot
`a62df57f1794186d6a83beb07fadb9db116ed51fb11168411d62e14be0dc78b5`, with92 fresh
compilation/emulation and31 scoped native records. LF working/index checks, staged
gates and tracked hooks are required before commit. Original compiler/link configuration remains
unresolved; GCC/Clang/LLD are provisional behavior-validation compilers. Arbitrary
caller-stack aliases, dispatch replacement/reentry, native faults/lost surfaces,
primary clipping, production frame integration and native startup, menus, input,
audio and representative races remain unverified. Focused DLLs are not a playable
rebuilt game. Continue authentic pixel copying and its real rendering dependencies
after this separately validated feature commit.
