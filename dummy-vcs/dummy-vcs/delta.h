#pragma once

#include <stdio.h>

#include "my_string.h"
#include "error.h"

typedef char delta_line_type_t;
#define DELTA_ADD '+'
#define DELTA_ERASE '-'

struct delta_line
{
    size_t pos;
    char* text;

    delta_line_type_t type;
};
#define DELTA_LINE_INIT { 0, NULL, '?' }
static const struct delta_line delta_line_init = DELTA_LINE_INIT;

struct delta_line delta_line_new( size_t pos, char* text, delta_line_type_t type );

struct delta
{
    struct delta_line* lines;
    size_t len;
    size_t capacity;
};
#define DELTA_INIT { NULL, 0, 0 }
static const struct delta delta_init = DELTA_INIT;

void delta_append(struct delta* delta, struct delta_line line);

return_t delta_load(struct delta* out, FILE* stream);
void delta_line_free(struct delta_line* line);
void delta_free(struct delta* delta);

return_t delta_line_print(const struct delta_line* line, FILE* stream);
return_t delta_print(const struct delta* delta, FILE* stream);

return_t delta_apply(char** text, const struct delta* delta);
return_t delta_apply_alloc(
        char** out, const struct delta* delta, const char* source);

