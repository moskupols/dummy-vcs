#include "parse.h"

#include <stdlib.h>
#include <assert.h>

return_t read_line(struct string* out, FILE* stream)
{
    assert(out != NULL);

    size_t used = 0;
    size_t capacity = 8;

    char* buf = malloc(capacity, sizeof(char));
    if (buf == NULL)
        return ERR_BAD_ALLOC;

    char c = fgetc(stream);
    while (!feof(stream) && !ferror(stream) && c != '\n')
    {
        buf[used++] = c;
        if (used >= capacity)
        {
            capacity *= 2;
            char* new_buf = realloc(buf, capacity);
            if (new_buf == NULL)
            {
                free(buf);
                return ERR_BAD_ALLOC;
            }
            buf = new_buf;
        }
    }

    buf[used] = '\0';
    string_assign_cstr(out, buf);
    return SUCCESS;
}

