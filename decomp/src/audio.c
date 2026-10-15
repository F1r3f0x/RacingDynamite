#include "audio.h"

char g_cdAudioReturnString[256] = {0}; /* VA 0x0050EF70 */

/*
 * @original Audio_StopCD (IGN_WIN.EXE @ 0x00457FC0, audio.c)
 * @fidelity EXACT
 */
int32_t Audio_StopCD(void) {
    return mciSendStringA("stop cdaudio", (LPSTR)g_cdAudioReturnString, 256, NULL) == 0;
}

/*
 * @original Audio_OpenCD (IGN_WIN.EXE @ 0x00458040, audio.c)
 * @fidelity EXACT
 */
int32_t Audio_OpenCD(void) {
    if (mciSendStringA("open cdaudio", (LPSTR)g_cdAudioReturnString, 256, NULL) != 0) {
        return 0;
    }
    if (mciSendStringA("set cdaudio audio all on", (LPSTR)g_cdAudioReturnString, 256, NULL) != 0) {
        return 0;
    }
    return mciSendStringA("set cdaudio time format msf", (LPSTR)g_cdAudioReturnString, 256, NULL) == 0;
}

/*
 * @original Audio_CloseCD (IGN_WIN.EXE @ 0x004580A0, audio.c)
 * @fidelity EXACT
 */
int32_t Audio_CloseCD(void) {
    return mciSendStringA("close cdaudio", (LPSTR)g_cdAudioReturnString, 256, NULL) == 0;
}

/*
 * @original Audio_EjectCD (IGN_WIN.EXE @ 0x004580C0, audio.c)
 * @fidelity EXACT
 */
int32_t Audio_EjectCD(void) {
    return mciSendStringA("set cdaudio door open", (LPSTR)g_cdAudioReturnString, 256, NULL) == 0;
}

/*
 * @original Audio_ShutdownSound (IGN_WIN.EXE @ 0x00459D30, audio.c)
 * @fidelity EXACT
 */
int32_t Audio_ShutdownSound(void) {
    return 0;
}
