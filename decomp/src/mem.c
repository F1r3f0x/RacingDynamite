/*
 * mem.c - Memory management, file I/O and asset loading
 * Original file: d:\projects\ignition\mem\mem.c
 * Target: MAINDOS.EXE (Watcom C/C++ 10.6, 32-bit flat protected mode)
 */

#include "mem.h"

/* Global file error tracking variable (MAINDOS @ 0x000c5cf4) */
int g_fileErrorLine = 0;

/**
 * Mem_Alloc (MAINDOS @ 0x00060E90)
 * Allocates memory from the heap.
 */
void *Mem_Alloc(int pool_id, int size) {
    size_t s;
    if (size > 0) {
        s = size;
    } else if (pool_id > 0) {
        s = pool_id;
    } else {
        s = 1024;
    }
    return malloc(s);
}

/**
 * Mem_Free (MAINDOS @ 0x00060EC0)
 * Releases memory back to the heap.
 */
void Mem_Free(int pool_id, void *ptr) {
    (void)pool_id;
    if (ptr) {
        free(ptr);
    }
}

/**
 * File_Exists (MAINDOS @ 0x0006117C)
 * Checks if a file exists by attempting to open it in binary read mode.
 */
int File_Exists(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        return 0x7ef;
    }
    fclose(fp);
    return 1;
}

/**
 * File_GetSize (MAINDOS @ 0x00061100)
 * Queries the exact byte size of a file using fseek and ftell.
 */
int File_GetSize(const char *filename) {
    FILE *fp;
    long old_pos;
    long size;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        return 0;
    }
    old_pos = ftell(fp);
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, old_pos, SEEK_SET);
    fclose(fp);
    return size;
}

/**
 * File_ReadToBuffer (MAINDOS @ 0x00060F40)
 * Reads binary file data directly into a caller-provided preallocated buffer.
 */
int File_ReadToBuffer(const char *filename, void *buffer, int size, int offset) {
    FILE *fp;
    fpos_t pos;
    int res;

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        res = 2000;
        goto done;
    }
    pos = offset;
    fsetpos(fp, &pos);
    if ((int)fread(buffer, 1, size, fp) != size) {
        return 2010;
    }
    fclose(fp);
    res = 1;
done:
    return res;
}

/**
 * File_LoadToMemory (MAINDOS @ 0x00060F9C, IGN_WIN @ 0x004574A0)
 * Allocates memory buffer via malloc and loads entire file content from disk.
 */
void *File_LoadToMemory(const char *filename) {
    int file_size;
    void *buffer = NULL;
    FILE *fp;
    fpos_t pos;

    if (File_Exists(filename) != 1) {
        g_fileErrorLine = 0x7ee;
        return buffer;
    }

    file_size = File_GetSize(filename);
    if (file_size == 0) {
        g_fileErrorLine = 0x7f8;
        return NULL;
    }

    buffer = Mem_Alloc(0, file_size);
    if (buffer == NULL) {
        g_fileErrorLine = 0x802;
        return NULL;
    }

    fp = fopen(filename, "rb");
    if (fp == NULL) {
        g_fileErrorLine = 0x7d0;
        return NULL;
    }

    pos = 0;
    fsetpos(fp, &pos);
    if ((int)fread(buffer, 1, file_size, fp) != file_size) {
        g_fileErrorLine = 0x7da;
        return NULL;
    }

    fclose(fp);
    return buffer;
}
