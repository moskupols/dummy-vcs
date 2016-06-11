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
    char* text;

    delta_line_type type;
};

struct delta
{
    int parent;
    struct delta_line* lines;
};
#define DELTA_NULL {-1, NULL}

struct delta_line* delta_calc_lines(const char* a, const char* b);
struct delta delta_calc(const char* a, const char* b, int parent);

return_t delta_load_parent(int* out, FILE* stream);
return_t delta_load_lines(struct delta_line** out, FILE* stream);
return_t delta_load(struct delta* out, FILE* stream);

void delta_lines_free(struct delta_line* lines);
void delta_free(struct delta* delta);

return_t delta_lines_apply(char** text, const struct delta_line *lines);
return_t delta_lines_apply_alloc(
        char** out, const struct delta_line* lines, const char* source);

return_t delta_save(const struct delta* delta, FILE* stream);

