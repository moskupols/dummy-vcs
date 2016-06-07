#pragma once

#include <stdio.h>

#include "my_string.h"
#include "error.h"

typedef enum
{
    DELTA_ADD = '+',
    DELTA_ERASE = '-'
} delta_line_type;

struct delta_line
{
    struct delta_line* tail;

    size_t pos;
    union
    {
        struct string text;
        size_t erase_len;
    };

    delta_line_type type;
};

struct delta
{
    int parent; // TODO revision_t
    struct delta_line* lines;
};
#define DELTA_NULL {-1, NULL}

return_t delta_calc(struct delta* out, const struct string* a, const struct string* b);
return_t delta_load(struct delta* out, FILE* stream);

void delta_free(struct delta* delta);

return_t delta_apply(struct string* text, const struct delta *delta);
return_t delta_apply_alloc(
        struct string* out, const struct delta* delta, const struct string* source);

return_t delta_save(const struct delta* delta, FILE* stream);


