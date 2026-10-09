# Authentic Windows primitive initialization: work in progress

Standard IGN_WIN.EXE, 915968 bytes, SHA256
7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782,
PE32 i386 preferred VA0x00400000. Originals are untouched. Local fingerprint,
FPO extents/instruction bytes and calls are authenticated; Ghidra responds.
Active authentic program is the user's operating assumption, not automated
identity verification. Missing function definitions do not contradict local
PE evidence; supplementary available xrefs/disassembly are retained in disposable
preparation. Semantic names/source placement are inferred.

| RVA / VA | Bytes / FPO tuple | SHA256 |
| --- | --- | --- |
| 0x5e610 / 0x0045E610 | 326 / (326, 0, 0, 0) | fd2abfef167ef5b92074ef41c17478dd955591025afb3aad714da050a29704a3 |
| 0x5f490 / 0x0045F490 | 14 / (14, 0, 0, 0) | a1a3b1021d987df71e310755f2adbd67749fa172443f0c85a583158118583f38 |
| 0x60740 / 0x00460740 | 60 / (60, 1, 0, 260) | d674e3df6ab18f1b8a699a6d69fdc82cf9ec6549914e46e23fe819ca7a6ca6f6 |
| 0x607c0 / 0x004607C0 | 11 / (11, 0, 0, 0) | 8c8710963f05a7f5a62074f2d9d1903f56311a193ee2af96aaa569f1d781e89d |
| 0x5f640 / 0x0045F640 | 68 / (68, 0, 0, 0) | c3b41d7b766be5723dc8350191a818ff5badf66130e2700298ed2b4a49286c6f |
| 0x5fa50 / 0x0045FA50 | 11 / (11, 0, 0, 0) | ac413b1076087447011efe2202591091027f7595c3cfe38b06960dffc90a86d2 |
| 0x60b50 / 0x00460B50 | 130 / (130, 0, 0, 257) | 2fc72c837750bc901796befe065be5c30a194bdd09a26ff4c50f9cc21dfcd7d7 |
| 0x63ab0 / 0x00463AB0 | 72 / (72, 0, 1, 257) | 771c561d8b04a3859a6ac714e9ef6660465b32fd1c776a2aefc97f62fe509d61 |

## Authenticated instruction contracts

Root RVA0x5E610 zeroes four-word packet VA0x0051FD00 then copies it in order to
seven complete 16-byte controls: 0x0051FE98, 0x005203B8, 0x00520360, 0x0051FE30,
0x0051FC88, 0x005203A0, 0x00520380. Preserve individual load/store ordering.
It calls RVAs0x5F490,0x60740,0x611D0,0x614D0,0x607C0,0x5F640,0x5FA50 then
jumps to0x60B50. Default sprite/packing initialization already have production
bodies. No guard, freeing of old tables, allocation check or rollback is present.

RVA0x5F490 calls0x63AB0 with VA0x0051FC20. Generic initializer0x63AB0 allocates
128 bytes through real Gfx_AllocBytes, publishes packet+12, then clears sixteen
pairs of dwords with two live pointer reads per pair. It writes packet+8=15,
+4=0,+0=16 in order. Consumer meanings remain open; do not invent ownership or
general capacity semantics from initializer constants alone.

RVA0x607C0 writes VA0x0051FE98=1; RVA0x5FA50 writes VA0x0051FE30=8.
RVA0x5F640 clears eleven words beginning VA0x0051FBC8 at offsets0,8,4,12,16,
24,20,28,32,36, writes VA0x00520360=1, then clears offset40. This separate block
follows the sprite default's64 bytes. Do not infer extra whole-structure clearing.

RVA0x60B50 calls0x63AB0 with VA0x00520370; writes default record VA0x0051AAE0
name=shared default string VA0x004BACF8; word+12=0x3F800000; words+28/+4=0;
byte+32=0; word+24=0x3ECCCCCD; +20=0x3F000000; +16=0x3DCCCCCD. Word+8 stays
untouched. Real Gfx_AllocBytes(16) pointer publishes at unaligned VA0x0051AB01
(record+33), then allocated words12,0,4095,0 are written and VA0x005203B8=1.
Recover packed layout independently; adjacency of compiler globals proves nothing.
Broader field meanings/consumers remain unverified.

## Sine/compiler/native preparatory evidence

RVA0x60740 builds4096 signed ints at VA0x0051BB60, preserving preceding word
0x0051BB5C. For indices0..4095 it FILDs index, multiplies binary64 constants
0.000244140625 (VA0x0047AF48),6.283192 (0x0047AF50), FSIN, multiplies
2147418112.0 (0x0047AF58), and calls private CRT conversion VA0x0046950C.
Converter saves CW, selects truncation, FISTPs qword, restores CW, returns EDX:EAX.
Low EAX stores to the table; last conversion remains EAX on return. Do not substitute
M_PI, a DOS table or invented return-zero helper.

A disposable strict C89 sin/integer-cast draft compiled with GCC16.2 `-m32
-ffast-math -fno-associative-math -fexcess-precision=fast -mno-sse -mfpmath=387`
emits FSIN and original multiply ordering, with no handwritten/inline assembly.
Without no-associative-math, multiplication reorders; that configuration is rejected.
LLD PE32 uses /safeseh:no for this GCC object. This is provisional behavior compiler
evidence; original compiler/link layout remains unresolved.

Emulation preparation passes all4096 ordered stores, return/cdecl stack/saved
registers/CW under twelve masked x87 modes:24/53/64-bit precision and four roundings.
A separate native Win32 probe also passes all4096 final values, return/CW and original
preceding-word preservation in the same twelve modes, executing real original
sine/private ftol bodies on Windows. It maps a fingerprinted disposable original
PE copy under build/runtime and applies existing HIGHLOW relocations if preferred
VA is unavailable. No imports/game startup are invoked or modeled. The compiled
C draft loads as an import-free PE32 DLL. Real Win32 APIs and msvcrt control/clear
functions service the native host. Empty x87 stack and masked exceptions delimit
these probes; unmasked exceptions, FPSW/nonempty-stack details and complete native
startup remain unverified. Preparatory source/artifact hashes and results are
retained under build/workflow; none are recorded production/native promotion.

## Production and validation

All eight production C89 bodies now execute the authentic chain with real existing
sprite-default, packing and byte-allocation helpers. Seven distinct controls are
owned coherent 16-byte objects. Pair controls retain two opaque prefix words,
limit and storage fields; consumer interpretation remains open. The named default
has a verified packed 37-byte view with parameters at byte33 and preserved word8.
No calloc, table freeing, allocation guard or rollback replaces native effects.
The file header no longer claims that DOS is the Windows target or identifies an
unproven compiler. Historical DOS bodies remain explicitly marked as such.

`uv run python tools/verify_primitive_init.py` builds a fresh focused PE32 DLL and
runs247 original-versus-C comparisons:96 persistent follow-ups,32 faults,
18 aliases and12 mutations. Per-routine counts are root37, graphics-pair28,
sine24, enable24, secondary-default24, line-control24, named-default33 and
pair-control53. Both images execute all real helper bodies; only CRT malloc is
modeled for differential execution. The independent raw-address structural oracle
uses authenticated original sine execution for numeric values, never the C body.
It compares ordered external reads/writes, dependency entries/arguments and heap
snapshots, complete declared BSS/arena/packed state, unchanged unrelated image
bytes, caller stack, nonvolatile registers/DF/CW/x87 stack top and normal results
where meaningful. Repeated initialization, live pointer/template mutations, two
loads per pair, late self-clearing storage with captured return pointer, packed
parameter aliases, partial fault effects and failures in each allocation phase
are covered. Unknown adjacent BSS aliases are excluded rather than inventing faults.
Cross-page atomicity, fault-time frames/registers, arbitrary reentry/concurrency,
exhaustive aliases and scalar/address collisions are not certified.

The same command compiles tools/native_sine_probe.c as a fresh Win32 host, copies
and fingerprints the authentic input under build/runtime/primitive_native, then
runs the production DLL sine on native Windows. Twelve masked x87 precision/round
modes compare all4096 values, return, CW and original preceding-word preservation,
with real original private ftol and real Win32/msvcrt control APIs. Existing PE
HIGHLOW relocations are applied in RAM when needed; imports/game startup are not
invoked. Native result recording is scoped only to RVA0x60740. Native whole
primitive/heap/startup/menu/render/input/audio/race parity remains unverified.
Unmasked exceptions and nonempty x87/FPSW details remain outside the native probe.

The builder compiles only the standard C89 sine body with separately probed GCC
ordered x87 flags; other bodies retain strict Clang settings. `sin`/integer cast
lower to real compiler-generated FSIN/FISTP; no math fixture or handwritten
assembly exists. LLD uses /safeseh:no for the GCC object. The provisional mixed
compiler is not an original toolchain identification. verify_matching checks the
full direct call graph and this contract after retained suites. Existing lifecycle
primitive boundary tests remain explicitly modeled and separate from this full
real-chain contract. Full completion renews59 reconstructed candidates; adjacent
annotation/global changes also renew existing default/packing provenance as needed.
Workflow audits/exports, LF working/index checks, staged gates/hooks and a separate
commit remain required. No playable rebuilt game is certified.

Custom font-cleanup, font-parser and file-loader DLL builders also link the real
sine object with /safeseh:no. The first full run exposed their missing dependency;
targeted cleanup validation and parser/loader builds pass after retargeting.
The full verifier is rerun against settled inputs. Existing Input_ResetCallbacks
RVA0x55AB0 and default initializer RVA0x611D0 bodies are unchanged; their source
annotation context changes with the corrected header/global declarations and is
covered by fresh prior suites and explicit RVA trailers.
