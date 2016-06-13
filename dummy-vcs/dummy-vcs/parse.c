#include "parse.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "utils.h"
#include "my_string.h"

return_t read_until(char** out, FILE* stream, int stop_char)
{
    assert(out != NULL);

    size_t used = 0; // Сколько символов прочитано и записано
    size_t capacity = 8; // Сколько выделено памяти

    char* buf = checked_malloc(capacity);

    char c;
    while ((c = fgetc(stream)) != stop_char && !feof(stream) && !ferror(stream))
    {
        buf[used++] = c;
        if (used >= capacity) // Если записано столько символов, сколько выделено памяти, то уже надо
                              // выделить ещё, потому что мы собираемся ещё дописать '\0'
        {
            capacity *= 2;
            checked_realloc((void**)&buf, capacity);
        }
    }

    buf[used] = '\0';
    *out = buf;
    return SUCCESS;
}

return_t read_file(char** out, const char* path)
{
    FILE* f = fopen(path, "r");
    if (f == NULL)
        return ERR_NO_SUCH_FILE;
    return_t ret = read_all(out, f);
    fclose(f);
    return ret;
}

