#ifndef MEM_H
#define MEM_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/* Global file system error tracker matching MAINDOS @ 0x000c5cf4 */
extern int g_fileErrorLine;

/* Function prototypes */
void *Mem_Alloc(int pool_id, int size);
int File_Exists(const char *filename);
int File_GetSize(const char *filename);
int File_ReadToBuffer(const char *filename, void *buffer, int size, int offset);
void *File_LoadToMemory(const char *filename);

#endif /* MEM_H */
