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

void delta_lines_free(struct delta_line* head)
{
    if (head == NULL)
        return;
    delta_lines_free(head->tail);
    delta_line_free(head);
    free(head);
}

return_t delta_lines_apply(char** text, const struct delta_line *lines)
{
    assert(text != NULL);
    if (lines == NULL)
        return SUCCESS;

    return_t ret = SUCCESS;

    for (const struct delta_line* line = lines;
            ret == SUCCESS && line != NULL;
            line = line->tail)
    {
        assert(line->type == DELTA_ADD || line->type == DELTA_ERASE);
        if (line->type == DELTA_ADD)
            ret = string_insert(text, line->pos, line->text);
        else
            ret = string_erase(text, line->pos, strlen(line->text));
        assert((*text)[strlen(*text)] == '\0');
    }

    return ret;
}

return_t delta_lines_apply_alloc(char** out,
        const struct delta_line* lines, const char* source)
{
    assert(source != NULL);
    assert(out != NULL);

    *out = string_copy_alloc(source);
    return_t err = delta_lines_apply(out, lines);

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
        *first = (struct delta_line)
        {
            .type = DELTA_ERASE,
            .pos = a->pos,
            .text = substr_to_string_alloc(a),
        };
    if (b->len)
    {
        if (a->len)
            first->tail = last = checked_calloc(1, sizeof(struct delta_line));
        *last = (struct delta_line)
        {
            .type = DELTA_ADD,
            .pos = a->pos,
            .text = substr_to_string_alloc(b),
        };
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

struct delta_line* delta_calc_lines(const char* a, const char* b)
{
    assert(a != NULL);
    assert(b != NULL);

    if (strcmp(a, b) == 0)
        return NULL;

    struct delta_line *first, *last;

    delta_calc_recursive(
        &first, &last,
        string_substr(a, 0, FICTIVE_LEN), string_substr(b, 0, FICTIVE_LEN));

    return first;
}

struct delta delta_calc(const char* a, const char* b, int parent)
{
    return (struct delta)
    {
        .parent = parent,
        .lines = delta_calc_lines(a, b)
    };
}

return_t delta_load_parent(int* out, FILE* stream)
{
    assert(out != NULL);
    assert(stream != NULL);

    fscanf(stream, "%d\n", out);
    return SUCCESS;
}

return_t delta_load_lines(struct delta_line** out, FILE* stream)
{
    assert(out != NULL);
    assert(stream != NULL);

    struct delta_line* first = NULL;
    struct delta_line** next_line_ptr = &first;

    char type;
    while (fscanf(stream, "\n%c", &type) > 0) // TODO: check retval
    {
        if (type != DELTA_ADD && type != DELTA_ERASE)
        {
            delta_lines_free(first);
            return ERR_INVALID_DELTA;
        }

        struct delta_line* new_line = checked_calloc(1, sizeof(struct delta_line));

        new_line->type = type;
        fscanf(stream, "%zu ", &new_line->pos);
        read_line(&new_line->text, stream);

        *next_line_ptr = new_line;
        next_line_ptr = &new_line->tail;
    }
    *out = first;
    return SUCCESS;
}

return_t delta_load(struct delta* out, FILE* stream)
{
    return_t ret = delta_load_parent(&out->parent, stream);
    if (ret == SUCCESS)
        ret = delta_load_lines(&out->lines, stream);
    return ret;
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
        fprintf(stream, "%c %zu %s\n", line->type, line->pos, line->text);
    }
    // TODO: check fprintf return value: there could be errors
    return SUCCESS;
}

void delta_free(struct delta* delta)
{
    delta_lines_free(delta->lines);
    /* delta->parent = -1; */
    delta->lines = NULL;
}

