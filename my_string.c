#include "my_string.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void string_reserve(char** s, size_t new_len)
{
    if (s == NULL)
    {
        *s = checked_calloc(new_len + 1, sizeof(char));
        return;
    }

    size_t old_len = strlen(*s);
    if (old_len >= new_len)
        return;

    checked_realloc((void**)s, new_len + 1);
    memset(*s + old_len, 0, new_len - old_len + 1);
}

void string_shrink(char** s)
{
    assert(s != NULL);

    checked_realloc((void**)s, strlen(*s) + 1);
}

char* string_copy_alloc(const char* source)
{
    return string_copy_n_alloc(source, strlen(source));
}

char* string_copy_n_alloc(const char* source, size_t n)
{
    assert(source != NULL);

    char* ret = checked_malloc(n + 1);
    strncpy(ret, source, n);
    ret[n] = '\0';
    return ret;
}

void string_assign_copy(char** dest, const char* source)
{
    string_reserve(dest, strlen(source));
    strcpy(*dest, source);
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

struct substr string_substr(const char* string, size_t pos, size_t len)
{
    bool valid_substr = check_substr(strlen(string), pos, len, &len);
    assert(valid_substr);
    return (struct substr){string, pos, len};
}

return_t string_insert(char** into, size_t pos, const char* what)
{
    size_t old_len = strlen(*into);
    size_t add_len = strlen(what);

    if (pos > old_len)
        return ERR_INVALID_RANGE;

    string_reserve(into, old_len + add_len);

    size_t suffix_len = old_len - pos;
    char* suffix = *into + pos;

    memmove(suffix + add_len, suffix, suffix_len);
    memcpy(suffix, what, add_len);
    (*into)[old_len + add_len] = '\0';

    return SUCCESS;
}

return_t string_erase(char** from, size_t pos, size_t len)
{
    size_t old_len = strlen(*from);
    if (!check_substr(old_len, pos, len, &len))
        return ERR_INVALID_RANGE;

    char* erased = *from + pos;
    size_t suffix_len = old_len - pos - len;

    memmove(erased, erased + len, suffix_len);
    (*from)[old_len - len] = '\0';

    return SUCCESS;
}




const char* substr_begin(const struct substr* substr)
{
    return substr->str + substr->pos;
}

struct substr substr_substr(const struct substr* substr, size_t pos, size_t len)
{
    bool valid_substr = check_substr(substr->len, pos, len, &len);
    assert(valid_substr);
    return (struct substr){substr->str, substr->pos + pos, len};
}

char* substr_to_string_alloc(const struct substr* sub)
{
    return string_copy_n_alloc(substr_begin(sub), sub->len);
}

