#include "parse.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#include "utils.h"

return_t read_until(char** out, FILE* stream, int stop_char)
{
    assert(out != NULL);

    size_t used = 0;
    size_t capacity = 8;

    char* buf = checked_malloc(capacity);

    char c;
    while ((c = fgetc(stream)) != stop_char && !feof(stream) && !ferror(stream))
    {
        buf[used++] = c;
        if (used >= capacity)
        {
            capacity *= 2;
            checked_realloc((void**)&buf, capacity);
        }
    }

    buf[used] = '\0';
    *out = buf;
    string_shrink(out);
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

