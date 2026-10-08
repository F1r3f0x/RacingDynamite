/* Native file wrappers; semantic module placement follows the adjacent
 * Windows loader/size/readability call graph. Original filename is unknown.
 */
#include "file.h"
#include "mem.h"
#include "geputget.h"
#include <stddef.h>

extern FileStream *fopen(const char *filename, const char *mode);
extern int fclose(FileStream *stream);
extern long ftell(FileStream *stream);
extern int fseek(FileStream *stream, long offset, int origin);
extern int fsetpos(FileStream *stream, const void *pos);
extern unsigned int fread(void *ptr, unsigned int size, unsigned int nmemb, FileStream *stream);

typedef char File_LongMustBe32Bits[(sizeof(long) == 4) ? 1 : -1];
typedef char File_PointerMustBe32Bits[(sizeof(void *) == 4) ? 1 : -1];

/* @original File_GetStreamSize (IGN_WIN.EXE @ 0x004575F0, inferred file.c)
 * @fidelity EXACT
 * Both seek results are ignored, including restoration after a failed tell.
 */
long File_GetStreamSize(FileStream *stream)
{
    long original_position;
    long size;

    original_position = ftell(stream);
    fseek(stream, 0L, 2);
    size = ftell(stream);
    fseek(stream, original_position, 0);
    return size;
}

/* @original File_GetSize (IGN_WIN.EXE @ 0x00457630, inferred file.c)
 * @fidelity EXACT
 * Null fopen is still passed to the size helper and fclose. No global error write.
 */
long File_GetSize(const char *filename)
{
    FileStream *stream;
    long size;

    stream = fopen(filename, "r");
    size = File_GetStreamSize(stream);
    fclose(stream);
    return size;
}

/* @original File_CheckReadable (IGN_WIN.EXE @ 0x004576B0, inferred file.c)
 * @fidelity EXACT
 * Uses text mode; close failure does not change the successful return.
 */
int File_CheckReadable(const char *filename)
{
    FileStream *stream;

    stream = fopen(filename, "r");
    if (stream == NULL) {
        return 2031;
    }
    fclose(stream);
    return 1;
}

/* @original File_LoadToMemory (IGN_WIN.EXE @ 0x004574A0, inferred file.c)
 * @fidelity EXACT
 * Uses text mode checks; allocates memory, uses fsetpos/fread, handles errors.
 */
void *File_LoadToMemory(const char *filename)
{
    long size;
    void *buffer;
    FileStream *stream;
    long position[2];
    unsigned int read_result;

    if (File_CheckReadable(filename) != 1) {
        g_fileErrorLine = 2030;
        return NULL;
    }

    size = File_GetSize(filename);
    if (size == 0) {
        g_fileErrorLine = 2040;
        return NULL;
    }

    buffer = Mem_Alloc(0, (unsigned int)size);
    if (buffer == NULL) {
        g_fileErrorLine = 2050;
        return NULL;
    }

    stream = fopen(filename, "rb");
    if (stream == NULL) {
        g_fileErrorLine = 2000;
        return NULL;
    }

    position[0] = 0;
    position[1] = 0;
    fsetpos(stream, position);

    read_result = fread(buffer, 1, (unsigned int)size, stream);
    if (read_result != (unsigned int)size) {
        g_fileErrorLine = 2010;
        return NULL;
    }

    fclose(stream);
    return buffer;
}
