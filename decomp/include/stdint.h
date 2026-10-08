#ifndef _STDINT_H_
#define _STDINT_H_

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed int int32_t;
typedef unsigned int uint32_t;
#ifndef _UINTPTR_T_DEFINED
#define _UINTPTR_T_DEFINED
typedef unsigned long uintptr_t;
typedef uintptr_t __w_uintptr_t;
#endif
#ifndef _INTPTR_T_DEFINED
#define _INTPTR_T_DEFINED
typedef signed long intptr_t;
typedef intptr_t __w_intptr_t;
#endif

#endif
