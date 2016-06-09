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
        char* text;
        size_t erase_len;
    };

    delta_line_type type;
};

struct delta
{
    int parent;
    struct delta_line* lines;
};
#define DELTA_NULL {-1, NULL}

struct delta delta_calc(const char* a, const char* b);
return_t delta_load(struct delta* out, FILE* stream);

void delta_free(struct delta* delta);

return_t delta_apply(char** text, const struct delta *delta);
return_t delta_apply_alloc(
        char** out, const struct delta* delta, const char* source);

return_t delta_save(const struct delta* delta, FILE* stream);


