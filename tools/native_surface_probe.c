#include <windows.h>
#include <ddraw.h>
#include <stdint.h>

#ifdef SURFACE_TEST_CLEAR
#define SURFACE_TEST_SHUTDOWN
#endif

#ifdef SURFACE_TEST_ACCESS
#define SURFACE_TEST_SHUTDOWN
#endif

#ifdef SURFACE_TEST_TIMED
#define SURFACE_TEST_POLL
#endif
#ifdef SURFACE_TEST_POLL
#define SURFACE_TEST_INPUT
#endif

#ifdef SURFACE_TEST_INPUT
#define SURFACE_TEST_SHUTDOWN
#include <dinput.h>
#include <mmsystem.h>
typedef int (__cdecl *InputInitFn)(int32_t, int32_t);
#ifdef SURFACE_TEST_TIMED
typedef int (__cdecl *InputTimedInitFn)(uint32_t, int32_t, int32_t);
#endif
#ifndef SURFACE_TEST_TIMED
static void CALLBACK input_timer_probe(UINT id, UINT message, DWORD_PTR user, DWORD_PTR first, DWORD_PTR second) {
    (void)id; (void)message; (void)user; (void)first; (void)second;
}
#endif
#endif

#ifdef SURFACE_TEST_ENTRY
#define SURFACE_TEST_REBUILD
#endif

typedef int (__cdecl *OpenFn)(void);
typedef struct NativeRecord {
    uint32_t active, marker, opaque[5], width, height, depth, kind;
    LPDIRECTDRAWSURFACE surface;
} NativeRecord;
typedef char record_size[(sizeof(NativeRecord)==48)?1:-1];

static void output(const char *text) {
    DWORD written, n;
    n = 0;
    while (text[n]) { n++; }
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), text, n, &written, 0);
}
static void fail(const char *text, DWORD code) {
    output(text);
    ExitProcess(code);
}
static int equal_text(const char *left, const char *right) {
    while (*left && *left==*right) { left++; right++; }
    return *left==*right;
}
static FARPROC symbol(HMODULE module, const char *name) {
    FARPROC address;
    address = GetProcAddress(module, name);
    if (!address) { fail("FAIL native export/import\n", 30); }
    return address;
}

static void *data_symbol(HMODULE module, const char *name) {
    union { FARPROC function; void *object; } binding;
    binding.function=symbol(module,name);
    return binding.object;
}

#ifdef SURFACE_TEST_CLEAR
static void check_native_clear(unsigned char *image, HMODULE compiled, int side, NativeRecord *record) {
    typedef int (__cdecl *LockFn)(NativeRecord *, int);
    typedef int (__cdecl *UnaryFn)(NativeRecord *);
    union { unsigned char *bytes; FARPROC generic; LockFn lock; UnaryFn unary; OpenFn install; } binding;
    OpenFn install;
    UnaryFn entry;
    UnaryFn *installed;
    LockFn lock;
    UnaryFn unlock, clear;
    DDSURFACEDESC desc;
    unsigned char *bytes;
    volatile unsigned char *pixels;
    uint32_t width, height, pitch, n, index, row, column;
    int mode, k, zero;

    if (side==0) {
        binding.bytes=image+0x5c530;lock=binding.lock;
        binding.bytes=image+0x5c680;unlock=binding.unary;
        binding.bytes=image+0x5c3d0;clear=binding.unary;
        binding.bytes=image+0x56bb0;entry=binding.unary;
        binding.bytes=image+0x5b690;install=binding.install;
        installed=(UnaryFn *)(image+0x10eb84);
    } else {
        binding.generic=symbol(compiled,"Gfx_SurfaceLockNative");lock=binding.lock;
        binding.generic=symbol(compiled,"Gfx_SurfaceUnlockNative");unlock=binding.unary;
        binding.generic=symbol(compiled,"Gfx_SurfaceClearNative");clear=binding.unary;
        binding.generic=symbol(compiled,"Gfx_ClearSurfaceNative");entry=binding.unary;
        binding.generic=symbol(compiled,"Gfx_InstallSurfaceDispatch");install=binding.install;
        installed=(UnaryFn *)data_symbol(compiled,"g_surfaceClear");
    }
    if (install()!=1 || *installed!=clear) { fail("FAIL native Clear real installer\n",99); }
    width=record->width;height=record->height;
    bytes=(unsigned char *)&desc;
    for (k=0; k<(int)sizeof(desc); k++) { bytes[k]=0; }
    desc.dwSize=sizeof(desc);
    if (record->surface->lpVtbl->GetSurfaceDesc(record->surface,&desc)!=DD_OK || desc.dwHeight<32) {
        fail("FAIL paired native bounded Clear dimensions\n",90);
    }
    for (mode=0; mode<=3; mode++) {
        if (lock(record,2)!=1) { fail("FAIL paired native Clear seed lock\n",91); }
        pixels=(volatile unsigned char *)(uintptr_t)record->opaque[3];pitch=record->opaque[4];
        if (pitch<32 || pitch>0x100000) { fail("FAIL paired native Clear pitch bound\n",92); }
        n=32*pitch;
        for (index=0; index<n; index++) { pixels[index]=0xa5; }
        if (unlock(record)!=1 || (mode && lock(record,mode)!=1)) {
            fail("FAIL paired native Clear initial mode\n",93);
        }
        record->width=8;record->height=8;
        if ((mode%2 ? entry(record) : clear(record))!=1 || record->opaque[1]!=(uint32_t)mode || record->opaque[0]!=(uint32_t)(mode!=0)) {
            fail("FAIL paired native Clear restored mode\n",94);
        }
        record->width=width;record->height=height;
        if (unlock(record)!=1 || lock(record,2)!=1) { fail("FAIL paired native Clear inspection lock\n",95); }
        pixels=(volatile unsigned char *)(uintptr_t)record->opaque[2];
        if (record->opaque[4]!=pitch) { fail("FAIL paired native Clear inspection pitch change\n",96); }
        for (index=0; index<n; index++) {
            row=index/(4*pitch);column=index%(4*pitch);
            zero=row<8 && column<32;
            if (pixels[index]!=(unsigned char)(zero ? 0 : 0xa5)) {
                fail("FAIL paired native DWORD-scaled byte-pitch pattern\n",97);
            }
        }
        if (unlock(record)!=1) { fail("FAIL paired native Clear final unlock\n",98); }
    }
}


#endif

#ifdef SURFACE_TEST_ACCESS
static void check_native_access(unsigned char *image, HMODULE compiled, int side, NativeRecord *record) {
    typedef int (__cdecl *LockFn)(NativeRecord *, int);
    typedef int (__cdecl *UnlockFn)(NativeRecord *);
    union { unsigned char *bytes; FARPROC generic; LockFn lock; UnlockFn unlock; } binding;
    LockFn lock;
    UnlockFn unlock;
    uint32_t captured[5];
    int mode, next_mode, k;

    if (side==0) {
        binding.bytes=image+0x5c530;lock=binding.lock;
        binding.bytes=image+0x5c680;unlock=binding.unlock;
    } else {
        binding.generic=symbol(compiled,"Gfx_SurfaceLockNative");lock=binding.lock;
        binding.generic=symbol(compiled,"Gfx_SurfaceUnlockNative");unlock=binding.unlock;
    }
    for (mode=1; mode<=3; mode++) {
        if (lock(record,mode)!=1 || record->opaque[0]!=1 || record->opaque[1]!=(uint32_t)mode ||
                (mode==3 ? record->opaque[2]!=0 : record->opaque[2]==0) ||
                (mode==1 ? record->opaque[3]!=0 : record->opaque[3]==0) || !record->opaque[4] ||
                (mode==2 && record->opaque[2]!=record->opaque[3])) {
            fail("FAIL actual native surface lock role/pitch\n",80);
        }
        for (k=0; k<5; k++) { captured[k]=record->opaque[k]; }
        if (lock(record,mode)!=1) { fail("FAIL actual native same-mode lock\n",81); }
        for (k=0; k<5; k++) {
            if (record->opaque[k]!=captured[k]) { fail("FAIL actual native same-mode state\n",82); }
        }
        next_mode=mode==3 ? 1 : mode+1;
        if (lock(record,next_mode)!=1 || record->opaque[0]!=1 || record->opaque[1]!=(uint32_t)next_mode ||
                (next_mode==3 ? record->opaque[2]!=0 : record->opaque[2]==0) ||
                (next_mode==1 ? record->opaque[3]!=0 : record->opaque[3]==0)) {
            fail("FAIL actual native lock-mode transition\n",83);
        }
        for (k=0; k<5; k++) { captured[k]=record->opaque[k]; }
        if (unlock(record)!=1 || record->opaque[0]!=0 || record->opaque[1]!=0) {
            fail("FAIL actual native surface-field-address unlock\n",84);
        }
        for (k=2; k<5; k++) {
            if (record->opaque[k]!=captured[k]) { fail("FAIL actual native stale pointers/pitch\n",85); }
        }
        if (unlock(record)!=1) { fail("FAIL actual native repeated unlock\n",86); }
    }
}
#endif

#ifdef SURFACE_TEST_POLL
static void input_diagnostic_number(const char *label, uint32_t value) {
    char digits[12];
    unsigned int length, index;

    output(label);length=0;
    do { digits[length++]=(char)('0'+value%10);value/=10; } while (value);
    for (index=0; index<length/2; index++) {
        char temporary;
        temporary=digits[index];digits[index]=digits[length-1-index];digits[length-1-index]=temporary;
    }
    digits[length++]='\n';digits[length]=0;output(digits);
}
static volatile LONG native_presses, native_releases, native_notifications;
static volatile unsigned char *volatile *native_active_cursor;
static volatile unsigned char *native_active_queue;
static volatile uint32_t native_character_index;
static void __cdecl native_key_observer(int pressed, int scan) {
    if (scan==30) {
        if (pressed) {
            native_character_index=(uint32_t)(*native_active_cursor-native_active_queue);
            InterlockedIncrement(&native_presses);
        } else { InterlockedIncrement(&native_releases); }
    }
}
static void __cdecl native_poll_observer(void) { InterlockedIncrement(&native_notifications); }
static void pump_input_window(void) {
    MSG message;
    while (PeekMessageA(&message,0,0,0,PM_REMOVE)) {
        TranslateMessage(&message);DispatchMessageA(&message);
    }
}
static void check_native_poll(unsigned char *image, HMODULE compiled, int side, HWND window, int ring_index) {
    typedef void (__cdecl *PollFn)(void);
    typedef void (__cdecl *KeyFn)(int,int);
    typedef void (CALLBACK *TimerFn)(UINT,UINT,DWORD_PTR,DWORD_PTR,DWORD_PTR);
    union { unsigned char *bytes; FARPROC generic; PollFn poll; TimerFn timer; } binding;
    PollFn poll;
    TimerFn timer;
    KeyFn volatile *key_callback;
    PollFn volatile *poll_callback;
    volatile unsigned char *held, *events, *queue;
    INPUT event;
    unsigned char *bytes;
    unsigned int k, attempt;
    int failed;
#ifdef SURFACE_TEST_TIMED
    typedef int (__cdecl *ReadFn)(int);
    union { unsigned char *bytes; FARPROC generic; ReadFn read; } reader;
    ReadFn read_held, consume_press, consume_repeat;
    volatile UINT *timer_id;
    UINT active_id;
    LONG notification_count;
#endif

    if (side==0) {
        binding.bytes=image+0x55ef0;poll=binding.poll;
        binding.bytes=image+0x56160;timer=binding.timer;
        key_callback=(KeyFn volatile *)(image+0x10de14);
        poll_callback=(PollFn volatile *)(image+0x10e678);
        held=image+0x10de60;events=image+0x10df68;queue=image+0x10de18;
    } else {
        binding.generic=symbol(compiled,"Input_PollNative");poll=binding.poll;
        binding.generic=symbol(compiled,"Input_TimerCallbackNative");timer=binding.timer;
        key_callback=(KeyFn volatile *)data_symbol(compiled,"g_inputKeyEventCallback");
        poll_callback=(PollFn volatile *)data_symbol(compiled,"g_inputPollCallback");
        held=(volatile unsigned char *)data_symbol(compiled,"g_nativeKeyboardHeld");
        events=(volatile unsigned char *)data_symbol(compiled,"g_nativeKeyboardPressEvents");
        queue=(volatile unsigned char *)data_symbol(compiled,"g_nativeCharacterQueue");
    }
    native_presses=0;native_releases=0;native_notifications=0;
    native_active_queue=queue;
    native_active_cursor=side==0 ? (volatile unsigned char *volatile *)(image+0x10de10) :
        (volatile unsigned char *volatile *)data_symbol(compiled,"g_nativeCharacterQueueCursor");
    native_character_index=0xffffffffu;
#ifdef SURFACE_TEST_TIMED
    if (side==0) {
        reader.bytes=image+0x56070;read_held=reader.read;
        reader.bytes=image+0x56080;consume_press=reader.read;
        reader.bytes=image+0x560a0;consume_repeat=reader.read;
        timer_id=(volatile UINT *)(image+0xba658);
    } else {
        reader.generic=symbol(compiled,"Input_ReadHeldNative");read_held=reader.read;
        reader.generic=symbol(compiled,"Input_ConsumePressNative");consume_press=reader.read;
        reader.generic=symbol(compiled,"Input_ConsumeRepeatNative");consume_repeat=reader.read;
        timer_id=(volatile UINT *)data_symbol(compiled,"g_nativeInputTimerId");
    }
    active_id=*timer_id;
    if (!active_id) { fail("FAIL native timed input ID\n",69); }
#endif
    *key_callback=native_key_observer;*poll_callback=native_poll_observer;
    SetForegroundWindow(window);SetFocus(window);pump_input_window();
    for (attempt=0; attempt<10; attempt++) {
#ifndef SURFACE_TEST_TIMED
        poll();
#endif
        Sleep(10);pump_input_window();
    }
    /* Foreground DirectInput is a real prerequisite. Observe the slot at the
     * actual A callback instead of assuming unrelated events leave cursor0. */
    if (GetForegroundWindow()!=window) { fail("FAIL native keyboard foreground prerequisite\n",74); }
    native_presses=0;native_releases=0;native_character_index=0xffffffffu;
    *native_active_cursor=queue+ring_index;
    bytes=(unsigned char *)&event;
    for (k=0; k<sizeof(event); k++) { bytes[k]=0; }
    event.type=INPUT_KEYBOARD;event.ki.wScan=30;event.ki.dwFlags=KEYEVENTF_SCANCODE;
    if (SendInput(1,&event,sizeof(event))!=1) { fail("FAIL native key injection\n",65); }
    for (attempt=0; attempt<100 && !native_presses; attempt++) {
        Sleep(10);pump_input_window();
#ifndef SURFACE_TEST_TIMED
        poll();
#endif
    }
    failed=!native_presses || held[30]!=1 || events[30]!=1;
#ifdef SURFACE_TEST_TIMED
    if (!failed && (read_held(0x1234001e)!=1 || consume_press(0x1234001e)!=1 ||
            consume_press(30)!=0 || consume_repeat(0x1234001e)!=1 || consume_repeat(30)!=0)) {
        failed=1;
    }
#endif
    event.ki.dwFlags=KEYEVENTF_SCANCODE|KEYEVENTF_KEYUP;
    if (SendInput(1,&event,sizeof(event))!=1) { fail("FAIL native key release injection\n",66); }
    for (attempt=0; attempt<100 && !native_releases; attempt++) {
        Sleep(10);pump_input_window();
#ifndef SURFACE_TEST_TIMED
        poll();
#endif
    }
    if (failed || !native_releases || held[30]!=0 || events[30]!=0 || native_character_index>=32 ||
            queue[native_character_index]!='A' || queue[native_character_index+32]!='A') {
        input_diagnostic_number("native input side=",(uint32_t)side);
        input_diagnostic_number("native input press check failed=",(uint32_t)failed);
        input_diagnostic_number("native input presses=",(uint32_t)native_presses);
        input_diagnostic_number("native input releases=",(uint32_t)native_releases);
        input_diagnostic_number("native input held=",held[30]);input_diagnostic_number("native input event=",events[30]);
        input_diagnostic_number("native input queue first=",queue[0]);input_diagnostic_number("native input queue mirror=",queue[32]);
        input_diagnostic_number("native input observed A slot=",native_character_index);
        input_diagnostic_number("native input foreground matches=",(uint32_t)(GetForegroundWindow()==window));
        fail("FAIL actual buffered native key press/release/ring\n",67);
    }
#ifdef SURFACE_TEST_TIMED
    /* Test-owned quiescence boundary: the authentic shutdown releases devices
     * before killing its timer. Do not claim arbitrary asynchronous teardown. */
    if (timeKillEvent(active_id)!=TIMERR_NOERROR) { fail("FAIL native timed input cancellation\n",70); }
    Sleep(50);
    notification_count=native_notifications;
    if (!notification_count || read_held(30)!=0 || consume_press(30)!=0 || consume_repeat(30)!=0) {
        fail("FAIL native asynchronous notification/consumers\n",71);
    }
#endif
    timer(1,2,3,4,5);
#ifdef SURFACE_TEST_TIMED
    if (native_notifications!=notification_count+1) { fail("FAIL native timed callback ABI\n",72); }
#else
    if (native_notifications!=1) { fail("FAIL actual native timer callback ABI/notification\n",68); }
#endif
    *key_callback=0;*poll_callback=0;
#ifdef SURFACE_TEST_TIMED
    (void)poll;
#endif
}
#endif

static void check_records(NativeRecord *primary, NativeRecord *bank1,
        NativeRecord *bank2, int count, int success, uint32_t depth) {
    NativeRecord *record;
    DDSURFACEDESC desc;
    int bank, index, n, k, active;
    for (bank=0; bank<3; bank++) {
        n = bank==0 ? 1 : bank==1 ? 5 : 20;
        for (index=0; index<n; index++) {
            record = bank==0 ? primary : bank==1 ? &bank1[index] : &bank2[index];
            active = success && (bank==0 || (bank==1 && index<count));
            if (record->kind!=(uint32_t)bank || record->active!=(uint32_t)active ||
                record->marker!=(uint32_t)active || (!!record->surface)!=active ||
                record->width!=(uint32_t)(active ? 640 : 0) ||
                record->height!=(uint32_t)(active ? 480 : 0) ||
                record->depth!=(active ? depth : 0)) {
                fail("FAIL native record state\n", 31);
            }
            for (k=0; k<5; k++) {
                if (record->opaque[k]!=0) { fail("FAIL native opaque words\n", 32); }
            }
            if (active) {
                unsigned char *bytes;
                bytes=(unsigned char *)&desc;
                for (k=0; k<(int)sizeof(desc); k++) { bytes[k]=0; }
                desc.dwSize=sizeof(desc);
                if (record->surface->lpVtbl->GetSurfaceDesc(record->surface,&desc)!=DD_OK) {
                    fail("FAIL native surface descriptor\n", 33);
                }
                if (bank==1 && (desc.dwWidth!=640 || desc.dwHeight!=480 ||
                    !(desc.ddsCaps.dwCaps&DDSCAPS_OFFSCREENPLAIN))) {
                    fail("FAIL native offscreen surface dimensions/caps\n", 34);
                }
                if (bank==0 && !(desc.ddsCaps.dwCaps&DDSCAPS_PRIMARYSURFACE)) {
                    fail("FAIL native primary surface caps\n", 35);
                }
            }
        }
    }
}

void WINAPI NativeEntry(void) {
    HANDLE file;
    DWORD size, read, delta, reloc_offset, reloc_end, target;
    unsigned char *raw, *image;
    IMAGE_DOS_HEADER *dos;
    IMAGE_NT_HEADERS32 *nt;
    IMAGE_SECTION_HEADER *section;
    IMAGE_BASE_RELOCATION *reloc;
    WORD *entries;
    uint32_t *word;
    unsigned int n, j;
    HMODULE compiled, module;
    IMAGE_IMPORT_DESCRIPTOR *import;
    IMAGE_THUNK_DATA32 *lookup, *iat;
    IMAGE_IMPORT_BY_NAME *entry;
    FARPROC procedure;
    WNDCLASSA window_class;
    HINSTANCE instance;
    HCURSOR cursor;
    OpenFn original_fn, rebuilt_fn;
#ifdef SURFACE_TEST_INPUT
#ifndef SURFACE_TEST_TIMED
    InputInitFn input_init[2];
#endif
#ifdef SURFACE_TEST_TIMED
    InputTimedInitFn input_timed[2];
    OpenFn input_reinit[2];
#endif
    OpenFn input_shutdown[2];
    volatile LPDIRECTINPUTA *inputs[2];
    volatile LPDIRECTINPUTDEVICEA *keyboards[2];
    volatile uint32_t *acquired[2], *timer_active[2], *timer_id[2];
    int input_cycle;
    UINT owned_timer;
    uint32_t expected_timer_active;
#endif
#if defined(SURFACE_TEST_SHUTDOWN) || defined(SURFACE_TEST_REBUILD)
    OpenFn shutdown_fn[2];
#ifdef SURFACE_TEST_REBUILD
    OpenFn finalize_fn[2];
    LPDIRECTDRAWSURFACE retained_surfaces[6];
    LPDIRECTDRAW retained_draw;
#endif
    LPDIRECTDRAWCLIPPER retained_clipper;
    DDSURFACEDESC extra_desc;
#endif
    NativeRecord *records[2][3];
    volatile HINSTANCE *instances[2];
    volatile HWND *windows[2], *duplicates[2];
    volatile LPDIRECTDRAW *draws[2];
    volatile LPDIRECTDRAWCLIPPER *clippers[2];
    volatile LPDIRECTDRAWPALETTE *palettes[2];
    volatile int32_t *modes[2], *widths[2], *heights[2], *depths[2], *counts[2];
    int pass, side, bank, count, result, k;
#if !defined(SURFACE_TEST_SHUTDOWN) && !defined(SURFACE_TEST_REBUILD)
    int index;
#endif
    HWND window, clip_window;
    RECT client;
    LPDIRECTDRAWCLIPPER installed;
    LONG style;
    uint32_t *words;
    union { unsigned char *bytes; OpenFn open; WNDPROC window; FARPROC generic;
#ifdef SURFACE_TEST_INPUT
        InputInitFn input;
#ifdef SURFACE_TEST_TIMED
        InputTimedInitFn timed;
#endif
#endif
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
    /* Bind only actual platform imports used by the constructor/reference
     * WndProc. Game/input/CRT startup is not invoked or replaced by success. */
    import=(IMAGE_IMPORT_DESCRIPTOR *)(image+nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
    while (import->Name) {
        const char *name;
        name=(const char *)(image+import->Name);
        if (equal_text(name,"USER32.dll") || equal_text(name,"GDI32.dll") || equal_text(name,"DDRAW.dll")
#ifdef SURFACE_TEST_INPUT
                || equal_text(name,"DINPUT.dll") || equal_text(name,"WINMM.dll")
#endif
                ) {
            module=LoadLibraryA(name);
            if (!module) { fail("FAIL native platform module\n",36); }
            lookup=(IMAGE_THUNK_DATA32 *)(image+import->OriginalFirstThunk);
            iat=(IMAGE_THUNK_DATA32 *)(image+import->FirstThunk);
            while (lookup->u1.AddressOfData) {
                if (lookup->u1.Ordinal&IMAGE_ORDINAL_FLAG32) { fail("FAIL unexpected platform ordinal\n",37); }
                entry=(IMAGE_IMPORT_BY_NAME *)(image+lookup->u1.AddressOfData);
                procedure=symbol(module,(const char *)entry->Name);
                iat->u1.Function=(DWORD)procedure;
                lookup++; iat++;
            }
        }
        import++;
    }
    compiled=LoadLibraryA("surface_open_validation.dll");
    if (!compiled) { fail("FAIL native production DLL load\n",38); }
#ifdef SURFACE_TEST_ENTRY
    /* Execute the real surface installer. Sprite installation also allocates
     * workspaces, outside this host's scope; bind only its authenticated two
     * lifecycle slots to their real original/rebuilt bodies. */
    binding.bytes=image+0x5b690;
    if (binding.open()!=1) { fail("FAIL native original surface installer\n",58); }
    binding.generic=symbol(compiled,"Gfx_InstallSurfaceDispatch");
    if (binding.open()!=1) { fail("FAIL native rebuilt surface installer\n",59); }
    binding.bytes=image+0x56f20;*(OpenFn volatile *)(image+0x10eba0)=binding.open;
    binding.bytes=image+0x56f30;*(OpenFn volatile *)(image+0x10eba4)=binding.open;
    binding.generic=symbol(compiled,"Gfx_SpriteOpenNative");
    *(OpenFn volatile *)data_symbol(compiled,"g_spriteOpen")=binding.open;
    binding.generic=symbol(compiled,"Gfx_SpriteShutdownNative");
    *(OpenFn volatile *)data_symbol(compiled,"g_spriteShutdown")=binding.open;
    binding.bytes=image+0x56bc0;original_fn=binding.open;
    binding.generic=symbol(compiled,"Gfx_Open");rebuilt_fn=binding.open;
#else
    binding.bytes=image+0x5b740;original_fn=binding.open;
    binding.generic=symbol(compiled,"Gfx_SurfaceOpenNative");rebuilt_fn=binding.open;
#endif
    records[0][0]=(NativeRecord *)(image+0x10e778);
    records[0][1]=(NativeRecord *)(image+0x10e688);
    records[0][2]=(NativeRecord *)(image+0x10e7a8);
    records[1][0]=(NativeRecord *)data_symbol(compiled,"g_nativePrimarySurface");
    records[1][1]=(NativeRecord *)data_symbol(compiled,"g_nativeType1Surfaces");
    records[1][2]=(NativeRecord *)data_symbol(compiled,"g_nativeType2Surfaces");
    instances[0]=(volatile HINSTANCE *)(image+0xc5398);
    windows[0]=(volatile HWND *)(image+0xc539c);duplicates[0]=(volatile HWND *)(image+0x93738);
    draws[0]=(volatile LPDIRECTDRAW *)(image+0x112c50);
    clippers[0]=(volatile LPDIRECTDRAWCLIPPER *)(image+0xbac8c);
    palettes[0]=(volatile LPDIRECTDRAWPALETTE *)(image+0x112448);
    modes[0]=(volatile int32_t *)(image+0x93728);
    widths[0]=(volatile int32_t *)(image+0xba6e0);heights[0]=(volatile int32_t *)(image+0xba6e4);
    depths[0]=(volatile int32_t *)(image+0xba6e8);counts[0]=(volatile int32_t *)(image+0xba6ec);
    instances[1]=(volatile HINSTANCE *)data_symbol(compiled,"g_nativeWindowInstance");
    windows[1]=(volatile HWND *)data_symbol(compiled,"g_nativeWindow");duplicates[1]=(volatile HWND *)data_symbol(compiled,"g_nativeSurfaceWindow");
    draws[1]=(volatile LPDIRECTDRAW *)data_symbol(compiled,"g_nativeDirectDraw");
    clippers[1]=(volatile LPDIRECTDRAWCLIPPER *)data_symbol(compiled,"g_nativeClipper");
    palettes[1]=(volatile LPDIRECTDRAWPALETTE *)data_symbol(compiled,"g_nativePalette");
    modes[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeFullscreen");
    widths[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceWidth");
    heights[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceHeight");
    depths[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeSurfaceBitDepth");
    counts[1]=(volatile int32_t *)data_symbol(compiled,"g_nativeBackbufferCount");
#ifdef SURFACE_TEST_INPUT
#ifndef SURFACE_TEST_TIMED
    binding.bytes=image+0x55ac0;input_init[0]=binding.input;
    binding.generic=symbol(compiled,"Input_InitNative");input_init[1]=binding.input;
#endif
    binding.bytes=image+0x55e20;input_shutdown[0]=binding.open;
    binding.generic=symbol(compiled,"Input_ShutdownNative");input_shutdown[1]=binding.open;
#ifdef SURFACE_TEST_TIMED
    binding.bytes=image+0x55c60;input_timed[0]=binding.timed;
    binding.generic=symbol(compiled,"Input_InitTimedNative");input_timed[1]=binding.timed;
    binding.bytes=image+0x55eb0;input_reinit[0]=binding.open;
    binding.generic=symbol(compiled,"Input_ReinitializeNative");input_reinit[1]=binding.open;
    binding.bytes=image+0x55ea0;input_shutdown[0]=binding.open;
    binding.generic=symbol(compiled,"Input_ShutdownNativeEntry");input_shutdown[1]=binding.open;
#endif
    inputs[0]=(volatile LPDIRECTINPUTA *)(image+0x10df60);
    keyboards[0]=(volatile LPDIRECTINPUTDEVICEA *)(image+0x10e26c);
    acquired[0]=(volatile uint32_t *)(image+0x10e270);
    timer_active[0]=(volatile uint32_t *)(image+0xba65c);timer_id[0]=(volatile uint32_t *)(image+0xba658);
    inputs[1]=(volatile LPDIRECTINPUTA *)data_symbol(compiled,"g_nativeDirectInput");
    keyboards[1]=(volatile LPDIRECTINPUTDEVICEA *)data_symbol(compiled,"g_nativeKeyboardDevice");
    acquired[1]=(volatile uint32_t *)data_symbol(compiled,"g_nativeKeyboardAcquired");
    timer_active[1]=(volatile uint32_t *)data_symbol(compiled,"g_nativeInputTimerActive");
    timer_id[1]=(volatile uint32_t *)data_symbol(compiled,"g_nativeInputTimerId");
#endif
#if defined(SURFACE_TEST_SHUTDOWN) || defined(SURFACE_TEST_REBUILD)
#ifdef SURFACE_TEST_REBUILD
#ifdef SURFACE_TEST_ENTRY
    binding.bytes=image+0x56be0;shutdown_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_Rebuild");shutdown_fn[1]=binding.open;
    binding.bytes=image+0x56bf0;finalize_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_Shutdown");finalize_fn[1]=binding.open;
#else
    binding.bytes=image+0x5bd70;shutdown_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_SurfaceRebuildNative");shutdown_fn[1]=binding.open;
    binding.bytes=image+0x5c060;finalize_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_SurfaceShutdownNative");finalize_fn[1]=binding.open;
#endif
#else
    binding.bytes=image+0x5c060;shutdown_fn[0]=binding.open;
    binding.generic=symbol(compiled,"Gfx_SurfaceShutdownNative");shutdown_fn[1]=binding.open;
#endif
#endif
    instance=GetModuleHandleA(0);
    cursor=LoadCursorA(0,MAKEINTRESOURCEA(32512));
    *(HCURSOR *)(image+0xc5360)=cursor;
    /* Prevent reference WndProc from entering unreconstructed game shutdown;
     * no Alt-Enter/input path is sent. Native constructor scope is explicit. */
    *(uint32_t *)(image+0x9372c)=1;
    *(uint32_t *)(image+0xc5390)=0;
    for (pass=0; pass<4; pass++) {
        if (pass==1) {
            unsigned char *bytes;
            bytes=(unsigned char *)&window_class;
            for (k=0; k<(int)sizeof(window_class); k++) { bytes[k]=0; }
            window_class.style=8;
            binding.bytes=image+0x122d0;window_class.lpfnWndProc=binding.window;
            window_class.hInstance=instance;
            window_class.hIcon=LoadIconA(0,MAKEINTRESOURCEA(32512));
            window_class.hCursor=cursor;
            window_class.hbrBackground=(HBRUSH)GetStockObject(4);
            window_class.lpszClassName=(const char *)(image+0x93740);
            if (!RegisterClassA(&window_class)) { fail("FAIL native original class registration\n",39); }
        }
        count=pass<2 ? 0 : pass==2 ? 1 : 4;
        for (side=0; side<2; side++) {
            for (bank=0; bank<3; bank++) {
                words=(uint32_t *)records[side][bank];
                n=bank==0 ? 12 : bank==1 ? 60 : 240;
                for (j=0; j<n; j++) { words[j]=0xa5a50000u+j; }
            }
            *instances[side]=instance;*windows[side]=0;*duplicates[side]=0;
            *draws[side]=0;*clippers[side]=0;*palettes[side]=0;
            *modes[side]=0;*widths[side]=640;*heights[side]=480;*depths[side]=8;*counts[side]=count;
            /* Both constructors use the exact same authenticated reference
             * procedure/class. Its global mode must match the test caller. */
            *modes[0]=0;
            result=side==0 ? original_fn() : rebuilt_fn();
            if (result!=(pass!=0)) { fail("FAIL native constructor result\n",40); }
            check_records(records[side][0],records[side][1],records[side][2],count,pass!=0,8);
            if (pass==0) {
#ifdef SURFACE_TEST_INPUT
                if (input_shutdown[side]()!=1 || *inputs[side] || *keyboards[side] || *acquired[side]) {
                    fail("FAIL empty native input shutdown\n",60);
                }
#endif
                if (*windows[side] || *duplicates[side] || *draws[side]) { fail("FAIL missing-class retained state\n",41); }
#ifdef SURFACE_TEST_SHUTDOWN
                if (shutdown_fn[side]()!=1 || *draws[side] || *clippers[side]) {
                    fail("FAIL empty native shutdown\n",50);
                }
#endif
                continue;
            }
            window=*windows[side];
            if (!window || *duplicates[side]!=window || !IsWindow(window) || !IsWindowVisible(window)) {
                fail("FAIL native HWND publication/show\n",42);
            }
            if (!GetClientRect(window,&client) || client.right-client.left!=640 || client.bottom-client.top!=480) {
                fail("FAIL native client geometry\n",43);
            }
            style=GetWindowLongA(window,GWL_STYLE);
            if ((style&WS_POPUP) || (style&0xc60000)!=0xc60000) { fail("FAIL native window style\n",44); }
            if (!*draws[side] || !*clippers[side] || *palettes[side]) { fail("FAIL native interface ownership\n",45); }
            if ((*clippers[side])->lpVtbl->GetHWnd(*clippers[side],&clip_window)!=DD_OK || clip_window!=window) {
                fail("FAIL native clipper HWND\n",46);
            }
            installed=0;
            if (records[side][0]->surface->lpVtbl->GetClipper(records[side][0]->surface,&installed)!=DD_OK || !installed) {
                fail("FAIL native installed clipper\n",47);
            }
            installed->lpVtbl->Release(installed);
#ifdef SURFACE_TEST_CLEAR
            if (count) { check_native_clear(image,compiled,side,records[side][1]); }
#endif
#ifdef SURFACE_TEST_ACCESS
            check_native_access(image,compiled,side,records[side][0]);
            if (count) { check_native_access(image,compiled,side,records[side][1]); }
#endif
#ifdef SURFACE_TEST_INPUT
            for (input_cycle=0; input_cycle<2; input_cycle++) {
#ifdef SURFACE_TEST_TIMED
                result=input_cycle==0 ? input_timed[side](10,300,30) : input_reinit[side]();
                if (result!=1 || !*inputs[side] || !*keyboards[side] || *acquired[side]>1 ||
                        *timer_active[side]!=1 || !*timer_id[side]) {
                    fail("FAIL actual timed input setup/reinit\n",73);
                }
#else
                if (input_init[side](300,30)!=1 || !*inputs[side] || !*keyboards[side] || *acquired[side]>1 ||
                        *timer_active[side] || *timer_id[side]) {
                    fail("FAIL actual DirectInput setup\n",61);
                }
#endif
                owned_timer=0;
#ifdef SURFACE_TEST_POLL
                check_native_poll(image,compiled,side,window,input_cycle==0 ? 13 : 31);
#endif
#ifdef SURFACE_TEST_TIMED
                owned_timer=*timer_id[side];
                expected_timer_active=1;
#else
                expected_timer_active=(uint32_t)(input_cycle!=0);
                if (input_cycle!=0) {
                    /* Test-owned real multimedia timer; production timed input
                     * initialization/poll callback remains outside this scope. */
                    owned_timer=timeSetEvent(10,1,input_timer_probe,0,TIME_PERIODIC);
                    if (!owned_timer) { fail("FAIL native timer setup\n",62); }
                    *timer_active[side]=1;*timer_id[side]=owned_timer;
                }
#endif
                if (input_shutdown[side]()!=1 || *inputs[side] || *keyboards[side] || *acquired[side] ||
                        *timer_active[side]!=expected_timer_active || *timer_id[side]!=owned_timer || !IsWindow(window)) {
                    fail("FAIL native input shutdown state\n",63);
                }
                if (owned_timer && timeKillEvent(owned_timer)==TIMERR_NOERROR) {
                    fail("FAIL native production timer cancellation\n",64);
                }
            }
#endif
#if defined(SURFACE_TEST_SHUTDOWN) || defined(SURFACE_TEST_REBUILD)
            {
                unsigned char *bytes;
                bytes=(unsigned char *)&extra_desc;
                for (k=0; k<(int)sizeof(extra_desc); k++) { bytes[k]=0; }
                extra_desc.dwSize=sizeof(extra_desc);
                extra_desc.dwFlags=DDSD_CAPS|DDSD_WIDTH|DDSD_HEIGHT;
                extra_desc.dwWidth=32;extra_desc.dwHeight=16;
                extra_desc.ddsCaps.dwCaps=DDSCAPS_OFFSCREENPLAIN;
                for (j=0; j<20; j+=19) {
                    NativeRecord *record;
                    record=&records[side][2][j];
                    if ((*draws[side])->lpVtbl->CreateSurface(*draws[side],&extra_desc,&record->surface,0)!=DD_OK) {
                        fail("FAIL native type2 surface setup\n",53);
                    }
                    record->active=0;record->marker=0x12345678;
                    record->width=32;record->height=16;record->depth=8;
                }
            }
#ifdef SURFACE_TEST_REBUILD
            retained_draw=*draws[side];
            retained_surfaces[0]=records[side][0]->surface;
            for (j=0; j<(unsigned int)count; j++) { retained_surfaces[j+1]=records[side][1][j].surface; }
#endif
            retained_clipper=*clippers[side];
#ifdef SURFACE_TEST_REBUILD
            if (shutdown_fn[side]()!=1 || *draws[side]!=retained_draw || *clippers[side]!=retained_clipper ||
                    *windows[side]!=window || !IsWindowVisible(window)) {
                fail("FAIL native rebuild retained globals\n",54);
            }
            check_records(records[side][0],records[side][1],records[side][2],0,0,8);
            if (shutdown_fn[side]()!=1 || *draws[side]!=retained_draw) { fail("FAIL repeated native rebuild\n",55); }
            check_records(records[side][0],records[side][1],records[side][2],0,0,8);
            for (j=0; j<(unsigned int)count+1; j++) {
                unsigned char *bytes;
                bytes=(unsigned char *)&extra_desc;
                for (k=0; k<(int)sizeof(extra_desc); k++) { bytes[k]=0; }
                extra_desc.dwSize=sizeof(extra_desc);
                if (retained_surfaces[j]->lpVtbl->GetSurfaceDesc(retained_surfaces[j],&extra_desc)!=DD_OK) {
                    fail("FAIL native rebuild unexpectedly released retained surface\n",56);
                }
                retained_surfaces[j]->lpVtbl->Release(retained_surfaces[j]);
            }
            if (finalize_fn[side]()!=1 || *draws[side] || *clippers[side]!=retained_clipper ||
                    !IsWindow(window)) {
                fail("FAIL native rebuild/shutdown production sequence\n",57);
            }
#else
            if (shutdown_fn[side]()!=1 || *draws[side] || *clippers[side]!=retained_clipper ||
                    *windows[side]!=window || !IsWindow(window)) {
                fail("FAIL native shutdown retained globals\n",51);
            }
            for (bank=0; bank<3; bank++) {
                n=bank==0 ? 1 : bank==1 ? 5 : 20;
                for (j=0; j<n; j++) {
                    NativeRecord *record;
                    int prior_active, extra;
                    record=&records[side][bank][j];
                    prior_active=bank==0 || (bank==1 && (int)j<count);
                    extra=bank==2 && (j==0 || j==19);
                    if (record->active || record->surface || record->kind!=(uint32_t)bank ||
                            record->marker!=(extra ? 0x12345678u : (uint32_t)prior_active) ||
                            record->width!=(uint32_t)(extra ? 32 : prior_active ? 640 : 0) ||
                            record->height!=(uint32_t)(extra ? 16 : prior_active ? 480 : 0) ||
                            record->depth!=(uint32_t)(extra || prior_active ? 8 : 0)) {
                        fail("FAIL native shutdown record retention\n",52);
                    }
                }
            }
#endif
#else
            /* Test-owned cleanup. Rebuilt native shutdown remains a next
             * feature; this cleanup is not production behavior or evidence. */
            for (index=0; index<count; index++) {
                records[side][1][index].surface->lpVtbl->Release(records[side][1][index].surface);
            }
            records[side][0]->surface->lpVtbl->Release(records[side][0]->surface);
            (*clippers[side])->lpVtbl->Release(*clippers[side]);
            (*draws[side])->lpVtbl->Release(*draws[side]);
            *draws[side]=0;*clippers[side]=0;
#endif
            if (!DestroyWindow(window)) { fail("FAIL native test window disposal\n",48); }
        }
    }
    if (!UnregisterClassA("Ignition",instance)) { fail("FAIL native class disposal\n",49); }
#ifdef SURFACE_TEST_CLEAR
    output("PASS native Win32/DirectDraw original versus C89 clear: bounded offscreen DWORD-scaled byte-pitch writes, mode restoration and real installed-table entry; 16 paired clears\n");
#elif defined(SURFACE_TEST_ACCESS)
    output("PASS native Win32/DirectDraw original versus C89 access: real lock roles/pitch, same-mode and changed-mode access, surface-field-address unlock, stale pointers/pitch and repeated unlock; primary/backbuffers in paired windowed0/1/4 scenarios\n");
#elif defined(SURFACE_TEST_INPUT)
#ifdef SURFACE_TEST_TIMED
    output("PASS native Win32/DirectDraw original versus C89 timed: actual asynchronous timer polling, buffered scan30, consumers, shutdown entry and reinit; paired windowed0/1/4; test-owned timer quiescence before teardown\n");
#else
#ifdef SURFACE_TEST_POLL
    output("PASS native Win32/DirectDraw original versus C89 polling: real buffered key press/release, mirrored character ring and timer callback; paired windowed 0/1/4 lifecycles; original WndProc reference\n");
#else
    output("PASS native Win32/DirectDraw original versus C89 input: real DirectInput setup/shutdown and multimedia timer cancellation; paired windowed 0/1/4 scenarios, empty shutdown; original WndProc reference; no native polling/game parity\n");
#endif
#endif
#elif defined(SURFACE_TEST_SHUTDOWN) || defined(SURFACE_TEST_REBUILD)
#ifdef SURFACE_TEST_REBUILD
#ifdef SURFACE_TEST_ENTRY
    output("PASS native Win32/DirectDraw original versus C89 entry: real surface installer, graphics open/repeated rebuild/shutdown and real sprite leaves; missing class and windowed 0/1/4 backbuffers; original WndProc reference boundary; no native game parity\n");
#else
    output("PASS native Win32/DirectDraw original versus C89 rebuild: windowed 0/1/4 backbuffers and type2 boundary releases; repeated real rebuild clears records while retaining primary/type1 interfaces; original WndProc reference boundary; no native game parity\n");
#endif
#else
    output("PASS native Win32/DirectDraw original versus C89 shutdown: missing class and 0/1/4 windowed backbuffers; real production releases and retained clipper/record metadata; original WndProc reference boundary; no native game parity\n");
#endif
#else
    output("PASS native Win32/DirectDraw original versus C89 constructor: missing class and 0/1/4 windowed backbuffers; authenticated original WndProc reference boundary; no native game parity\n");
#endif
    ExitProcess(0);
}
