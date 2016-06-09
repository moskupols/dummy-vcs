#include "vcs.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "parse.h"
#include "delta.h"

int version_for_filename(const char* filename)
{
    const char *p = filename + strlen(filename);
    while (p != filename && isdigit(*(p-1)))
        --p;
    if (*p == '\0')
        return 0;
    return atoi(p);
}

char* filename_for_version(const char* filename, int version)
{
    char* ret = string_copy_alloc(filename);

    if (version == 0)
        return ret;

    size_t len = strlen(ret);
    char *p = ret + len;
    while (p != ret && *p != '.')
        --p;

    size_t suffix_pos = p - ret;
    if (suffix_pos == 0)
        suffix_pos = len;

    return_t err = string_erase(&ret, suffix_pos, FICTIVE_LEN);
    assert(err == SUCCESS);

    char buf[10];
    sprintf(buf, ".%d", version);

    err = string_insert(&ret, suffix_pos, buf);
    assert(err == SUCCESS);

    return ret;
}

void vcs_free(struct vcs_state* vcs)
{
    free(vcs->working_state);
    free(vcs->base_filename);
    free(vcs->cur_filename);
    *vcs = VCS_NULL;
}

return_t vcs_open(struct vcs_state* vcs, const char* fname, int version)
{
    assert(vcs != NULL);
    assert(fname != NULL);

    char* versioned_file = filename_for_version(fname, version);

    FILE* f;
    return_t ret = SUCCESS;
    f = fopen(versioned_file, "r");
    if (f == NULL)
        ret = ERR_NO_SUCH_FILE;

    char* new_working_state = NULL;
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
        free(versioned_file);
        free(new_working_state);
    }

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

    FILE* clean_file = fopen(vcs->cur_filename, "r");
    if (clean_file == NULL)
        return ERR_NO_SUCH_FILE;

    char* clean_state = NULL;
    return_t ret = read_all(&clean_state, clean_file);

    struct delta delta = DELTA_NULL;
    if (ret == SUCCESS)
    {
        fclose(clean_file);
        string_shrink(&vcs->working_state);
        delta = delta_calc(clean_state, vcs->working_state, vcs->version);
    }

    int new_version;
    char* new_filename = NULL;
    if (ret == SUCCESS)
        ret = find_new_version(&new_version, &new_filename, vcs);

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
        free(vcs->cur_filename);
        free(clean_state);
        delta_free(&delta);

        vcs->cur_filename = new_filename;
        vcs->version = new_version;
    }

    return ret;
}

