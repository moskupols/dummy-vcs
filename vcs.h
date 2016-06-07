#pragma once

#include <stdio.h>

#include "my_string.h"
#include "error.h"

int revision_for_filename(const struct string* filename);
return_t filename_for_revision(struct string* filename, int revision);

return_t vcs_open(const struct string* fname, int version);
return_t vcs_print(FILE* stream);

return_t vcs_edit(size_t i, size_t j, const struct string* data);
return_t vcs_add(size_t i, const struct string* data);
return_t vcs_remove(size_t i, size_t j);

return_t vcs_push();
return_t vcs_pull(int version);
return_t vcs_delete_version(int version);
return_t vcs_rebase();

int vcs_get_current_version();

