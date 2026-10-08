#ifndef IGNITION_FILE_H
#define IGNITION_FILE_H

/* Opaque native CRT stream. These wrappers only forward its pointer;
 * the CRT FILE layout and filesystem implementation remain unreconstructed.
 */
typedef struct FileStream FileStream;

/* One cdecl stack dword each; signed 32-bit long size results, no error guard. */
long File_GetStreamSize(FileStream *stream);
long File_GetSize(const char *filename);
/* Returns 2031 on null fopen, otherwise closes and returns 1. Text mode "r". */
int File_CheckReadable(const char *filename);
void *File_LoadToMemory(const char *filename);

#endif
