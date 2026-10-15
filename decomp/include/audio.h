#ifndef IGNITION_AUDIO_H
#define IGNITION_AUDIO_H

#include <windows.h>
#include <mmsystem.h>
#include <stdint.h>

extern char g_cdAudioReturnString[256]; /* VA 0x0050EF70 */

int32_t Audio_StopCD(void);
int32_t Audio_OpenCD(void);
int32_t Audio_CloseCD(void);
int32_t Audio_EjectCD(void);
int32_t Audio_ShutdownSound(void);

#endif /* IGNITION_AUDIO_H */
