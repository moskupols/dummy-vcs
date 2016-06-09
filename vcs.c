#include "vcs.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "parse.h"
#include "delta.h"

int version_for_filename(const struct string* filename)
{
    char *p = filename->data + filename->len;
    while (p != filename->data && isdigit(*(p-1)))
        --p;
    if (*p == '\0')
        return 0;
    return atoi(p);
}

struct string filename_for_version(const struct string* filename, int version)
{
    struct string ret = string_copy_alloc(filename);

    if (version == 0)
        return ret;

    char *p = ret.data + ret.len;
    while (p != ret.data && *p != '.')
        --p;

    size_t suffix_pos = p - ret.data;
    if (suffix_pos == 0)
        suffix_pos = ret.len;

    return_t err = string_erase(&ret, suffix_pos, FICTIVE_LEN);
    assert(err == SUCCESS);

    char buf[10];
    sprintf(buf, ".%d", version);
    struct string s = string_from_cstr(buf);

    err = string_insert(&ret, suffix_pos, &s);
    assert(err == SUCCESS);

    return ret;
}

void vcs_free(struct vcs_state* vcs)
{
    string_free(&vcs->working_state);
    string_free(&vcs->base_filename);
    string_free(&vcs->cur_filename);
    *vcs = VCS_NULL;
}

return_t vcs_open(struct vcs_state* vcs, const struct string* fname, int version)
{
    assert(vcs != NULL);
    assert(fname != NULL);
    assert(!string_is_null(fname));

    struct string versioned_file = filename_for_version(fname, version);

    FILE* f;
    return_t ret = SUCCESS;
    f = fopen(versioned_file.data, "r");
    if (f == NULL)
        ret = ERR_NO_SUCH_FILE;

    struct string new_working_state = STRING_NULL;
    if (ret == SUCCESS)
    {
        ret = read_all(&new_working_state, f);
        fclose(f);
    }

    if (ret == SUCCESS)
    {
        vcs_free(vcs);

        vcs->working_state = new_working_state;
        vcs->base_filename = string_copy_alloc(fname);
        vcs->cur_filename = versioned_file;
        vcs->version = version;
    }
    else
    {
        string_free(&versioned_file);
        string_free(&new_working_state);
    }

    return ret;
}

return_t vcs_print(const struct vcs_state* vcs, FILE* stream)
{
    return fputs(vcs->working_state.data, stream) >= 0 
        ? SUCCESS
        : ERR_NO_SUCH_FILE;
}

return_t vcs_edit(struct vcs_state* vcs,
        size_t i, size_t j, const struct string* data)
{
    assert(vcs != NULL);
    assert(data != NULL);
    assert(!string_is_null(data));

    size_t len = j - i + 1;
    if (i > j || !check_substr(vcs->working_state.len, i, len, NULL))
        return ERR_INVALID_RANGE;

    if (data->len > len)
        string_reserve(&vcs->working_state,
            vcs->working_state.len + data->len - len);

    return_t ret = string_erase(&vcs->working_state, i, len);
    assert(ret == SUCCESS);
    ret = string_insert(&vcs->working_state, i, data);
    assert(ret == SUCCESS);

    return SUCCESS;
}

return_t vcs_add(struct vcs_state* vcs, size_t i, const struct string* data)
{
    assert(vcs != NULL);

    return string_insert(&vcs->working_state, i, data);
}

return_t vcs_remove(struct vcs_state* vcs, size_t i, size_t j)
{
    assert(vcs != NULL);

    return string_erase(&vcs->working_state, i, j - i + 1);
}

static bool file_exists(const struct string * path)
{
    FILE* f = fopen(path->data, "r");
    if (f == NULL)
        return false;
    fclose(f);
    return true;
}

static return_t find_new_version(
        int* new_version, struct string* new_filename,
        struct vcs_state* vcs)
{
    for (int v = vcs->version + 1; v < 100000; ++v)
    {
        *new_filename = filename_for_version(&vcs->base_filename, v);

        if (!file_exists(new_filename))
        {
            *new_version = v;
            return SUCCESS;
        }
        string_free(new_filename);
    }
    return ERR_VERSIONS_LIMIT;
}

return_t vcs_push(struct vcs_state* vcs)
{
    assert(vcs != NULL);

    FILE* clean_file = fopen(vcs->cur_filename.data, "r");
    if (clean_file == NULL)
        return ERR_NO_SUCH_FILE;

    struct string clean_state = STRING_NULL;
    return_t ret = read_all(&clean_state, clean_file);

    struct delta delta = DELTA_NULL;
    if (ret == SUCCESS)
    {
        fclose(clean_file);
        string_shrink(&vcs->working_state);
        delta = delta_calc(&clean_state, &vcs->working_state);
    }
    delta.parent = vcs->version;

    int new_version;
    struct string new_filename;
    if (ret == SUCCESS)
        ret = find_new_version(&new_version, &new_filename, vcs);

    FILE* new_file;
    if (ret == SUCCESS)
    {
        new_file = fopen(new_filename.data, "w");
        if (new_file == NULL)
            ret = ERR_WRITE_ERROR;
    }

    if (ret == SUCCESS)
    {
        ret = delta_save(&delta, new_file);
        fclose(new_file);
    }

    if (ret == SUCCESS)
    {
        string_free(&vcs->cur_filename);
        string_free(&clean_state);
        delta_free(&delta);

        vcs->cur_filename = new_filename;
        vcs->version = new_version;
    }

    return ret;
}

