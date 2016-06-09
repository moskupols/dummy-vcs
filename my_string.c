#include "my_string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

bool string_is_null(const struct string* s)
{
    return s->data == NULL;
}

void string_reserve(struct string* s, size_t size)
{
    if (string_is_null(s))
        *s = string_alloc(size);

    if (s->len >= size)
        return;

    checked_realloc((void**)&s->data, size + 1);
    memset(s->data + s->len, 0, size - s->len + 1);
    /* s->len = size; */
}

void string_shrink(struct string* s)
{
    assert(s != NULL);
    assert(s->data != NULL);

    s->len = strlen(s->data);
    checked_realloc((void**)&s->data, s->len + 1);
}

struct string string_alloc(size_t size)
{
    return (struct string) { 0, checked_calloc(size + 1, sizeof(char)) };
}

struct string string_copy_alloc(const struct string* source)
{
    return string_copy_cstr_n_alloc(source->data, source->len);
}

struct string string_copy_cstr_alloc(const char* cstr)
{
    return string_copy_cstr_n_alloc(cstr, strlen(cstr));
}

struct string string_copy_cstr_n_alloc(const char* cstr, size_t n)
{
    assert(cstr != NULL);

    struct string ret = string_alloc(n);
    strncpy(ret.data, cstr, n);
    return ret;
}

struct string string_from_cstr(char* cstr)
{
    assert(cstr != NULL);

    return (struct string){strlen(cstr), cstr};
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

    size_t old_len = into->len;

    string_reserve(into, into->len + what->len);

    size_t suffix_len = old_len - pos;
    char* suffix = into->data + pos;

    memmove(suffix + what->len, suffix, suffix_len);
    memcpy(suffix, what->data, what->len);

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

struct string substr_to_string_alloc(const struct substr* sub)
{
    return string_copy_cstr_n_alloc(substr_begin(sub), sub->len);
}

