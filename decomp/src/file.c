/* Native file wrappers; semantic module placement follows the adjacent
 * Windows loader/size/readability call graph. Original filename is unknown.
 */
#include "file.h"
#include <stddef.h>

extern FileStream *fopen(const char *filename, const char *mode);
extern int fclose(FileStream *stream);
extern long ftell(FileStream *stream);
extern int fseek(FileStream *stream, long offset, int origin);

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
