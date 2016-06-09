#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "error.h"

#define FICTIVE_LEN ((size_t)-1)

struct substr
{
    const char* str;
    size_t pos;
    size_t len;
};

void string_reserve(char** s, size_t size);
void string_shrink(char** s);

char* string_copy_alloc(const char* source);
char* string_copy_n_alloc(const char* source, size_t n);

bool check_substr(size_t old_len, size_t pos, size_t new_len, size_t* new_len_p);

struct substr string_substr(const char* string, size_t pos, size_t len);

return_t string_insert(char** into, size_t pos, const char* what);
return_t string_erase(char** from, size_t pos, size_t len);


const char* substr_begin(const struct substr* substr);
char* substr_to_string_alloc(const struct substr* sub);
struct substr substr_substr(const struct substr* string, size_t pos, size_t len);

