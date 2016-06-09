#include "parse.h"

#include <stdlib.h>
#include <assert.h>

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

