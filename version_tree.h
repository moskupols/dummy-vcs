#pragma once

#include "vcs.h"

int version_for_filename(const char* filename);
char* filename_for_version(const char* base_fname, int version);

int get_parent(struct vcs_state* vcs, int v);

