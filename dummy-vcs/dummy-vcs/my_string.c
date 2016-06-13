#include "my_string.h"

#include <assert.h>
#include <string.h>

#include "utils.h"

void string_reserve(char** s, size_t new_len)
{
    assert(*s != NULL);

    size_t old_len = strlen(*s);
    if (old_len >= new_len) // строка и так не короче
        return;

    checked_realloc((void**)s, new_len + 1);
    memset(*s + old_len, 0, new_len - old_len + 1); // установим нули везде в новой памяти
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

char* string_substr_alloc(const char* str, size_t pos, size_t len)
{
    assert(check_substr(strlen(str), pos, len, &len));
    return string_copy_n_alloc(str + pos, len);
}

return_t string_insert(char** into, size_t pos, const char* what)
{
    size_t old_len = strlen(*into);
    size_t add_len = strlen(what);

    if (pos > old_len)
        return ERR_INVALID_RANGE;

    string_reserve(into, old_len + add_len); // убедимся, что памяти достаточно

    size_t suffix_len = old_len - pos;
    char* suffix = *into + pos;

    memmove(suffix + add_len, suffix, suffix_len); // сдвинем суффикс вправо
    memcpy(suffix, what, add_len); // скопируем what туда, где раньше начинался суффикс
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

    memmove(erased, erased + len, suffix_len); // Сдвинем суффикс туда, где раньше была удалённая подстрока
    (*from)[old_len - len] = '\0';

    return SUCCESS;
}
