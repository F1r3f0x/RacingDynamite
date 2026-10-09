#include <windows.h>
#include <stdint.h>

typedef int (__cdecl *SineFn)(void);
typedef unsigned int (__cdecl *ControlFn)(unsigned int, unsigned int);
typedef unsigned int (__cdecl *ClearFn)(void);

static void output(const char *text) {
    DWORD written;
    DWORD size;
    size = 0;
    while (text[size] != 0) {
        size++;
    }
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), text, size, &written, 0);
}

static void fail(const char *message, unsigned int code) {
    output(message);
    ExitProcess(code);
}

void WINAPI NativeEntry(void) {
    HANDLE file;
    DWORD size;
    DWORD read;
    unsigned char *raw;
    unsigned char *image;
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS32 *nt;
    IMAGE_SECTION_HEADER *section;
    HMODULE compiled;
    HMODULE crt;
    SineFn original_fn;
    SineFn compiled_fn;
    ControlFn control;
    ClearFn clear;
    volatile int *original_table;
    volatile int *compiled_table;
    unsigned int precision[3];
    unsigned int mode;
    unsigned int p;
    unsigned int r;
    unsigned int n;
    unsigned int j;
    unsigned int prior;
    unsigned int after;
    int original_result;
    int compiled_result;
    uint32_t *word;
    IMAGE_BASE_RELOCATION *reloc;
    WORD *entries;
    DWORD reloc_offset;
    DWORD reloc_end;
    DWORD delta;
    DWORD target;
    union {
        SineFn fn;
        FARPROC generic;
        unsigned char *bytes;
        volatile int *table;
    } binding;

    file = CreateFileA("IGN_WIN.EXE", GENERIC_READ, FILE_SHARE_READ, 0,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if (file == INVALID_HANDLE_VALUE) {
        fail("FAIL authentic disposable input open\n", 1);
    }
    size = GetFileSize(file, 0);
    if (size != 915968) {
        fail("FAIL authentic size\n", 2);
    }
    raw = (unsigned char *)VirtualAlloc((void *)0x30000000, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (raw == 0 || !ReadFile(file, raw, size, &read, 0) || read != size) {
        fail("FAIL authentic read\n", 3);
    }
    CloseHandle(file);
    dos = (IMAGE_DOS_HEADER *)raw;
    nt = (IMAGE_NT_HEADERS32 *)(raw + dos->e_lfanew);
    if (dos->e_magic != 0x5a4d || nt->Signature != 0x4550 ||
        nt->FileHeader.Machine != 0x14c || nt->OptionalHeader.Magic != 0x10b ||
        nt->OptionalHeader.ImageBase != 0x400000) {
        fail("FAIL authentic PE32\n", 4);
    }
    image = (unsigned char *)VirtualAlloc((void *)0x400000,
        nt->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (image == 0) {
        image = (unsigned char *)VirtualAlloc(0, nt->OptionalHeader.SizeOfImage,
            MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    }
    if (image == 0) {
        fail("FAIL original native mapping\n", 5);
    }
    for (j = 0; j < nt->OptionalHeader.SizeOfHeaders; j++) {
        image[j] = raw[j];
    }
    section = IMAGE_FIRST_SECTION(nt);
    for (n = 0; n < nt->FileHeader.NumberOfSections; n++) {
        if (section[n].PointerToRawData + section[n].SizeOfRawData > size ||
            section[n].VirtualAddress + section[n].SizeOfRawData > nt->OptionalHeader.SizeOfImage) {
            fail("FAIL original section bounds\n", 6);
        }
        for (j = 0; j < section[n].SizeOfRawData; j++) {
            image[section[n].VirtualAddress + j] = raw[section[n].PointerToRawData + j];
        }
    }
    delta = (DWORD)image - nt->OptionalHeader.ImageBase;
    reloc_offset = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress;
    reloc_end = reloc_offset + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size;
    if (reloc_end > nt->OptionalHeader.SizeOfImage) {
        fail("FAIL relocation directory bounds\n", 13);
    }
    while (reloc_offset < reloc_end) {
        reloc = (IMAGE_BASE_RELOCATION *)(image + reloc_offset);
        if (reloc->SizeOfBlock < 8 || reloc_offset + reloc->SizeOfBlock > reloc_end) {
            fail("FAIL relocation block bounds\n", 14);
        }
        entries = (WORD *)(reloc + 1);
        for (n = 0; n < (reloc->SizeOfBlock - 8) / 2; n++) {
            if ((entries[n] >> 12) == IMAGE_REL_BASED_ABSOLUTE) {
                continue;
            }
            if ((entries[n] >> 12) != IMAGE_REL_BASED_HIGHLOW) {
                fail("FAIL unsupported PE32 relocation\n", 15);
            }
            target = reloc->VirtualAddress + (entries[n] & 0xfff);
            if (target + 4 > nt->OptionalHeader.SizeOfImage) {
                fail("FAIL relocation operand bounds\n", 16);
            }
            word = (uint32_t *)(image + target);
            *word += delta;
        }
        reloc_offset += reloc->SizeOfBlock;
    }
    /* This isolated authentic routine reaches only its real private _ftol body,
     * constants and BSS table. No imports or game startup are invoked or modeled. */
    binding.bytes = image + 0x60740;
    original_fn = binding.fn;
    original_table = (volatile int *)(image + 0x11bb60);
    compiled = LoadLibraryA("sine_validation.dll");
    crt = LoadLibraryA("msvcrt.dll");
    if (compiled == 0 || crt == 0) {
        fail("FAIL native modules\n", 7);
    }
    compiled_fn = (SineFn)GetProcAddress(compiled, "Gfx_InitSineTable");
    binding.generic = GetProcAddress(compiled, "g_nativeSineTable");
    compiled_table = binding.table;
    control = (ControlFn)GetProcAddress(crt, "_controlfp");
    clear = (ClearFn)GetProcAddress(crt, "_clearfp");
    if (compiled_fn == 0 || compiled_table == 0 || control == 0 || clear == 0) {
        fail("FAIL native exports\n", 8);
    }
    precision[0] = 0x20000;
    precision[1] = 0x10000;
    precision[2] = 0;
    prior = control(0, 0);
    for (p = 0; p < 3; p++) {
        for (r = 0; r < 4; r++) {
            mode = 0x8001f | precision[p] | (r << 8);
            after = control(mode, 0x8031f | 0x30000);
            if ((after & (0x30000 | 0x300)) != (mode & (0x30000 | 0x300))) {
                fail("FAIL requested native x87 control mode\n", 9);
            }
            for (n = 0; n < 4096; n++) {
                original_table[n] = (int)(0x13570000u + n);
                compiled_table[n] = (int)(0x24680000u + n);
            }
            word = (uint32_t *)(image + 0x11bb5c);
            *word = 0x11223344;
            clear();
            original_result = original_fn();
            if (control(0, 0) != after || *word != 0x11223344) {
                fail("FAIL authentic native control/preceding-word preservation\n", 10);
            }
            clear();
            compiled_result = compiled_fn();
            if (control(0, 0) != after || original_result != compiled_result) {
                fail("FAIL compiled native control/result\n", 11);
            }
            for (n = 0; n < 4096; n++) {
                if (original_table[n] != compiled_table[n]) {
                    fail("FAIL native sine table comparison\n", 12);
                }
            }
        }
    }
    control(prior, 0x8031f | 0x30000);
    output("PASS native Win32 original versus pure C89 sine probe: 12 masked control modes, 4096 entries each; real original _ftol; values/result/control and original preceding word. No game startup or playable-game certification.\n");
    ExitProcess(0);
}
