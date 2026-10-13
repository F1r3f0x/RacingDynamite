#ifndef IGNITION_INPUT_H
#define IGNITION_INPUT_H

#include <stdint.h>

/* Windows input callback ABI; semantic names, original symbols unknown. */
typedef void (*InputKeyEventCallback)(int pressed, int scan_code);
typedef void (*InputPollCallback)(void);
extern InputKeyEventCallback volatile g_inputKeyEventCallback;
extern InputPollCallback volatile g_inputPollCallback;
void Input_ResetCallbacks(void);

int Input_InitNative(int32_t repeat_delay, int32_t repeat_interval); /* VA0x00455AC0 */
int Input_ShutdownNative(void); /* VA0x00455E20 */
int Input_InitTimedNative(uint32_t period, int32_t repeat_delay, int32_t repeat_interval); /* VA0x00455C60 */
int Input_ShutdownNativeEntry(void); /* VA0x00455EA0 */
int Input_ReinitializeNative(void); /* VA0x00455EB0 */
int Input_ReadHeldNative(int scan_code); /* VA0x00456070, low byte index */
int Input_ConsumePressNative(int scan_code); /* VA0x00456080, low byte index */
int Input_ConsumeRepeatNative(int scan_code); /* VA0x004560A0, low byte index */
void Input_PollNative(void); /* VA0x00455EF0 */
void Input_DispatchKeyNative(int pressed, int scan_code); /* VA0x004560C0 */
void Input_QueueCharacterNative(int unused, int scan_code); /* VA0x004560E0 */
void __stdcall Input_TimerCallbackNative(unsigned int timer_id, unsigned int message,
    unsigned long user, unsigned long value1, unsigned long value2); /* VA0x00456160 */

#endif
