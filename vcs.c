#include "vcs.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>

#include "parse.h"

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

