#include "my_string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

bool string_is_null(const struct string* s)
{
    return s->len == FICTIVE_LEN;
}

return_t string_alloc(struct string* out, size_t size)
{
    assert(string_is_null(out));

    out->data = calloc(size + 1, sizeof(char));
    if (out->data == NULL)
        return ERR_BAD_ALLOC;

    out->len = size;
    return SUCCESS;
}

return_t string_copy_alloc(struct string* out, const struct string* source)
{
    return_t ret = string_alloc(out, source->len);
    if (ret != SUCCESS)
        return ret;

    strncpy(out->data, source->data, out->len);
    return SUCCESS;
}

return_t string_copy_cstr_alloc(struct string* out, const char* cstr)
{
    size_t len = strlen(cstr);

    return_t ret = string_alloc(out, len);
    if (ret != SUCCESS)
        return ret;

    strncpy(out->data, cstr, len);
    return SUCCESS;
}

void string_assign_cstr(struct string* out, const char* cstr)
{
    assert(out != NULL);
    assert(cstr != NULL);

    out->len = strlen(cstr);
    out->data = cstr;
}

void string_free(struct string* string)
{
    assert(!string_is_null(string));
    free(string->data);
    *string = STRING_NULL;
}

inline void check_substr(
        const struct string* string, size_t pos, size_t* len)
{
    assert(pos <= string->len);

    if (*len == FICTIVE_LEN)
        *len = string->len - pos;
    assert(pos + *len <= string->len);
}

substring string_substr(const struct string* string, size_t pos, size_t len)
{
    check_substr(string, pos, &len);
    return (substring){len, string->data + pos};
}

void string_insert(struct string* into, size_t pos, const struct string* what)
{
    assert(pos <= into->len);
    size_t suffix_len = into->len - pos;
    char* suffix = into->data + pos;

    memmove(suffix + what->len, suffix, suffix_len);
    memcpy(suffix, what->data, what->len);
    into->len += what->len;
}

void string_erase(struct string* from, size_t pos, size_t len)
{
    check_substr(from, pos, &len);

    char* erased = from->data + pos;
    size_t suffix_len = from->len - pos - len;

    memmove(erased, erased + len, suffix_len);
    from->len -= len;
    from->data[from->len] = '\0';
}

