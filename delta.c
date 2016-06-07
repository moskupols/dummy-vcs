#include "delta.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "my_string.h"
#include "utils.h"

static void delta_line_free(struct delta_line* line)
{
    string_free(&line->text);
}

static void free_delta_lines(struct delta_line* head)
{
    if (head == NULL)
        return;
    free_delta_lines(head->tail);
    delta_line_free(head);
    free(head);
}

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

static return_t delta_calc_recursive(
        struct delta_line** out_first, struct delta_line** out_last,
        struct substr a, struct substr b)
{
    assert(out_first != NULL);
    assert(out_last != NULL);

    if (a.len + b.len == 0)
    {
        *out_first = *out_last = NULL;
        return SUCCESS;
    }

    struct substr comm_a, comm_b;
    find_max_common_substr(&comm_a, &comm_b, &a, &b);
    assert(comm_a.len == comm_b.len);

    return_t ret = SUCCESS;

    if (comm_a.len == 0)
    {
        struct delta_line* first = calloc(1, sizeof(struct delta_line));

        if (a.len)
        {
            first->type = DELTA_ERASE;
            first->pos = a.pos;
            first->erase_len = a.len;
        }
        else
        {
            assert(b.len);
            first->type = DELTA_ADD;
            first->pos = a.pos;
            if ((ret = substr_to_string_alloc(&first->text, &b)) != SUCCESS)
            {
                free(first);
                return ret;
            }
        }
        *out_first = *out_last = first;
        return SUCCESS;
    }

    struct delta_line *first, *mid_l, *mid_r, *last;
    if ((ret = delta_calc_recursive(&first, &mid_l,
            substr_substr(&a, 0, comm_a.pos),
            substr_substr(&b, 0, comm_b.pos)))
        != SUCCESS)
    {
        return ret;
    }

    if ((ret = delta_calc_recursive(&mid_r, &last,
            substr_substr(&a, comm_a.pos + comm_a.len, FICTIVE_LEN),
            substr_substr(&b, comm_b.pos + comm_b.len, FICTIVE_LEN)))
        != SUCCESS)
    {
        free_delta_lines(first);
        return ret;
    }

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
    return SUCCESS;
}

return_t delta_calc(struct delta* out,
        const struct string* a, const struct string* b)
{
    assert(out != NULL);

    struct delta_line *first, *last;

    return_t ret = delta_calc_recursive(
            &first, &last,
            string_substr(a, 0, a->len), string_substr(b, 0, b->len));
    if (ret != SUCCESS)
        return ret;

    out->lines = first;
    return SUCCESS;
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

        struct delta_line* new_line = calloc(1, sizeof(struct delta_line));
        if (new_line == NULL)
        {
            delta_free(&res);
            return ERR_NO_MEMORY;
        }

        new_line->type = type;
        if (type == DELTA_ADD)
        {
            char* text;
            fscanf(stream, "%zu %ms", &new_line->pos, &text); // TODO proper reading
            string_assign_cstr(&new_line->text, text);
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

    fprintf(stream, "%d\n", delta->parent);
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
    free_delta_lines(delta->lines);
}

