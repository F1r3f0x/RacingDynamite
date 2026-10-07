# First verified Windows startup map

Evidence recorded 2026-10-07 (America/Santiago). This is a bounded static map and
one behavioral reconstruction, not full startup or gameplay parity.

## Target and analysis provenance

Read directly from `Ignition/Ignition/IGN_WIN.EXE`: 915,968 bytes; SHA-256
`7665E4E736BFD6C90790CEDBB27E2DE7E98A167374EB77933533C54EF0DC8782`;
PE32 (`0x010b`), x86 (`0x014c`), preferred base `0x00400000`, entry RVA
`0x00069950`, entry VA `0x00469950`. All VAs below assume this preferred base;
RVA = VA minus `0x00400000`.

The localhost:8080 Ghidra bridge responded. Its selected section map and bounded
startup listings agreed with direct file disassembly, but its API has no selected
program fingerprint endpoint. Selected-program identity therefore remains
unverified. `RacingDynamite.lock`/`.lock~` existed; no import into the open project,
symbol synchronization, or Ghidra mutations were attempted. Instead, authoritative
analysis used pefile and Capstone on the independently fingerprinted original.
Bridge names, annotations, and pseudocode served only as search hypotheses. In
particular, its one-argument `WinMain` prototype is incomplete: the original caller
pushes four arguments and the application entry returns with `RET 16`.

No DOS executable, inherited function status, or regional executable was used.
Scratch assembly/pseudocode and copied original assets are not committed.

## PE inventory and compiler/ABI clues

| Section | RVA | Virtual bytes | Raw offset | Raw bytes | Characteristics |
| --- | --- | --- | --- | --- | --- |
| .text | 0x00001000 | 0x77B8C | 0x400 | 0x77C00 | 0x60000020 |
| .rdata | 0x00079000 | 0x25A0 | 0x78000 | 0x2600 | 0x40000040 |
| .data | 0x0007C000 | 0x1CFF80 | 0x7A600 | 0x40E00 | 0xC0000040 |
| .idata | 0x0024C000 | 0xC54 | 0xBB400 | 0xE00 | 0xC0000040 |
| STACK | 0x0024D000 | 0x200 | 0xBC200 | 0x200 | 0xC0000040 |
| code | 0x0024E000 | 0x1597 | 0xBC400 | 0x1600 | 0xC0000040 |
| .rsrc | 0x00250000 | 0x6DAC | 0xBDA00 | 0x6E00 | 0x40000040 |
| .reloc | 0x00257000 | 0x16FD4 | 0xC4800 | 0x17000 | 0x42000040 |

Eight import descriptors contain 114 imports: KERNEL32 (63), USER32 (36), GDI32
(2), WINMM (8), DINPUT (1), DDRAW (1), DSOUND (1), and DPLAYX (two ordinals,
1 and 2; their semantics were not assumed). `windows_inspect.py` prints each IAT
VA and symbol. Representative slots: DirectInputCreateA `0x0064C2A4`,
DirectDrawCreate `0x0064C29C`, DirectSoundCreate `0x0064C2B8`,
QueryPerformanceFrequency `0x0064C2CC`, QueryPerformanceCounter `0x0064C38C`,
CreateWindowExA `0x0064C440`, mciSendStringA `0x0064C468`.

The relocation directory RVA/size is `0x00257000/0x15E44`: 164 blocks, 44,089
HIGHLOW entries and 89 ABSOLUTE padding entries. Do not count the entire larger
`.reloc` section as the declared relocation directory. There are no export, TLS,
exception, or load-config directories. Debug RVA/size is `0x00079000/0x54`:
MISC image name `d:\projects\ignition/Ign_win.exe`, FPO records (16,192 bytes),
and NB10 CodeView data naming `d:\projects\ignition\Ign_win.pdb`.

Linker version bytes are 4.20. The PDB/FPO format, FS:[0] exception frame at entry,
CRT organization, stack argument calls, and stdcall Windows callbacks support a
Microsoft-compatible compiler/ABI hypothesis. These do **not** prove MSVC 4.2,
an exact compiler version, or original optimization flags. The extra `STACK` and
`code` sections are unexplained; no packing/protection diagnosis is asserted.
Watcom register conventions are not used. Modern Clang 19.1.1 and LLD 19.1.1 are
provisional behavioral-validation tools, not the recovered vintage compiler.

## Verified startup sequence

1. At `0x00469950`, entry installs an FS:[0] exception frame, calls GetVersion,
   and writes version state at `0x004BB0BC–0x004BB0C8`. Its observed path calls
   `0x0046AE90`, `0x0046EF10`, `0x0046EF00`, GetCommandLineA,
   `0x0046EAA0`, `0x0046E620`, `0x0046E530`, and `0x004697B0`.
   These are CRT dependencies; their complete contracts are not reconstructed.
   `0x004697B0` dispatches an optional pointer at `0x004BB0A0` and traverses
   initializer ranges `0x0047C00C–0x0047C014` and `0x0047C000–0x0047C008`
   through `0x004698F0`. Their contents and targets still require analysis.
2. CRT skips the executable token (including quote/character handling), obtains
   GetStartupInfoA show settings, and calls GetModuleHandleA(NULL). At
   `0x00469A96` it calls `0x004120A0` with instance, NULL previous instance,
   remaining command line, and show argument. At `0x00469A9C` its result goes to
   the termination path at `0x004697E0`. The PE entry is **not** WinMain.
3. Application entry stores the instance at `0x004C5398` and the pointer
   `0x00479558` at `0x004C53A0`. The latter's meaning is unresolved; it is not
   treated as a recovered struct. It loads cursor/icon resource 32512, stores
   cursor at `0x004C5360`, and registers class string `Ignition` (`0x00493740`),
   style 8, window procedure `0x004122D0`, and GetStockObject(4) background.
   Registration failure returns zero.
4. QueryPerformanceFrequency writes the 8-byte value at `0x004C5348` and a
   Boolean availability flag at `0x00493720`. On success, the initial counter is
   stored at `0x004C5368`; otherwise timeBeginPeriod(1) is called.
5. Calls `0x0045B170` then `0x00412500`. The former calls `0x0045B4F0`,
   `0x0045AD50`, **`0x0045B1F0`**, `0x00455AB0`, `0x0045E610`, and
   `0x00456AF0(0)` in that order. Only the bold helper is reconstructed here.
   `0x00412500` again calls `0x00456AF0(0)`, then `0x00456BC0`, then
   `0x00455AC0`. The last two failures return zero; success sets
   `0x004C5390 = 1`. The first call's result is not checked.
6. `0x00456AF0(0)` initializes dispatch tables through `0x0045B690` and
   `0x00456E60`. Direct stores prove `0x0050EB6C = 0x0045B740` and
   `0x0050EBA0 = 0x00456F20`. Thus `0x00456BC0` calls the window/DirectDraw
   initializer `0x0045B740`; after a nonzero result, it tail-dispatches to
   `0x00456F20`, a six-byte original function returning 1. This original behavior
   is observed, not a replacement success stub.
7. `0x0045B740` calls CreateWindowExA at `0x0045B783`, stores the HWND at
   `0x004C539C` and `0x00493738`, then calls UpdateWindow and SetFocus. At
   `0x0045B7C6` it calls DirectDrawCreate's thunk `0x0047879C`, with interface
   storage at `0x00512C50`. Subsequent vtable calls set cooperative/display
   behavior; fullscreen selection reads `0x00493728`. Full interface contracts,
   surfaces, palette and renderer behavior remain unreconstructed.
8. `0x00455AC0` calls the DirectInputCreateA thunk `0x00478778` at
   `0x00455B1A`, using version `0x300` and interface storage `0x0050DF60`.
   It creates/configures a device at `0x0050E26C`, uses HWND `0x004C539C`,
   acquires it, and initializes input arrays. Its COM vtable offsets are observed;
   complete structure/format semantics are not implemented here.
9. The active message loop (`0x004C5384 != 0`) uses PeekMessageA followed by
   GetMessageA/TranslateMessage/DispatchMessageA when messages exist. Otherwise
   it calls frame dispatcher `0x00412230`; a zero result posts shutdown through
   `0x00412530`. Inactive mode blocks in GetMessageA. A zero GetMessage result
   returns MSG.wParam. The code does not explicitly distinguish GetMessage's
   negative error result from other nonzero results; no fix is introduced.

### Timing, messages and initial presentation

The window procedure stores WM_ACTIVATEAPP's wParam in `0x004C5384`, updates
the rectangle at `0x004C5350` for move/size, handles cursor display, and routes
WM_DESTROY through cleanup `0x00412550` and PostQuitMessage unless
`0x0049372C` is set. WM_SYSKEYDOWN/VK_RETURN tears down and reinitializes graphics
and input while toggling `0x00493728`. Remaining messages go to DefWindowProcA.
The dispatch jump table is included in the 400-byte FPO extent, not executable
instructions. Its five entries cover messages 1–5.

Timer helper `0x00412460` computes milliseconds since the initial performance
counter using 64-bit arithmetic helpers `0x00469730/0x00469680`, or uses
timeGetTime. It stores last time at `0x004937B8`, accumulates active elapsed time
at `0x004937AC`, and returns that accumulator. Dispatcher `0x00412230` uses
stage `0x00493734`, initialization guard `0x004937B0`, shutdown guard
`0x004937B4`, and timestamp `0x0049373C` to choose initialization
`0x00417270`, normal dispatch `0x004172B0`, or cleanup `0x00417E90`.

`0x00417270` sets state `0x00639394 = 2`, clears several game flags, and calls
`0x00417EA0` then `0x004184C0`. Direct code in `0x00417EA0` requests first
640x480x8 and then 320x200x8 through `0x00456BE0`; these are static requests,
not observed display modes. Palette dispatch is `0x00456C40`. The initializer
calls `0x00418130` to load system graphics/fonts; original strings include
`n_Sysg_2.pic` at `0x00498998` and `SYS.COL` at `0x00498960`. It initializes
joystick capability state through `0x004596A0` (joyGetDevCapsA), then calls
`0x00458040` for MCI strings `open cdaudio`, `set cdaudio audio all on`, and
`set cdaudio time format msf`, using buffer `0x0050EF70`.

`0x004184C0` calls `0x00457B50`, conditionally `0x00457DA0` based on
`0x0050EF4C`, and ultimately sets `0x00639394 = 1`. It is therefore unsafe to
claim that the earlier state-2 assignment proves a played intro. The later
dispatcher/UI layout, language selection, asset decoding and playback await
separate verification. DirectSoundCreate is imported and its direct call at
`0x004679E2` belongs to FPO function `0x00467920` (1531 bytes); its reachability
from this early path is unresolved. No complete audio initialization claim is made.

### Extents and calling conventions

These extents come from authentic FPO records, corroborated by bounded code and
call sites. High confidence in byte extent does not imply a complete recovered
contract. Internal zero-argument RET functions are compatible with cdecl; a
zero-argument RET alone cannot distinguish every calling-convention spelling.

| Descriptive role (names assigned here) | VA / RVA | Bytes | ABI / evidence |
| --- | --- | --- | --- |
| CRT entry observed path | 0x00469950 / 0x00069950 | 337 observed | No full extent claim; termination/SEH branches unresolved |
| Application entry | 0x004120A0 / 0x000120A0 | 390 | Four stack args, RET 16, EAX result; high |
| Frame dispatcher | 0x00412230 / 0x00012230 | 154 | No args, RET, EAX result; high |
| Window procedure + table | 0x004122D0 / 0x000122D0 | 400 | Four stack args, RET 16; high |
| Active-time helper | 0x00412460 / 0x00012460 | 152 | No args, RET, EAX; high |
| Application platform init | 0x00412500 / 0x00012500 | 47 | No args, RET, EAX; high |
| Startup service initializer | 0x0045B170 / 0x0005B170 | 41 | No args, RET, EAX; high |
| Handle-table initializer | 0x0045B1F0 / 0x0005B1F0 | 76 | No args, RET, EAX=1, saves EDI; high, reconstructed |
| Dispatch selector | 0x00456AF0 / 0x00056AF0 | 30 | One stack arg, caller ADD ESP 4; high |
| Graphics table initializer | 0x0045B690 / 0x0005B690 | 146 | No args, RET; high |
| Secondary table initializer | 0x00456E60 / 0x00056E60 | 181 | No args, RET; high |
| Graphics startup dispatch | 0x00456BC0 / 0x00056BC0 | 19 | No args, RET or tail jump; high |
| Window/DirectDraw init | 0x0045B740 / 0x0005B740 | 1575 | No args, RET, EAX; extent high, partial semantics |
| Secondary init target | 0x00456F20 / 0x00056F20 | 6 | No args, RET, EAX=1; high |
| DirectInput initializer | 0x00455AC0 / 0x00055AC0 | 406 | No args, RET, EAX; high, COM structures unresolved |
| First game initialization | 0x00417270 / 0x00017270 | 58 | No args, RET, EAX=1; high |
| Graphics/assets/UI initialization | 0x00417EA0 / 0x00017EA0 | 655 | No args, RET, EAX=1 or termination; high extent |
| Initial state transition | 0x004184C0 / 0x000184C0 | 60 | No args, RET; high extent |
| MCI CD initialization | 0x00458040 / 0x00058040 | 89 | No args, RET, EAX result; high |

## Reconstructed routine: Mem_InitHandles

Chosen directly from the verified `entry -> application entry -> 0x0045B170 ->
0x0045B1F0` path. It is a non-CRT leaf with four global outputs and no platform
dependencies. The semantic name and placement in `mem.c` are inferred from
adjacent handle consumers, not recovered source symbols. It replaces the old DOS
`mem.c`/`mem.h`; none of their allocator/file-loader behavior is carried forward.

Its exact original range is `[0x0045B1F0,0x0045B23C)`; RVA `0x0005B1F0`, 76
bytes, 20 instructions. SHA-256 of those file bytes:
`bed1560d41e6ae198b393b26af425505bdae1eb3fb6f4169f6bb89267019cf8c`.
FPO independently gives the same 76 bytes, and INT3 padding follows the final RET.

| Global | VA / RVA | Layout and initial value |
| --- | --- | --- |
| g_memHandlesInitialized | 0x004BAB38 / 0x000BAB38 | 32-bit word, file initialized zero |
| g_memHandleStatus | 0x005116E0 / 0x001116E0 | 200 contiguous 32-bit words, 800 loader-zeroed bytes |
| g_memHandleIds | 0x00511230 / 0x00111230 | 200 contiguous 16-bit words, 400 loader-zeroed bytes |
| g_memHandleCursor | 0x00512040 / 0x00112040 | 16-bit word, loader initialized zero |

No struct or packing is invented. The adjacent consumer at `0x0045B1B0` uses
MOVSX for the cursor and IDs, supporting signed `short`; all initialized IDs
are positive 1–200. Flag/status signedness is not distinguished by this routine;
unsigned 32-bit storage preserves all bit patterns. Compile-time C89 size checks
reject hosts with incompatible word widths. The original BSS is the virtual tail
of `.data`, not bytes read by seeking beyond its raw extent.

When the flag is **exactly 1**, return 1 without writing any state. For every other
32-bit flag value, first write flag=1, clear exactly 200 dwords with REP STOSD,
store words 1–200 at ID indices 0–199, reset the cursor to zero, and return 1.
No allocation, platform calls, error path, or success stub is added. EDI is
preserved; EAX/ECX are scratch in the original. The contract assumes clear DF,
as required by the x86 C ABI. Concurrent/asynchronously observed intermediate
states and instruction timing are not validated. `@fidelity EXACT` refers to this
behavioral contract; it does not claim compiler instruction equality.

## Build and independent validation

From the repository root in PowerShell, with `uv`, `clang`, and `lld-link` on PATH:

```powershell
uv run tools/windows_inspect.py
uv run python tools/build_decomp.py
uv run tools/verify_matching.py
uv run python tools/verify_fidelity.py
git diff --check
```

`windows_inspect.py` reproduces sections/imports/relocations/FPO extents, checks
specific startup call sites and dispatch initializers, and distinguishes raw
initialized state from BSS. These checks use original file bytes, no bridge names.
Both analysis/validation scripts declare pinned dependencies through PEP 723;
`uv run <script>` provides an isolated environment without changing project deps.

Build selects only `decomp/src/mem.c`, includes `decomp/include`, and uses Clang
19.1.1 with `--target=i686-pc-windows-msvc -std=c89 -pedantic-errors -Wall
-Wextra -Werror -O2 -ffreestanding -fno-builtin -fno-vectorize
-fno-slp-vectorize -mno-sse -mno-sse2`. LLD links `/dll /noentry /nodefaultlib
/machine:x86 /base:0x10000000`, exporting the routine and four globals. The
printed command lines include all actual input/output paths. Compiler/linker
overrides are available only as `--compiler`/`--linker` in the build command.
No DOS fallback, stale object globbing, dependency stubs, or playable EXE is used.
Fresh products live only in ignored `build/decomp/windows/`.

Results: strict C89 compilation and focused PE32 DLL linkage passed. The DLL has
no import dependencies; its generated `.text` is 241 bytes. Original routine
bytes/instructions are fingerprinted and decoded; raw/relocated instruction
equality and linked binary matching are **not claimed**. The raw code-byte
comparison reports false; relocation-aware instruction comparison is not evaluated.
Code generation differs.

The verifier maps the original PE and independently compiled DLL into separate
Unicorn x86 CPUs, directly executes their machine instructions, and compares all
four state regions. It does not load the original DLLs or execute game startup.
Thirty comparisons passed: flags 0, 1, 2, 0xFFFFFFFF, 0x80000000; zero, 0xFF,
and seeded random table/cursor inputs; first and repeat invocation of each.
Tests verify EAX=1, ESP return balance, EBX/ESI/EDI/EBP preservation, DF clear,
full 200-entry boundaries, exact skip behavior, write-region bounds, unchanged
image bytes outside those globals, and the original's first flag/last cursor
writes. Assembly-derived independent expectations check the original result too.
Emulated original execution is independent of the new C; native routine execution
and native API integration remain unverified.

The four existing `test_decomp_doctor.py` regression tests also passed, the
read-only doctor verified the target without errors, modified Python tools passed
syntax compilation, and `git diff --check` passed. Verification scripts reject
Python `-O`/`PYTHONOPTIMIZE` so assertion checks cannot silently disappear.

## Disposable original runtime probe

Copied the full `Ignition/Ignition/` directory (342 files, about 541 MB) to
`build/runtime/windows-first-milestone/`, then rechecked the copy's executable
hash. Started only that copy with its directory as working directory and
`Start-Process -WindowStyle Hidden -PassThru`; sampled process state after five
seconds. The process had exited with code **0**. It was not necessary to terminate
it. No compatibility settings or wrappers were added; the launch inherited
`__COMPAT_LAYER=RedirectDrvMgt`. Other registry-based compatibility settings were
not inventoried, and the cause of exit is unresolved. SysWOW64 copies of ddraw,
dsound, dinput and dplayx were present; presence alone does not prove loader success.

This session's UI automation does not expose native Windows apps. There were no
visual observations of language selection, loading, intro, first menu, error
dialogs, sound, or gameplay. A normal exit code does not prove any of those states.
The original runtime baseline is therefore limited to launch/process evidence.
Future visual/debugger work must use another disposable copy and record its
actual compatibility configuration. Original files remain untouched.

## Tracking boundary and next dependency

`docs/tracking/windows_inventory.json` is the temporary active inventory for this
feature: one identified/analyzed/reconstructed/compiled routine, emulation validated,
no instruction or linked match and no native parity claim. No completion totals
are transferred from DOS. `database/decomp.db`, its SQL dump, old Ghidra registries,
remaining decomp modules, dashboards and unretargeted audits remain explicitly
legacy. `verify_fidelity.py` audits only this Windows inventory and migrated module;
it is not a project-wide fidelity certificate. Broader database/dashboard migration
is deferred to keep this milestone bounded.

Next reconstruct the adjacent handle-ID consumer at **`0x0045B1B0`**, whose direct
code reads the initialized flag, sign-extends cursor/IDs, checks the 200-entry
boundary, and increments the cursor. Recover its signed boundary behavior and
callers before implementing it. Further startup priorities are CRT initializer
targets, complete `0x0045B740` window/DirectDraw contracts, and an observed native
language/menu baseline.
