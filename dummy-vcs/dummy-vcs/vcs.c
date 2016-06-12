#include "vcs.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "parse.h"
#include "delta.h"
#include "version_tree.h"

return_t vcs_open(struct vcs_state* vcs, const char* fname, int version)
{
    assert(vcs != NULL);
    assert(fname != NULL);

    struct vcs_state new_vcs = VCS_INIT;

    new_vcs.version = version;
    return_t ret = vt_load(&new_vcs.vt, fname);

    if (ret == SUCCESS)
        ret = vt_checkout(&new_vcs.clean_state, &new_vcs.vt, version);

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

    return_t ret = vcs_remove(vcs, i, j);
    assert(ret == SUCCESS);
    ret = vcs_add(vcs, i, data);
    assert(ret == SUCCESS);

    return SUCCESS;
}

return_t vcs_add(struct vcs_state* vcs, size_t i, const char* data)
{
    assert(vcs != NULL);

    return_t ret = string_insert(&vcs->working_state, i, data);
    if (ret == SUCCESS)
        delta_append(&vcs->changes,
                delta_line_new(i, string_copy_alloc(data), DELTA_ADD));
    return ret;
}

return_t vcs_remove(struct vcs_state* vcs, size_t i, size_t j)
{
    assert(vcs != NULL);

    if (check_substr(strlen(vcs->working_state), i, j-i, NULL))
    {
        struct substr substr = string_substr(vcs->working_state, i, j-i);
        delta_append(&vcs->changes,
                delta_line_new(i, substr_to_string_alloc(&substr), DELTA_ERASE));
    }
    return string_erase(&vcs->working_state, i, j - i);
}

return_t vcs_push(struct vcs_state* vcs)
{
    assert(vcs != NULL);

    return_t ret = vt_push(
            &vcs->version, &vcs->vt, vcs->version, &vcs->changes);

    if (ret == SUCCESS)
    {
        string_assign_copy(&vcs->clean_state, vcs->working_state);
        delta_free(&vcs->changes);
    }

    return ret;
}

void vcs_free(struct vcs_state* vcs)
{
    free(vcs->clean_state);
    free(vcs->working_state);
    delta_free(&vcs->changes);
    vt_free(&vcs->vt);
    *vcs = vcs_init;
}

