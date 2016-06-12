#pragma once

#include <stdio.h>

#include "error.h"
#include "delta.h"
#include "version_tree.h"

struct vcs_state
{
    char* clean_state;
    char* working_state;

    struct delta changes;

    struct version_tree vt;
    int version;
};
#define VCS_INIT { NULL, NULL, DELTA_INIT, VERSION_TREE_INIT, -1 }
static const struct vcs_state vcs_init = VCS_INIT;

return_t vcs_open(struct vcs_state* vcs, const char* fname, int version);
return_t vcs_print(const struct vcs_state* vcs, FILE* stream);

return_t vcs_edit(struct vcs_state* vcs, size_t i, size_t j, const char* data);
return_t vcs_add(struct vcs_state* vcs, size_t i, const char* data);
return_t vcs_remove(struct vcs_state* vcs, size_t i, size_t j);

return_t vcs_push(struct vcs_state* vcs);
return_t vcs_pull(struct vcs_state* vcs, int version);
return_t vcs_delete_version(struct vcs_state* vcs, int version);
return_t vcs_rebase(struct vcs_state* vcs);

return_t vcs_save(struct vcs_state* vcs, const char* path);

void vcs_free(struct vcs_state* vcs);

