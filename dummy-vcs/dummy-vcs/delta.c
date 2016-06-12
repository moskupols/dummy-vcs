#include "delta.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "my_string.h"
#include "parse.h"
#include "utils.h"

struct delta_line delta_line_new(size_t pos, char* text, delta_line_type_t type)
{
    struct delta_line ret;
    ret.pos = pos;
    ret.text = text;
    ret.type = type;
    return ret;
}

void delta_append(struct delta* delta, struct delta_line line)
{
    if (delta->len == delta->capacity)
    {
        delta->capacity = max(3, delta->capacity * 2);
        checked_realloc(
                (void**)&delta->lines,
                delta->capacity * sizeof(struct delta_line));
    }
    delta->lines[delta->len++] = line;
}

return_t delta_line_load(struct delta_line* out, FILE* stream)
{
    char type;
    if (fscanf(stream, "%c ", &type) <= 0)
        return ERR_READ;

    if (type != DELTA_ADD && type != DELTA_ERASE)
        return ERR_INVALID_DELTA;

    size_t pos = 0;
    if (fscanf(stream, "%Iu ", &pos) <= 0)
        return ERR_INVALID_DELTA;

    out->type = type;
    out->pos = pos;
    fgetc(stream); // skip one space
    return
        SUCCESS == read_line(&out->text, stream)
        ? SUCCESS
        : ERR_INVALID_DELTA;
}

return_t delta_load(struct delta* out, FILE* stream)
{
    assert(out != NULL);
    assert(stream != NULL);

    struct delta new_delta = DELTA_INIT;
    struct delta_line next_line = DELTA_LINE_INIT;

    return_t ret;
    while ((ret = delta_line_load(&next_line, stream)) == SUCCESS)
        delta_append(&new_delta, next_line);

    if (ret == ERR_READ && feof(stream))
        ret = SUCCESS;

    if (ret == SUCCESS)
    {
        delta_free(out);
        *out = new_delta;
    }
    else
        delta_free(&new_delta);

    return ret;
}

void delta_line_free(struct delta_line* line)
{
    free(line->text);
    line->text = NULL;
}

void delta_free(struct delta* delta)
{
    for (size_t i = 0; i < delta->len; ++i)
        delta_line_free(delta->lines + i);
    free(delta->lines);
    *delta = delta_init;
}

return_t delta_line_print(const struct delta_line* line, FILE* stream)
{
    assert(line->type == DELTA_ADD || line->type == DELTA_ERASE);
    return
        0 < fprintf_s(stream, "%c %Iu %s\n", line->type, line->pos, line->text)
        ? SUCCESS
        : ERR_WRITE;
}

return_t delta_print(const struct delta* delta, FILE* stream)
{
    return_t ret = SUCCESS;
    for (size_t i = 0; ret == SUCCESS && i < delta->len; ++i)
        ret = delta_line_print(delta->lines + i, stream);
    return ret;
}

return_t delta_apply(char** text, const struct delta *delta)
{
    assert(text != NULL);

    return_t ret = SUCCESS;

    for (size_t i = 0; ret == SUCCESS && i < delta->len; ++i)
    {
        struct delta_line* line = delta->lines + i;

        assert(line->type == DELTA_ADD || line->type == DELTA_ERASE);
        if (line->type == DELTA_ADD)
            ret = string_insert(text, line->pos, line->text);
        else
            ret = string_erase(text, line->pos, strlen(line->text));
        assert((*text)[strlen(*text)] == '\0');
    }

    return ret;
}

return_t delta_apply_alloc(char** out,
        const struct delta* delta, const char* source)
{
    assert(source != NULL);
    assert(out != NULL);

    *out = string_copy_alloc(source);
    return_t err = delta_apply(out, delta);

    if (err != SUCCESS)
        free(*out);

    return err;
}

