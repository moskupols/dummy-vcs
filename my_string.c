#include "my_string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

bool string_is_null(const struct string* s)
{
    return s->len == FICTIVE_LEN;
}

return_t string_alloc(struct string* out, size_t size)
{
    out->data = calloc(size + 1, sizeof(char));
    if (out->data == NULL)
        return ERR_BAD_ALLOC;

    out->len = size;
    return SUCCESS;
}

return_t string_copy_alloc(struct string* out, const struct string* source)
{
    return string_copy_cstr_n_alloc(out, source->data, source->len);
}

return_t string_copy_cstr_alloc(struct string* out, const char* cstr)
{
    return string_copy_cstr_n_alloc(out, cstr, strlen(cstr));
}

return_t string_copy_cstr_n_alloc(struct string* out, const char* cstr, size_t n)
{
    return_t ret = string_alloc(out, n);
    if (ret != SUCCESS)
        return ret;

    strncpy(out->data, cstr, n);
    return SUCCESS;
}

void string_assign_cstr(struct string* out, char* cstr)
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

static inline void check_substr(
        size_t mylen, size_t pos, size_t* len)
{
    assert(pos <= mylen);

    if (*len == FICTIVE_LEN)
        *len = mylen - pos;
    assert(pos + *len <= mylen);
}

struct substr string_substr(const struct string* string, size_t pos, size_t len)
{
    check_substr(string->len, pos, &len);
    return (struct substr){string, pos, len};
}

return_t string_insert(struct string* into, size_t pos, const struct string* what)
{
    assert(pos <= into->len);

    return_t ret = my_realloc((void**)&into->data, into->len + what->len + 1);
    if (ret != SUCCESS)
        return ret;

    size_t suffix_len = into->len - pos;
    char* suffix = into->data + pos;

    memmove(suffix + what->len, suffix, suffix_len);
    memcpy(suffix, what->data, what->len);
    into->len += what->len;
    into->data[into->len] = '\0';

    return SUCCESS;
}

return_t string_erase(struct string* from, size_t pos, size_t len)
{
    check_substr(from->len, pos, &len);

    char* erased = from->data + pos;
    size_t suffix_len = from->len - pos - len;

    memmove(erased, erased + len, suffix_len);
    from->len -= len;
    from->data[from->len] = '\0';

    return SUCCESS;
}

return_t string_shrink(struct string* s)
{
    assert(s != NULL);
    assert(s->data != NULL);

    s->len = strlen(s->data);
    return my_realloc(&s->data, s->len + 1);
}



const char* substr_begin(const struct substr* substr)
{
    return substr->str->data + substr->pos;
}

struct substr substr_substr(const struct substr* substr, size_t pos, size_t len)
{
    check_substr(substr->len, pos, &len);
    return (struct substr){substr->str, substr->pos + pos, len};
}

return_t substr_to_string_alloc(struct string* out, const struct substr* sub)
{
    return string_copy_cstr_n_alloc(out, substr_begin(sub), sub->len);
}

