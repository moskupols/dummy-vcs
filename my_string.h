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

return_t string_alloc(struct string* out, size_t size);
return_t string_copy_alloc(struct string* out, const struct string* source);
return_t string_copy_cstr_n_alloc(struct string* out, const char* cstr, size_t n);
return_t string_copy_cstr_alloc(struct string* out, const char* cstr);

void string_assign_cstr(struct string* inout, char* cstr);
void string_assign_cstr_n(struct string* inout, char* cstr, size_t n);

void string_free(struct string* string);

struct substr string_substr(const struct string* string, size_t pos, size_t len);

return_t string_insert(struct string* into, size_t pos, const struct string* what);
return_t string_erase(struct string* from, size_t pos, size_t len);

int string_cmp(const struct string* a, const struct string* b);

return_t string_shrink(struct string* s);



const char* substr_begin(const struct substr* substr);
return_t substr_to_string_alloc(struct string* out, const struct substr* sub);
struct substr substr_substr(const struct substr* string, size_t pos, size_t len);

