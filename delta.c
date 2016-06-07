#include "delta.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "my_string.h"


return_t delta_apply(struct string* text, const struct delta *delta)
{
    assert(text != NULL);
    assert(delta != NULL);

    return_t ret = SUCCESS;

    for (const struct delta_line* line = delta->lines;
            ret == SUCCESS && line != NULL;
            line = line->tail)
    {
        assert(line->type == DELTA_ADD || line->type == DELTA_ERASE);
        if (line->type == DELTA_ADD)
            ret = string_insert(text, line->pos, &line->text);
        else
            ret = string_erase(text, line->pos, line->erase_len);
        assert(text->data[text->len] == '\0');
    }

    return ret;
}

return_t delta_apply_alloc(struct string* out,
        const struct delta* delta, const struct string* source)
{
    assert(source != NULL);
    assert(!string_is_null(source));
    assert(out != NULL);

    return_t err;
    if ((err = string_copy_alloc(out, source)) != SUCCESS
     || (err = delta_apply(out, delta)) != SUCCESS)
    {
        string_free(out);
        return err;
    }

    return SUCCESS;
}

static return_t delta_calc_recursive(
        struct delta_line** out_first, struct delta_line** out_last,
        substring* a, substring* b)
{
    assert(out_first == NULL);
    assert(out_last == NULL);

    size_t common_len = 0;
    const char* common_a = NULL;
    const char* common_b = NULL;

    /* for (size_t len = min(a_size, b_size); !common_len && len; --len) */
    /* { */
        /* const char* a = a_begin; */
        /* for (size_t i = 0; i !common_len && + len <= a_size; ++i, ++a) */
        /* { */
            /* const char* b = b_begin; */
            /* for (size_t j = 0; !common_len && j + len <= b_size; ++j, ++b) */
                /* if (strncmp(a, b, len) == 0) */
                /* { */
                    /* common_len = len; */
                    /* common_a = a; */
                    /* common_b = b; */
                /* } */
        /* } */
    /* } */
}

return_t delta_calc(struct delta* out,
        const struct string* a, const struct string* b)
{
    assert(out != NULL);

    struct delta_line *first, *last;

    return_t ret = delta_calc_recursive(&first, &last, a, b);
    if (ret != SUCCESS)
        return ret;

    out->lines = first;
}

return_t delta_load(struct delta* out, FILE* stream)
{
    assert(out != NULL);

    struct delta res = DELTA_NULL;
    fscanf(stream, "%d\n", &res.parent); // TODO: check retval
    if (res.parent < 0)
        return ERR_INVALID_DELTA;

    struct delta_line** next_line_ptr = &res.lines;

    char type;
    while (fscanf(stream, "%1s", &type) > 0) // TODO: check retval
    {
        if (type != DELTA_ADD && type != DELTA_ERASE)
        {
            delta_free(&res);
            return ERR_INVALID_DELTA;
        }

        struct delta_line* new_line = calloc(1, sizeof(struct delta_line));
        if (new_line == NULL)
        {
            delta_free(&res);
            return ERR_BAD_ALLOC;
        }

        new_line->type = type;
        if (type == DELTA_ADD)
        {
            char* text;
            fscanf(stream, "%zu %ms", &new_line->pos, &text); // TODO proper reading
            string_assign_cstr(&new_line->text, text);
            free(text);
        }
        else
        {
            fscanf(stream, "%zu %zu", &new_line->pos, &new_line->erase_len); // TODO check this
        }

        *next_line_ptr = new_line;
        next_line_ptr = &new_line->tail;
    }

    *out = res;
    return SUCCESS;
}

return_t delta_save(const struct delta* delta, FILE* stream)
{
    assert(delta->parent != -1);

    fprintf(stream, "%d", delta->parent);
    for (const struct delta_line* line = delta->lines;
            line != NULL;
            line = line->tail)
    {
        assert(line->type == DELTA_ADD || line->type == DELTA_ERASE);
        if (line->type == DELTA_ADD)
            fprintf(stream, "+ %zu %s\n", line->pos, line->text.data);
        else
            fprintf(stream, "- %zu %zu\n", line->pos, line->erase_len);
    }
    // TODO: check fprintf return value: there could be errors
    return SUCCESS;
}

void delta_free(struct delta* delta)
{
    delta->parent = -1;

    struct delta_line* next_line = delta->lines;
    while (next_line != NULL)
    {
        struct delta_line* old_line = next_line;
        next_line = next_line->tail;
        free(old_line);
    }
}

