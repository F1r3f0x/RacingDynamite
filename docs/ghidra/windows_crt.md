# Authenticated Windows CRT Inventory & Architecture (2026-10-07)

Scope: Identification, classification, and architectural boundaries of statically
linked C Run-Time (CRT) library routines in `IGN_WIN.EXE`.

All observations below derive directly from the fingerprinted Windows executable:
- Target: `Ignition/Ignition/IGN_WIN.EXE`
- Size: 915,968 bytes
- SHA-256: `7665e4e736bfd6c90790cedbb27e2de7e98a167374eb77933533c54ef0dc8782`
- Preferred image base: `0x00400000`
- PE entry RVA: `0x00069950` / VA `0x00469950`

---

## 1. Compiler and Linker Evidence

1. **PE Linker Version**:
   The optional header specifies linker major 4, minor 20 (Microsoft Incremental Linker 4.20).
2. **Debug Directories**:
   CodeView NB10 record names `d:\projects\ignition\Ign_win.pdb`.
   Authentic FPO debug records cover 16,192 bytes.
3. **PE Startup Organization**:
   PE entry point `0x00469950` implements Microsoft `WinMainCRTStartup` / `CRT_Entry`.
   It registers an `FS:[0]` Structured Exception Handling frame, queries `GetVersion`,
   initializes heap and I/O tables, builds environment and argument lists (`__setargv`,
   `__setenvp`), processes C initializer tables (`__cinit` via `__initterm`), and
   dispatches to application `WinMain` at `0x004120A0` with four stack arguments.
4. **CRT Clustered Layout**:
   All statically linked CRT routines are positioned contiguously at the upper end of
   the `.text` section, spanning **RVA `0x00069100` through `0x00078A40`** (VA `0x00469100`
   through `0x00478A40`).

---

## 2. Functional Categories of CRT Routines

### Process Lifecycle, Startup & Teardown
- `CRT_Entry` / `WinMainCRTStartup` (`0x00469950`): PE entry point with SEH frame.
- `__cinit` (`0x004697B0`): Dispatches static C initializers across pointer tables.
- `__initterm` (`0x004698F0`): Traverses function pointer arrays (`0x0047C000`..`0x0047C014`).
- `_exit` (`0x004697E0`) / `__exit` (`0x00469800`) / `doexit` (`0x00469840`): Exit handlers and atexit dispatch.
- `__amsg_exit` (`0x00469AF0`): Runtime fatal error message formatter and terminator.
- `__setargv` (`0x0046E620`) / `parse_cmdline` (`0x0046E6C0`): Command line tokenization (`argc`, `argv`).
- `__setenvp` (`0x0046E530`) / `___crtGetEnvironmentStringsA` (`0x0046EAA0`): Environment variable setup.

### Heap Management
- `_malloc` (`0x00469400`) / `_free` (`0x004693B0`): Standard heap allocation and release.
- `_calloc` (`0x004692C0`): Zero-initialized allocation.
- `__nh_malloc` (`0x00469420`) / `__heap_alloc` (`0x00469470`): Low-level Win32 heap wrappers.
- `__heap_init` (`0x0046AE90`): Process heap initialization (`HeapCreate`).
- `operator_new` (`0x004694B0`): C++ heap operator wrapping `_malloc`.

### Stream and File I/O
- `_fclose` (`0x00469100`), `_fread` (`0x00469170`), `_fwrite` (`0x00469B20`): Stream read/write/close.
- `__fsopen` (`0x00469360`): File stream opener backing `fopen`.
- `_fseek` (`0x00469D40`), `_ftell` (`0x00469DE0`), `_fsetpos` (`0x00469D20`): File stream position handlers.
- `_fflush` (`0x0046A540`), `flsall` (`0x0046A610`): Buffer flushing.
- `__filelength` (`0x004787C0`): File length query by low-level file handle.
- `__ioinit` (`0x0046EF10`), `__close` (`0x0046CD80`), `__freebuf` (`0x0046CCA0`), `__filbuf` (`0x0046CD30`).

### Formatted I/O & Parsing
- `_sprintf` (`0x004695C0`), `_fprintf` (`0x00469770`), `_printf` (`0x0046A500`): Printf family.
- `_vsprintf` (`0x0046AC90`), `_sscanf` (`0x0046AD60`): Vararg formatting and scanning.
- Internal output formatting helpers (`write_char`, `write_multi_char`, `write_string`, `get_int_arg`, `get_int64_arg`, `get_short_arg`).

### String & Conversion Utilities
- `_atol` (`0x0046A010`), `_atoi` (`0x0046A0C0`), `__atoi64` (`0x0046A0D0`): Ascii-to-number conversion.
- `__itoa` (`0x00478850`), `xtoa` (`0x00478890`), `__ultoa` (`0x00478920`), `x64toa` (`0x00478980`): Integer-to-string formatters.
- `__strupr` (`0x00478A40`), `_strncat` (`0x0046B310`), `wcsncnt` (`0x004758C0`): String manipulation.

### Math, 64-Bit Arithmetic & Floating Point
- `__allmul` (`0x00469680`): 64-bit integer multiplication helper for 32-bit x86.
- `__alldiv` (`0x00469730`): 64-bit integer division helper for 32-bit x86.
- `__ftol` (`0x00469520`): Float-to-integer conversion helper.
- `__fpmath` (`0x00469540`): Floating point initialization support.
- `_rand` (`0x004694E0`): Pseudo-random number generator.
- `$I10_OUTPUT` (`0x004747D0`), `__ld12tod`, `__atodbl`: Floating point parsing and representation.

### Operating System & Environment Support
- `__findfirst` (`0x0046A1B0`), `__findnext` (`0x0046A300`): Directory enumeration (`FindFirstFileA`).
- `findenv` (`0x004770B0`), `copy_environ` (`0x00477110`): Process environment table lookups.
- `getSystemCP` (`0x0046EE10`), `setSBCS` (`0x0046EEC0`): Locale / Code Page support.
- `cvtdate` (`0x00472ED0`): Time/date conversion helper.

---

## 3. Reconstruction & Fidelity Strategy

1. **Classification in `database/decomp.db`**:
   - Every confirmed CRT routine is registered with `classification = 'crt'`, `abi = 'cdecl'`,
     `analysis_stage = 'named'`, and references this evidence document.
2. **Exclusion from Decompilation**:
   - CRT routines are compiler-supplied standard library code; they are **never** reconstructed
     as custom game C source in `decomp/src/`.
   - Reconstructed game routines call standard C89 runtime headers (`<stdlib.h>`, `<string.h>`,
     `<stdio.h>`) instead of re-implementing CRT functions.
3. **Differential Emulation & Modeling**:
   - In test harnesses (e.g. `verify_matching.py`), calls from game routines to CRT entry points
     are hooked and modeled using standard host C library semantics (e.g. host `malloc`, `free`,
     `fopen`, `fread`), isolating game logic verification from compiler library details.
