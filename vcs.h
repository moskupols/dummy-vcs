#pragma once

#include <stdio.h>

#include "my_string.h"
#include "error.h"

struct vcs_state
{
    char* working_state;
    char* base_filename;
    char* cur_filename;
    int version;
};
#define VCS_NULL ((struct vcs_state){NULL, NULL, NULL, -1})

int version_for_filename(const char* filename);
char* filename_for_version(const char* filename, int version);

void vcs_free(struct vcs_state* vcs);

return_t vcs_open(struct vcs_state* vcs, const char* fname, int version);
return_t vcs_print(const struct vcs_state* vcs, FILE* stream);

return_t vcs_edit(struct vcs_state* vcs, size_t i, size_t j, const char* data);
return_t vcs_add(struct vcs_state* vcs, size_t i, const char* data);
return_t vcs_remove(struct vcs_state* vcs, size_t i, size_t j);

return_t vcs_push(struct vcs_state* vcs);
return_t vcs_pull(struct vcs_state* vcs, int version);
return_t vcs_delete_version(struct vcs_state* vcs, int version);
return_t vcs_rebase(struct vcs_state* vcs);

