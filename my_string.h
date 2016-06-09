#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "error.h"

struct string
{
    size_t len;
    char* data;
};
#define FICTIVE_LEN ((size_t)-1)
#define STRING_NULL ((struct string){0, NULL})

struct substr
{
    const struct string* str;
    size_t pos;
    size_t len;
};

bool string_is_null(const struct string* s);

void string_reserve(struct string* s, size_t size);
void string_shrink(struct string* s);

struct string string_alloc(size_t size);
struct string string_copy_alloc(const struct string* source);
struct string string_copy_cstr_n_alloc(const char* cstr, size_t n);
struct string string_copy_cstr_alloc(const char* cstr);

struct string string_from_cstr(char* cstr);

void string_free(struct string* string);

bool check_substr(size_t old_len, size_t pos, size_t new_len, size_t* new_len_p);

struct substr string_substr(const struct string* string, size_t pos, size_t len);

return_t string_insert(struct string* into, size_t pos, const struct string* what);
return_t string_erase(struct string* from, size_t pos, size_t len);

int string_cmp(const struct string* a, const struct string* b);


const char* substr_begin(const struct substr* substr);
struct string substr_to_string_alloc(const struct substr* sub);
struct substr substr_substr(const struct substr* string, size_t pos, size_t len);

