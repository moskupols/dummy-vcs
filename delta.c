#include "delta.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "my_string.h"
#include "parse.h"
#include "utils.h"

static void delta_line_free(struct delta_line* line)
{
    free(line->text);
}

static void free_delta_lines(struct delta_line* head)
{
    if (head == NULL)
        return;
    free_delta_lines(head->tail);
    delta_line_free(head);
    free(head);
}

return_t delta_apply(char** text, const struct delta *delta)
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
            ret = string_insert(text, line->pos, line->text);
        else
            ret = string_erase(text, line->pos, line->erase_len);
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
    {
        free(out);
        return err;
    }

    return SUCCESS;
}

static void find_max_common_substr(
        struct substr* out_a, struct substr* out_b,
        const struct substr* a, const struct substr* b)
{
    size_t common_a_pos = 0, common_b_pos = 0, common_len = 0;

    for (size_t len = min(a->len, b->len); !common_len && len; --len)
    {
        const char* pa = substr_begin(a);
        for (size_t i = 0; !common_len && i + len <= a->len; ++i, ++pa)
        {
            const char* pb = substr_begin(b);
            for (size_t j = 0; !common_len && j + len <= b->len; ++j, ++pb)
                if (strncmp(pa, pb, len) == 0)
                {
                    common_len = len;
                    common_a_pos = i;
                    common_b_pos = j;
                }
        }
    }

    *out_a = substr_substr(a, common_a_pos, common_len);
    *out_b = substr_substr(b, common_b_pos, common_len);
}

static void delta_calc_replace(
        struct delta_line** out_first, struct delta_line** out_last,
        const struct substr* a, const struct substr* b)
{
    struct delta_line* first = checked_calloc(1, sizeof(struct delta_line));
    struct delta_line* last = first;

    if (a->len)
    {
        first->type = DELTA_ERASE;
        first->pos = a->pos;
        first->erase_len = a->len;
    }
    if (b->len)
    {
        if (a->len)
            first->tail = last = checked_calloc(1, sizeof(struct delta_line));
        last->text = substr_to_string_alloc(b);
        last->type = DELTA_ADD;
        last->pos = a->pos;
    }

    *out_first = first;
    *out_last = last;
}

static void delta_calc_recursive(
        struct delta_line** out_first, struct delta_line** out_last,
        struct substr a, struct substr b)
{
    assert(out_first != NULL);
    assert(out_last != NULL);

    if (a.len + b.len == 0)
    {
        *out_first = *out_last = NULL;
        return;
    }

    struct substr comm_a, comm_b;
    find_max_common_substr(&comm_a, &comm_b, &a, &b);
    assert(comm_a.len == comm_b.len);

    if (comm_a.len == 0)
        return delta_calc_replace(out_first, out_last, &a, &b);

    struct delta_line *first = NULL, *mid_l = NULL, *mid_r = NULL, *last = NULL;

    delta_calc_recursive(&first, &mid_l,
        substr_substr(&a, comm_a.pos + comm_a.len, FICTIVE_LEN),
        substr_substr(&b, comm_b.pos + comm_b.len, FICTIVE_LEN));
    delta_calc_recursive(&mid_r, &last,
        substr_substr(&a, 0, comm_a.pos),
        substr_substr(&b, 0, comm_b.pos));

    if (first == NULL)
        first = mid_r;
    else if (last == NULL)
        last = mid_l;
    else
        mid_l->tail = mid_r;

    assert(first);
    assert(last);

    *out_first = first;
    *out_last = last;
}

struct delta delta_calc(const char* a, const char* b)
{
    assert(a != NULL);
    assert(b != NULL);

    struct delta ret = DELTA_NULL;

    if (strcmp(a, b) == 0)
        return ret;

    struct delta_line *last;

    delta_calc_recursive(
        &ret.lines, &last,
        string_substr(a, 0, FICTIVE_LEN), string_substr(b, 0, FICTIVE_LEN));

    return ret;
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
    while (fscanf(stream, "\n%c", &type) > 0) // TODO: check retval
    {
        if (type != DELTA_ADD && type != DELTA_ERASE)
        {
            delta_free(&res);
            return ERR_INVALID_DELTA;
        }

        struct delta_line* new_line = checked_calloc(1, sizeof(struct delta_line));

        new_line->type = type;
        if (type == DELTA_ADD)
        {
            fscanf(stream, "%zu ", &new_line->pos);
            read_line(&new_line->text, stream);
        }
        else
        {
            fscanf(stream, "%zu %zu", &new_line->pos, &new_line->erase_len); // TODO check this
        }

        *next_line_ptr = new_line;
        next_line_ptr = &new_line->tail;
    }

    if (out->lines)
        delta_free(out);
    *out = res;
    return SUCCESS;
}

return_t delta_save(const struct delta* delta, FILE* stream)
{
    assert(delta->parent != -1);

    fprintf(stream, "%d\n", delta->parent);
    for (const struct delta_line* line = delta->lines;
            line != NULL;
            line = line->tail)
    {
        assert(line->type == DELTA_ADD || line->type == DELTA_ERASE);
        if (line->type == DELTA_ADD)
            fprintf(stream, "+ %zu %s\n", line->pos, line->text);
        else
            fprintf(stream, "- %zu %zu\n", line->pos, line->erase_len);
    }
    // TODO: check fprintf return value: there could be errors
    return SUCCESS;
}

void delta_free(struct delta* delta)
{
    free_delta_lines(delta->lines);
    /* delta->parent = -1; */
    delta->lines = NULL;
}

