/* Verified Windows keyboard input. Reconstruction module input.c;
 * original translation-unit filename unknown. See windows_input_module.md.
 * State types are private; validation exports are explicit in the harness. */
#include "input.h"
#include <stddef.h>

typedef union NativeCharacterRing {
    unsigned char bytes[64];
    uint32_t words[16];
} NativeCharacterRing;

typedef union NativeKeyboardByteState {
    unsigned char bytes[256];
    uint32_t words[64];
} NativeKeyboardByteState;

/* Windows input callback state. Both words are loader-zeroed .data tail. */
InputKeyEventCallback volatile g_inputKeyEventCallback = 0; /* VA 0x0050DE14 */
InputPollCallback volatile g_inputPollCallback = 0;         /* VA 0x0050E678 */

/*
 * @original Input_ResetCallbacks (IGN_WIN.EXE @ 0x00455AB0, inferred input.c)
 * @fidelity EXACT
 */
void Input_ResetCallbacks(void) {
    g_inputKeyEventCallback = 0;
    g_inputPollCallback = 0;
}

/* Native DirectInput lifecycle implementation. */
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0300
#endif
#include <dinput.h>
#include <mmsystem.h>
LPDIRECTINPUTA volatile g_nativeDirectInput; /* VA 0x0050DF60 */
LPDIRECTINPUTDEVICEA volatile g_nativeKeyboardDevice; /* VA 0x0050E26C */
volatile int32_t g_nativeKeyboardAcquired; /* VA 0x0050E270 */
volatile uint32_t g_nativeInputTimerActive; /* VA 0x004BA65C */
volatile UINT g_nativeInputTimerId; /* VA 0x004BA658 */

/*
 * @original Input_ShutdownNative (IGN_WIN.EXE @ 0x00455E20, inferred input.c)
 * @fidelity EXACT
 */
int Input_ShutdownNative(void) {
    LPDIRECTINPUTDEVICEA keyboard;
    LPDIRECTINPUTA input;

    if (g_nativeKeyboardAcquired != 0) {
        keyboard = g_nativeKeyboardDevice;
        keyboard->lpVtbl->Unacquire(keyboard);
        g_nativeKeyboardAcquired = 0;
    }
    if (g_nativeKeyboardDevice != 0) {
        keyboard = g_nativeKeyboardDevice;
        keyboard->lpVtbl->Release(keyboard);
        g_nativeKeyboardDevice = 0;
    }
    if (g_nativeDirectInput != 0) {
        input = g_nativeDirectInput;
        input->lpVtbl->Release(input);
        g_nativeDirectInput = 0;
    }
    if (g_nativeInputTimerActive != 0) {
        timeKillEvent(g_nativeInputTimerId);
    }
    return 1;
}

const GUID g_nativeKeyboardDeviceGuid = {
    0x6f1d2b61, 0xd5a0, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0, 0}
};
const GUID g_nativeKeyGuid = {
    0x55728220, 0xd33c, 0x11cf, {0xbf, 0xc7, 0x44, 0x45, 0x53, 0x54, 0, 0}
};
#define NATIVE_KEY_FORMAT(index) {&g_nativeKeyGuid, index, 0x8000000cu | ((index) << 8), 0}
const DIOBJECTDATAFORMAT g_nativeKeyboardObjects[256] = {
    NATIVE_KEY_FORMAT(0u), NATIVE_KEY_FORMAT(1u), NATIVE_KEY_FORMAT(2u), NATIVE_KEY_FORMAT(3u), NATIVE_KEY_FORMAT(4u), NATIVE_KEY_FORMAT(5u), NATIVE_KEY_FORMAT(6u), NATIVE_KEY_FORMAT(7u),
    NATIVE_KEY_FORMAT(8u), NATIVE_KEY_FORMAT(9u), NATIVE_KEY_FORMAT(10u), NATIVE_KEY_FORMAT(11u), NATIVE_KEY_FORMAT(12u), NATIVE_KEY_FORMAT(13u), NATIVE_KEY_FORMAT(14u), NATIVE_KEY_FORMAT(15u),
    NATIVE_KEY_FORMAT(16u), NATIVE_KEY_FORMAT(17u), NATIVE_KEY_FORMAT(18u), NATIVE_KEY_FORMAT(19u), NATIVE_KEY_FORMAT(20u), NATIVE_KEY_FORMAT(21u), NATIVE_KEY_FORMAT(22u), NATIVE_KEY_FORMAT(23u),
    NATIVE_KEY_FORMAT(24u), NATIVE_KEY_FORMAT(25u), NATIVE_KEY_FORMAT(26u), NATIVE_KEY_FORMAT(27u), NATIVE_KEY_FORMAT(28u), NATIVE_KEY_FORMAT(29u), NATIVE_KEY_FORMAT(30u), NATIVE_KEY_FORMAT(31u),
    NATIVE_KEY_FORMAT(32u), NATIVE_KEY_FORMAT(33u), NATIVE_KEY_FORMAT(34u), NATIVE_KEY_FORMAT(35u), NATIVE_KEY_FORMAT(36u), NATIVE_KEY_FORMAT(37u), NATIVE_KEY_FORMAT(38u), NATIVE_KEY_FORMAT(39u),
    NATIVE_KEY_FORMAT(40u), NATIVE_KEY_FORMAT(41u), NATIVE_KEY_FORMAT(42u), NATIVE_KEY_FORMAT(43u), NATIVE_KEY_FORMAT(44u), NATIVE_KEY_FORMAT(45u), NATIVE_KEY_FORMAT(46u), NATIVE_KEY_FORMAT(47u),
    NATIVE_KEY_FORMAT(48u), NATIVE_KEY_FORMAT(49u), NATIVE_KEY_FORMAT(50u), NATIVE_KEY_FORMAT(51u), NATIVE_KEY_FORMAT(52u), NATIVE_KEY_FORMAT(53u), NATIVE_KEY_FORMAT(54u), NATIVE_KEY_FORMAT(55u),
    NATIVE_KEY_FORMAT(56u), NATIVE_KEY_FORMAT(57u), NATIVE_KEY_FORMAT(58u), NATIVE_KEY_FORMAT(59u), NATIVE_KEY_FORMAT(60u), NATIVE_KEY_FORMAT(61u), NATIVE_KEY_FORMAT(62u), NATIVE_KEY_FORMAT(63u),
    NATIVE_KEY_FORMAT(64u), NATIVE_KEY_FORMAT(65u), NATIVE_KEY_FORMAT(66u), NATIVE_KEY_FORMAT(67u), NATIVE_KEY_FORMAT(68u), NATIVE_KEY_FORMAT(69u), NATIVE_KEY_FORMAT(70u), NATIVE_KEY_FORMAT(71u),
    NATIVE_KEY_FORMAT(72u), NATIVE_KEY_FORMAT(73u), NATIVE_KEY_FORMAT(74u), NATIVE_KEY_FORMAT(75u), NATIVE_KEY_FORMAT(76u), NATIVE_KEY_FORMAT(77u), NATIVE_KEY_FORMAT(78u), NATIVE_KEY_FORMAT(79u),
    NATIVE_KEY_FORMAT(80u), NATIVE_KEY_FORMAT(81u), NATIVE_KEY_FORMAT(82u), NATIVE_KEY_FORMAT(83u), NATIVE_KEY_FORMAT(84u), NATIVE_KEY_FORMAT(85u), NATIVE_KEY_FORMAT(86u), NATIVE_KEY_FORMAT(87u),
    NATIVE_KEY_FORMAT(88u), NATIVE_KEY_FORMAT(89u), NATIVE_KEY_FORMAT(90u), NATIVE_KEY_FORMAT(91u), NATIVE_KEY_FORMAT(92u), NATIVE_KEY_FORMAT(93u), NATIVE_KEY_FORMAT(94u), NATIVE_KEY_FORMAT(95u),
    NATIVE_KEY_FORMAT(96u), NATIVE_KEY_FORMAT(97u), NATIVE_KEY_FORMAT(98u), NATIVE_KEY_FORMAT(99u), NATIVE_KEY_FORMAT(100u), NATIVE_KEY_FORMAT(101u), NATIVE_KEY_FORMAT(102u), NATIVE_KEY_FORMAT(103u),
    NATIVE_KEY_FORMAT(104u), NATIVE_KEY_FORMAT(105u), NATIVE_KEY_FORMAT(106u), NATIVE_KEY_FORMAT(107u), NATIVE_KEY_FORMAT(108u), NATIVE_KEY_FORMAT(109u), NATIVE_KEY_FORMAT(110u), NATIVE_KEY_FORMAT(111u),
    NATIVE_KEY_FORMAT(112u), NATIVE_KEY_FORMAT(113u), NATIVE_KEY_FORMAT(114u), NATIVE_KEY_FORMAT(115u), NATIVE_KEY_FORMAT(116u), NATIVE_KEY_FORMAT(117u), NATIVE_KEY_FORMAT(118u), NATIVE_KEY_FORMAT(119u),
    NATIVE_KEY_FORMAT(120u), NATIVE_KEY_FORMAT(121u), NATIVE_KEY_FORMAT(122u), NATIVE_KEY_FORMAT(123u), NATIVE_KEY_FORMAT(124u), NATIVE_KEY_FORMAT(125u), NATIVE_KEY_FORMAT(126u), NATIVE_KEY_FORMAT(127u),
    NATIVE_KEY_FORMAT(128u), NATIVE_KEY_FORMAT(129u), NATIVE_KEY_FORMAT(130u), NATIVE_KEY_FORMAT(131u), NATIVE_KEY_FORMAT(132u), NATIVE_KEY_FORMAT(133u), NATIVE_KEY_FORMAT(134u), NATIVE_KEY_FORMAT(135u),
    NATIVE_KEY_FORMAT(136u), NATIVE_KEY_FORMAT(137u), NATIVE_KEY_FORMAT(138u), NATIVE_KEY_FORMAT(139u), NATIVE_KEY_FORMAT(140u), NATIVE_KEY_FORMAT(141u), NATIVE_KEY_FORMAT(142u), NATIVE_KEY_FORMAT(143u),
    NATIVE_KEY_FORMAT(144u), NATIVE_KEY_FORMAT(145u), NATIVE_KEY_FORMAT(146u), NATIVE_KEY_FORMAT(147u), NATIVE_KEY_FORMAT(148u), NATIVE_KEY_FORMAT(149u), NATIVE_KEY_FORMAT(150u), NATIVE_KEY_FORMAT(151u),
    NATIVE_KEY_FORMAT(152u), NATIVE_KEY_FORMAT(153u), NATIVE_KEY_FORMAT(154u), NATIVE_KEY_FORMAT(155u), NATIVE_KEY_FORMAT(156u), NATIVE_KEY_FORMAT(157u), NATIVE_KEY_FORMAT(158u), NATIVE_KEY_FORMAT(159u),
    NATIVE_KEY_FORMAT(160u), NATIVE_KEY_FORMAT(161u), NATIVE_KEY_FORMAT(162u), NATIVE_KEY_FORMAT(163u), NATIVE_KEY_FORMAT(164u), NATIVE_KEY_FORMAT(165u), NATIVE_KEY_FORMAT(166u), NATIVE_KEY_FORMAT(167u),
    NATIVE_KEY_FORMAT(168u), NATIVE_KEY_FORMAT(169u), NATIVE_KEY_FORMAT(170u), NATIVE_KEY_FORMAT(171u), NATIVE_KEY_FORMAT(172u), NATIVE_KEY_FORMAT(173u), NATIVE_KEY_FORMAT(174u), NATIVE_KEY_FORMAT(175u),
    NATIVE_KEY_FORMAT(176u), NATIVE_KEY_FORMAT(177u), NATIVE_KEY_FORMAT(178u), NATIVE_KEY_FORMAT(179u), NATIVE_KEY_FORMAT(180u), NATIVE_KEY_FORMAT(181u), NATIVE_KEY_FORMAT(182u), NATIVE_KEY_FORMAT(183u),
    NATIVE_KEY_FORMAT(184u), NATIVE_KEY_FORMAT(185u), NATIVE_KEY_FORMAT(186u), NATIVE_KEY_FORMAT(187u), NATIVE_KEY_FORMAT(188u), NATIVE_KEY_FORMAT(189u), NATIVE_KEY_FORMAT(190u), NATIVE_KEY_FORMAT(191u),
    NATIVE_KEY_FORMAT(192u), NATIVE_KEY_FORMAT(193u), NATIVE_KEY_FORMAT(194u), NATIVE_KEY_FORMAT(195u), NATIVE_KEY_FORMAT(196u), NATIVE_KEY_FORMAT(197u), NATIVE_KEY_FORMAT(198u), NATIVE_KEY_FORMAT(199u),
    NATIVE_KEY_FORMAT(200u), NATIVE_KEY_FORMAT(201u), NATIVE_KEY_FORMAT(202u), NATIVE_KEY_FORMAT(203u), NATIVE_KEY_FORMAT(204u), NATIVE_KEY_FORMAT(205u), NATIVE_KEY_FORMAT(206u), NATIVE_KEY_FORMAT(207u),
    NATIVE_KEY_FORMAT(208u), NATIVE_KEY_FORMAT(209u), NATIVE_KEY_FORMAT(210u), NATIVE_KEY_FORMAT(211u), NATIVE_KEY_FORMAT(212u), NATIVE_KEY_FORMAT(213u), NATIVE_KEY_FORMAT(214u), NATIVE_KEY_FORMAT(215u),
    NATIVE_KEY_FORMAT(216u), NATIVE_KEY_FORMAT(217u), NATIVE_KEY_FORMAT(218u), NATIVE_KEY_FORMAT(219u), NATIVE_KEY_FORMAT(220u), NATIVE_KEY_FORMAT(221u), NATIVE_KEY_FORMAT(222u), NATIVE_KEY_FORMAT(223u),
    NATIVE_KEY_FORMAT(224u), NATIVE_KEY_FORMAT(225u), NATIVE_KEY_FORMAT(226u), NATIVE_KEY_FORMAT(227u), NATIVE_KEY_FORMAT(228u), NATIVE_KEY_FORMAT(229u), NATIVE_KEY_FORMAT(230u), NATIVE_KEY_FORMAT(231u),
    NATIVE_KEY_FORMAT(232u), NATIVE_KEY_FORMAT(233u), NATIVE_KEY_FORMAT(234u), NATIVE_KEY_FORMAT(235u), NATIVE_KEY_FORMAT(236u), NATIVE_KEY_FORMAT(237u), NATIVE_KEY_FORMAT(238u), NATIVE_KEY_FORMAT(239u),
    NATIVE_KEY_FORMAT(240u), NATIVE_KEY_FORMAT(241u), NATIVE_KEY_FORMAT(242u), NATIVE_KEY_FORMAT(243u), NATIVE_KEY_FORMAT(244u), NATIVE_KEY_FORMAT(245u), NATIVE_KEY_FORMAT(246u), NATIVE_KEY_FORMAT(247u),
    NATIVE_KEY_FORMAT(248u), NATIVE_KEY_FORMAT(249u), NATIVE_KEY_FORMAT(250u), NATIVE_KEY_FORMAT(251u), NATIVE_KEY_FORMAT(252u), NATIVE_KEY_FORMAT(253u), NATIVE_KEY_FORMAT(254u), NATIVE_KEY_FORMAT(255u)
};
#undef NATIVE_KEY_FORMAT
const DIDATAFORMAT g_nativeKeyboardFormat = {
    24, 16, 2, 256, 256, (LPDIOBJECTDATAFORMAT)g_nativeKeyboardObjects
};
extern HINSTANCE volatile g_nativeWindowInstance;
extern HWND volatile g_nativeWindow;
volatile uint32_t g_nativeInputClock; /* VA49373C, clock ownership pending */
volatile int32_t g_nativeRepeatDelay; /* VA50DF64 */
volatile int32_t g_nativeRepeatInterval; /* VA50E274 */
volatile uint32_t g_nativeRepeatEpoch; /* VA50E268 */
volatile NativeCharacterRing g_nativeCharacterQueue; /* VA50DE18 */
volatile unsigned char *volatile g_nativeCharacterQueueCursor; /* VA50DE10 */
volatile unsigned char g_nativeCharacterQueueEnd; /* VA50DE58 */
volatile NativeKeyboardByteState g_nativeKeyboardHeld; /* VA50DE60 */
volatile NativeKeyboardByteState g_nativeKeyboardPressEvents; /* VA50DF68 */
volatile unsigned char g_nativeKeyboardPressEdges[256]; /* VA50E068, consumed press edges */
volatile unsigned char g_nativeKeyboardPressLatch[256]; /* VA50E168, press latch until release */
volatile uint32_t g_nativeRepeatCounters[256]; /* VA50E278 */

/* @original Input_InitNative (IGN_WIN.EXE @ 0x00455AC0, inferred input.c)
 * @fidelity EXACT
 */
int Input_InitNative(int32_t repeat_delay, int32_t repeat_interval) {
    GUID keyboard_guid;
    DIPROPDWORD buffer_property;
    LPDIRECTINPUTA input;
    LPDIRECTINPUTDEVICEA keyboard;
    HRESULT result;
    HWND window;
    uint32_t now;
    int index;

    keyboard_guid = g_nativeKeyboardDeviceGuid;
    buffer_property.diph.dwSize = 20;
    buffer_property.diph.dwHeaderSize = 16;
    buffer_property.diph.dwObj = 0;
    buffer_property.diph.dwHow = 0;
    buffer_property.dwData = 32;
    if (DirectInputCreateA(g_nativeWindowInstance, 0x300,
            (LPDIRECTINPUTA *)&g_nativeDirectInput, 0) != 0) {
        return 0;
    }
    input = g_nativeDirectInput;
    if (input->lpVtbl->CreateDevice(input, &keyboard_guid,
            (LPDIRECTINPUTDEVICEA *)&g_nativeKeyboardDevice, 0) != 0) {
        return 0;
    }
    keyboard = g_nativeKeyboardDevice;
    if (keyboard->lpVtbl->SetDataFormat(keyboard, &g_nativeKeyboardFormat) != 0) {
        return 0;
    }
    window = g_nativeWindow;
    keyboard = g_nativeKeyboardDevice;
    if (keyboard->lpVtbl->SetCooperativeLevel(keyboard, window, 6) != 0) {
        return 0;
    }
    keyboard = g_nativeKeyboardDevice;
    if (keyboard->lpVtbl->SetProperty(keyboard, DIPROP_BUFFERSIZE,
            &buffer_property.diph) != 0) {
        return 0;
    }
    keyboard = g_nativeKeyboardDevice;
    result = keyboard->lpVtbl->Acquire(keyboard);
    g_nativeKeyboardAcquired = 1;
    if (result < 0) {
        g_nativeKeyboardAcquired = 0;
    }
    g_nativeCharacterQueueCursor = g_nativeCharacterQueue.bytes;
    for (index = 0; index < 16; index++) {
        g_nativeCharacterQueue.words[index] = 0xffffffffu;
    }
    g_nativeCharacterQueueEnd = 0;
    for (index = 0; index < 64; index++) {
        g_nativeKeyboardPressEvents.words[index] = 0;
    }
    for (index = 0; index < 64; index++) {
        g_nativeKeyboardHeld.words[index] = 0;
    }
    for (index = 0; index < 256; index++) {
        g_nativeKeyboardPressEdges[index] = 0;
        g_nativeRepeatCounters[index] = 0;
        g_nativeKeyboardPressLatch[index] = 0;
    }
    now = g_nativeInputClock;
    g_nativeRepeatDelay = repeat_delay;
    g_nativeRepeatInterval = repeat_interval;
    g_nativeRepeatEpoch = now;
    g_nativeInputTimerActive = 0;
    g_nativeInputTimerId = 0;
    return 1;
}

/* Original US keyboard translation, including literal 0xff entries. */
const unsigned char g_nativeCharacterMap[84] = {
    255,255,49,50,51,52,53,54,55,56,57,48,45,61,255,255,
    81,87,69,82,84,89,85,73,79,80,123,125,255,255,65,83,
    68,70,71,72,74,75,76,59,39,255,255,92,90,88,67,86,
    66,78,77,44,46,47,255,42,255,32,255,255,255,255,255,255,
    255,255,255,255,255,255,255,55,56,57,45,52,53,54,43,49,
    50,51,48,46
};

typedef char native_input_record_size[(sizeof(DIDEVICEOBJECTDATA) == 16) ? 1 : -1];
typedef char native_input_data_slot[(offsetof(IDirectInputDeviceAVtbl, GetDeviceData) == 0x28) ? 1 : -1];
typedef char native_character_ring_size[(sizeof(NativeCharacterRing) == 64) ? 1 : -1];

/* @original Input_DispatchKeyNative (IGN_WIN.EXE @ 0x004560C0, inferred input.c)
 * @fidelity EXACT */
void Input_DispatchKeyNative(int pressed, int scan_code) {
    InputKeyEventCallback callback;

    callback = g_inputKeyEventCallback;
    if (callback != 0) {
        callback(pressed, scan_code);
    }
}

/* @original Input_QueueCharacterNative (IGN_WIN.EXE @ 0x004560E0, inferred input.c)
 * @fidelity EXACT */
void Input_QueueCharacterNative(int unused, int scan_code) {
    unsigned char character;

    (void)unused;
    if (scan_code > 83) {
        return;
    }
    /* Poll supplies a masked scan code. Negative standalone indices are
     * outside the independently validated translation-table contract. */
    character = g_nativeCharacterMap[scan_code];
    *g_nativeCharacterQueueCursor = character;
    g_nativeCharacterQueueCursor[32] = character;
    ++g_nativeCharacterQueueCursor;
    if (g_nativeCharacterQueueCursor == g_nativeCharacterQueue.bytes + 32) {
        g_nativeCharacterQueueCursor = g_nativeCharacterQueue.bytes;
    }
}

/* @original Input_PollNative (IGN_WIN.EXE @ 0x00455EF0, inferred input.c)
 * @fidelity EXACT */
void Input_PollNative(void) {
    DIDEVICEOBJECTDATA records[32];
    volatile DWORD count;
    volatile DIDEVICEOBJECTDATA *record;
    LPDIRECTINPUTDEVICEA keyboard;
    HRESULT result;
    uint32_t counter, threshold;
    int index, scan_code;

    count = 32;
    for (index = 0; index < 256; ++index) {
        if (g_nativeKeyboardHeld.bytes[index] == 1) {
            counter = g_nativeRepeatCounters[index];
            threshold = (uint32_t)g_nativeRepeatDelay;
            counter -= g_nativeRepeatEpoch;
            threshold += (uint32_t)g_nativeRepeatInterval;
            counter += g_nativeInputClock;
            g_nativeRepeatCounters[index] = counter;
            if ((int32_t)counter >= (int32_t)threshold) {
                counter -= (uint32_t)g_nativeRepeatInterval;
                g_nativeKeyboardPressEvents.bytes[index] = 1;
                g_nativeRepeatCounters[index] = counter;
            }
        }
    }
    keyboard = g_nativeKeyboardDevice;
    result = keyboard->lpVtbl->GetDeviceData(keyboard, 16, records, (LPDWORD)&count, 0);
    if ((uint32_t)result == 0x8007001eu) {
        g_nativeKeyboardAcquired = 0;
        keyboard = g_nativeKeyboardDevice;
        result = keyboard->lpVtbl->Acquire(keyboard);
        if (result >= 0) {
            g_nativeKeyboardAcquired = 1;
        }
        return;
    }
    if (result == 0 || result == 1) {
        record = records;
        for (index = 0; index < (int32_t)count; ++index, ++record) {
            scan_code = (int)(record->dwOfs & 255u);
            if ((record->dwData & 128u) != 0) {
                g_nativeKeyboardHeld.bytes[scan_code] = 1;
                if (g_nativeKeyboardPressLatch[scan_code] == 0) {
                    g_nativeKeyboardPressEdges[scan_code] = 1;
                    g_nativeKeyboardPressLatch[scan_code] = 1;
                }
                if (g_nativeRepeatCounters[scan_code] == 0) {
                    g_nativeKeyboardPressEvents.bytes[scan_code] = 1;
                }
                Input_DispatchKeyNative(1, scan_code);
                scan_code = (int)(record->dwOfs & 255u);
                Input_QueueCharacterNative(1, scan_code);
            } else {
                g_nativeKeyboardHeld.bytes[scan_code] = 0;
                g_nativeKeyboardPressEdges[scan_code] = 0;
                g_nativeKeyboardPressLatch[scan_code] = 0;
                g_nativeKeyboardPressEvents.bytes[scan_code] = 0;
                g_nativeRepeatCounters[scan_code] = 0;
                Input_DispatchKeyNative(0, scan_code);
            }
        }
    }
    g_nativeRepeatEpoch = g_nativeInputClock;
}

/* @original Input_TimerCallbackNative (IGN_WIN.EXE @ 0x00456160, inferred input.c)
 * @fidelity EXACT */
void __stdcall Input_TimerCallbackNative(unsigned int timer_id, unsigned int message,
        unsigned long user, unsigned long value1, unsigned long value2) {
    InputPollCallback callback;

    (void)timer_id;
    (void)message;
    (void)user;
    (void)value1;
    (void)value2;
    Input_PollNative();
    callback = g_inputPollCallback;
    if (callback != 0) {
        callback();
    }
}

volatile uint32_t g_nativeInputPeriod; /* VA0x0050E67C, loader-zero .data tail */

/* @original Input_InitTimedNative (IGN_WIN.EXE @ 0x00455C60, inferred input.c)
 * @fidelity EXACT */
int Input_InitTimedNative(uint32_t period, int32_t repeat_delay, int32_t repeat_interval) {
    GUID keyboard_guid;
    DIPROPDWORD buffer_property;
    LPDIRECTINPUTA input;
    LPDIRECTINPUTDEVICEA keyboard;
    HRESULT result;
    HWND window;
    uint32_t now;
    UINT timer_id;
    int index;

    keyboard_guid = g_nativeKeyboardDeviceGuid;
    buffer_property.diph.dwSize = 20;
    buffer_property.diph.dwHeaderSize = 16;
    buffer_property.diph.dwObj = 0;
    buffer_property.diph.dwHow = 0;
    buffer_property.dwData = 32;
    if (DirectInputCreateA(g_nativeWindowInstance, 0x300,
            (LPDIRECTINPUTA *)&g_nativeDirectInput, 0) != 0) {
        return 0;
    }
    input = g_nativeDirectInput;
    if (input->lpVtbl->CreateDevice(input, &keyboard_guid,
            (LPDIRECTINPUTDEVICEA *)&g_nativeKeyboardDevice, 0) != 0) {
        return 0;
    }
    keyboard = g_nativeKeyboardDevice;
    if (keyboard->lpVtbl->SetDataFormat(keyboard, &g_nativeKeyboardFormat) != 0) {
        return 0;
    }
    window = g_nativeWindow;
    keyboard = g_nativeKeyboardDevice;
    if (keyboard->lpVtbl->SetCooperativeLevel(keyboard, window, 6) != 0) {
        return 0;
    }
    keyboard = g_nativeKeyboardDevice;
    if (keyboard->lpVtbl->SetProperty(keyboard, DIPROP_BUFFERSIZE,
            &buffer_property.diph) != 0) {
        return 0;
    }
    keyboard = g_nativeKeyboardDevice;
    result = keyboard->lpVtbl->Acquire(keyboard);
    g_nativeKeyboardAcquired = 1;
    if (result < 0) {
        g_nativeKeyboardAcquired = 0;
    }
    g_nativeCharacterQueueCursor = g_nativeCharacterQueue.bytes;
    for (index = 0; index < 16; index++) {
        g_nativeCharacterQueue.words[index] = 0xffffffffu;
    }
    g_nativeCharacterQueueEnd = 0;
    for (index = 0; index < 64; index++) {
        g_nativeKeyboardPressEvents.words[index] = 0;
    }
    for (index = 0; index < 64; index++) {
        g_nativeKeyboardHeld.words[index] = 0;
    }
    for (index = 0; index < 256; index++) {
        g_nativeKeyboardPressEdges[index] = 0;
        g_nativeRepeatCounters[index] = 0;
        g_nativeKeyboardPressLatch[index] = 0;
    }
    now = g_nativeInputClock;
    g_nativeRepeatDelay = repeat_delay;
    g_nativeRepeatInterval = repeat_interval;
    g_nativeInputPeriod = period;
    g_nativeRepeatEpoch = now;
    g_nativeInputTimerActive = 1;
    timer_id = timeSetEvent(period, period, Input_TimerCallbackNative, 0, TIME_PERIODIC);
    g_nativeInputTimerId = timer_id;
    return timer_id != 0;
}

/* @original Input_ShutdownNativeEntry (IGN_WIN.EXE @ 0x00455EA0, inferred input.c)
 * @fidelity EXACT */
int Input_ShutdownNativeEntry(void) {
    Input_ShutdownNative();
    return 1;
}

/* @original Input_ReinitializeNative (IGN_WIN.EXE @ 0x00455EB0, inferred input.c)
 * @fidelity EXACT */
int Input_ReinitializeNative(void) {
    int32_t interval, delay;
    uint32_t period;

    interval = g_nativeRepeatInterval;
    delay = g_nativeRepeatDelay;
    if (g_nativeInputTimerActive != 0) {
        period = g_nativeInputPeriod;
        Input_InitTimedNative(period, delay, interval);
    } else {
        Input_InitNative(delay, interval);
    }
    return 1;
}

/* @original Input_ReadHeldNative (IGN_WIN.EXE @ 0x00456070, inferred input.c)
 * @fidelity EXACT */
int Input_ReadHeldNative(int scan_code) {
    return g_nativeKeyboardHeld.bytes[(unsigned char)scan_code];
}

/* @original Input_ConsumePressNative (IGN_WIN.EXE @ 0x00456080, inferred input.c)
 * @fidelity EXACT */
int Input_ConsumePressNative(int scan_code) {
    unsigned char index;

    index = (unsigned char)scan_code;
    if (g_nativeKeyboardPressEdges[index] != 0) {
        g_nativeKeyboardPressEdges[index] = 0;
        return 1;
    }
    return 0;
}

/* @original Input_ConsumeRepeatNative (IGN_WIN.EXE @ 0x004560A0, inferred input.c)
 * @fidelity EXACT */
int Input_ConsumeRepeatNative(int scan_code) {
    unsigned char index;

    index = (unsigned char)scan_code;
    if (g_nativeKeyboardPressEvents.bytes[index] != 0) {
        g_nativeKeyboardPressEvents.bytes[index] = 0;
        return 1;
    }
    return 0;
}

/* End native DirectInput lifecycle implementation. */
