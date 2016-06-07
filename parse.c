#include "parse.h"

#include <stdlib.h>
#include <assert.h>

#include "utils.h"

return_t read_line(struct string* out, FILE* stream)
{
    assert(out != NULL);

    size_t used = 0;
    size_t capacity = 8;

    char* buf = malloc(capacity);
    if (buf == NULL)
        return ERR_NO_MEMORY;

    char c;
    while ((c = fgetc(stream)) != '\n' && !feof(stream) && !ferror(stream))
    {
        buf[used++] = c;
        if (used >= capacity)
        {
            capacity *= 2;

            return_t ret = my_realloc(&buf, capacity);
            if (ret != SUCCESS)
            {
                free(buf);
                return ret;
            }
        }
    }

    buf[used] = '\0';
    string_assign_cstr(out, buf);
    return SUCCESS;
}

