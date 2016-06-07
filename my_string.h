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
#define STRING_NULL ((struct string){FICTIVE_LEN, NULL})

typedef const struct string substring;

bool string_is_null(const struct string* s);

return_t string_alloc(struct string* out, size_t size);
return_t string_copy_alloc(struct string* out, const struct string* source);
return_t string_copy_cstr_alloc(struct string* out, const char* cstr);

void string_assign_cstr(struct string* inout, char* cstr);

void string_free(struct string* string);

struct string string_substr(const struct string* string, size_t pos, size_t len);

return_t string_insert(struct string* into, size_t pos, const struct string* what);
return_t string_erase(struct string* from, size_t pos, size_t len);

return_t string_shrink(struct string* s);

