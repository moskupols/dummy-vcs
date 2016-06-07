#include "vcs.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "parse.h"
#include "delta.h"

int revision_for_filename(const struct string* filename)
{
    char *p = filename->data + filename->len;
    while (p != filename->data && isdigit(*(p-1)))
        --p;
    if (*p == '\0')
        return 0;
    return atoi(p);
}

return_t filename_for_revision(struct string* filename, int revision)
{
    if (revision == 0)
        return SUCCESS;

    char *p = filename->data + filename->len;
    while (p != filename->data && *p != '.')
        --p;

    size_t suffix_pos = p - filename->data;
    if (suffix_pos == 0)
        suffix_pos = filename->len;

    return_t ret = string_reserve(filename, suffix_pos + 6);
    if (ret == SUCCESS)
        ret = string_erase(filename, suffix_pos, FICTIVE_LEN);

    if (ret == SUCCESS)
        sprintf(filename->data + suffix_pos, ".%d", revision);

    return ret;
}

void vcs_free(struct vcs_state* vcs)
{
    string_free(&vcs->working_state);
    string_free(&vcs->filename);
    *vcs = VCS_NULL;
}

return_t vcs_open(struct vcs_state* vcs, const struct string* fname, int version)
{
    assert(vcs != NULL);
    assert(fname != NULL);
    assert(!string_is_null(fname));

    return_t ret = SUCCESS;

    struct string versioned_file = STRING_NULL;
    ret = string_copy_alloc(&versioned_file, fname);

    if (ret == SUCCESS)
        ret = filename_for_revision(&versioned_file, version);

    FILE* f;
    if (ret == SUCCESS)
    {
        f = fopen(versioned_file.data, "r");
        if (f == NULL)
            ret = ERR_NO_SUCH_FILE;
    }

    if (ret == SUCCESS)
    {
        ret = read_all(&vcs->working_state, f);
        fclose(f);
    }

    if (ret == SUCCESS)
    {
        string_free(&vcs->filename);
        vcs->filename = versioned_file;
        vcs->version = version;
    }
    else
        string_free(&versioned_file);
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

    return_t ret = SUCCESS;

    if (data->len > len)
        ret = string_reserve(&vcs->working_state,
                vcs->working_state.len + data->len - len);

    if (ret == SUCCESS)
    {
        ret = string_erase(&vcs->working_state, i, len);
        assert(ret == SUCCESS);
        ret = string_insert(&vcs->working_state, i, data);
        assert(ret == SUCCESS);
    }
    return ret;
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
    return_t ret = string_copy_alloc(new_filename, &vcs->filename);
    for (int v = vcs->version + 1; v < 100000 && ret == SUCCESS; ++v)
    {
        ret = filename_for_revision(new_filename, v);
        if (ret != SUCCESS)
            return ret;

        if (!file_exists(new_filename))
        {
            *new_version = v;
            return SUCCESS;
        }
    }
    return ERR_VERSIONS_LIMIT;
}

return_t vcs_push(struct vcs_state* vcs)
{
    assert(vcs != NULL);

    FILE* clean_file = fopen(vcs->filename.data, "r");
    if (clean_file == NULL)
        return ERR_NO_SUCH_FILE;

    struct string clean_state = STRING_NULL;
    return_t ret = read_all(&clean_state, clean_file);

    struct delta delta = DELTA_NULL;
    delta.parent = vcs->version;
    if (ret == SUCCESS)
    {
        fclose(clean_file);
        string_shrink(&vcs->working_state);
        ret = delta_calc(&delta, &clean_state, &vcs->working_state);
    }

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
        string_free(&vcs->filename);
        string_free(&clean_state);
        delta_free(&delta);

        vcs->filename = new_filename;
        vcs->version = new_version;
    }

    return ret;
}

