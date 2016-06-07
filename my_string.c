#include "my_string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

bool string_is_null(const struct string* s)
{
    return s->len == FICTIVE_LEN || s->data == NULL;
}

return_t string_alloc(struct string* out, size_t size)
{
    out->data = calloc(size + 1, sizeof(char));
    if (out->data == NULL)
        return ERR_NO_MEMORY;

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
    assert(out != NULL);
    assert(cstr != NULL);

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
    free(string->data);
    *string = STRING_NULL;
}

bool check_substr(
        size_t mylen, size_t pos, size_t new_len, size_t* new_len_p)
{
    if (pos > mylen)
        return false;

    if (new_len == FICTIVE_LEN)
        new_len = mylen - pos;

    if (pos + new_len > mylen)
        return false;

    if (new_len_p != NULL)
        *new_len_p = new_len;

    return true;
}

struct substr string_substr(const struct string* string, size_t pos, size_t len)
{
    bool valid_substr = check_substr(string->len, pos, len, &len);
    assert(valid_substr);
    return (struct substr){string, pos, len};
}

return_t string_insert(struct string* into, size_t pos, const struct string* what)
{
    if (pos > into->len)
        return ERR_INVALID_RANGE;

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
    if (!check_substr(from->len, pos, len, &len))
        return ERR_INVALID_RANGE;

    char* erased = from->data + pos;
    size_t suffix_len = from->len - pos - len;

    memmove(erased, erased + len, suffix_len);
    from->len -= len;
    from->data[from->len] = '\0';

    return SUCCESS;
}

int string_cmp(const struct string* a, const struct string* b)
{
    size_t s = min(a->len, b->len);
    int ret = strncmp(a->data, b->data, s);
    if (!ret)
        ret = a->data[s] - b->data[s];
    return ret;
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
    bool valid_substr = check_substr(substr->len, pos, len, &len);
    assert(valid_substr);
    return (struct substr){substr->str, substr->pos + pos, len};
}

return_t substr_to_string_alloc(struct string* out, const struct substr* sub)
{
    return string_copy_cstr_n_alloc(out, substr_begin(sub), sub->len);
}

