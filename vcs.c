#include "vcs.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "parse.h"
#include "delta.h"
#include "version_tree.h"

static return_t load_version(char** out, struct vcs_state* vcs, int version)
{
    assert(version >= 0);

    if (version == 0)
        return read_file(out, vcs->base_filename);

    return_t ret = SUCCESS;

    char* version_filename = filename_for_version(vcs->base_filename, version);
    FILE* f = fopen(version_filename, "r");
    free(version_filename);

    if (f == NULL)
        ret = ERR_NO_SUCH_FILE;

    struct delta delta = DELTA_NULL;
    if (ret == SUCCESS)
        ret = delta_load_parent(&delta.parent, f);

    char* text = NULL;
    if (ret == SUCCESS)
        ret = load_version(&text, vcs, delta.parent);

    if (ret == SUCCESS)
        ret = delta_load_lines(&delta.lines, f);

    if (ret == SUCCESS)
        ret = delta_lines_apply(&text, delta.lines);

    if (ret == SUCCESS)
        *out = text;
    else
        free(text);

    delta_free(&delta);
    if (f)
        fclose(f);

    return ret;
}

return_t vcs_open(struct vcs_state* vcs, const char* fname, int version)
{
    assert(vcs != NULL);
    assert(fname != NULL);

    struct vcs_state new_vcs = VCS_NULL;

    new_vcs.base_filename = string_copy_alloc(fname);
    new_vcs.version = version;
    return_t ret = load_version(&new_vcs.clean_state, &new_vcs, version);

    if (ret == SUCCESS)
    {
        new_vcs.working_state = string_copy_alloc(new_vcs.clean_state);

        vcs_free(vcs);
        *vcs = new_vcs;
    }
    else
        vcs_free(&new_vcs);

    return ret;
}

return_t vcs_print(const struct vcs_state* vcs, FILE* stream)
{
    return fputs(vcs->working_state, stream) >= 0 
        ? SUCCESS
        : ERR_NO_SUCH_FILE;
}

return_t vcs_edit(struct vcs_state* vcs,
        size_t i, size_t j, const char* data)
{
    assert(vcs != NULL);
    assert(data != NULL);

    size_t len = j - i;
    if (i >= j || !check_substr(strlen(vcs->working_state), i, len, NULL))
        return ERR_INVALID_RANGE;

    return_t ret = string_erase(&vcs->working_state, i, len);
    assert(ret == SUCCESS);
    ret = string_insert(&vcs->working_state, i, data);
    assert(ret == SUCCESS);

    return SUCCESS;
}

return_t vcs_add(struct vcs_state* vcs, size_t i, const char* data)
{
    assert(vcs != NULL);

    return string_insert(&vcs->working_state, i, data);
}

return_t vcs_remove(struct vcs_state* vcs, size_t i, size_t j)
{
    assert(vcs != NULL);

    return string_erase(&vcs->working_state, i, j - i);
}

static bool file_exists(const char * path)
{
    FILE* f = fopen(path, "r");
    if (f == NULL)
        return false;
    fclose(f);
    return true;
}

static return_t find_new_version(
        int* new_version, char** new_filename,
        struct vcs_state* vcs)
{
    for (int v = vcs->version + 1; v < 100000; ++v)
    {
        *new_filename = filename_for_version(vcs->base_filename, v);

        if (!file_exists(*new_filename))
        {
            *new_version = v;
            return SUCCESS;
        }
        free(*new_filename);
    }
    return ERR_VERSIONS_LIMIT;
}

return_t vcs_push(struct vcs_state* vcs)
{
    assert(vcs != NULL);

    string_shrink(&vcs->working_state);
    struct delta delta = delta_calc(vcs->clean_state, vcs->working_state, vcs->version);

    int new_version;
    char* new_filename = NULL;
    return_t ret = find_new_version(&new_version, &new_filename, vcs);

    FILE* new_file;
    if (ret == SUCCESS)
    {
        new_file = fopen(new_filename, "w");
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
        string_assign_copy(&vcs->clean_state, vcs->working_state);
        vcs->version = new_version;
    }
    delta_free(&delta);
    free(new_filename);

    return ret;
}

void vcs_free(struct vcs_state* vcs)
{
    free(vcs->clean_state);
    free(vcs->working_state);
    free(vcs->base_filename);
    *vcs = VCS_NULL;
}

