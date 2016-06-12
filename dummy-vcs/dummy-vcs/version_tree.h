#pragma once

#include <stdio.h>

#include "error.h"
#include "delta.h"

//return_t read_parent(int* out, FILE* f);
//return_t print_parent(int parent, FILE* f);
//
//int version_for_filename(const char* filename);
//char* filename_for_version(const char* base_fname, int version);

struct version_tree
{
    char* base_fname;

    int* parent;
    size_t capacity;
};
#define VERSION_TREE_INIT { NULL, NULL, 0 }
static const struct version_tree version_tree_init = VERSION_TREE_INIT;

return_t vt_load(struct version_tree* out, const char* base_fname);
void vt_free(struct version_tree* vt);

int vt_get_parent(struct version_tree* vt, int child);
bool vt_version_is_known(struct version_tree* vt, int version);

int vt_find_common_ancestor(struct version_tree* vt, int a, int b);

return_t vt_apply_path(char** text, struct version_tree* vt, int start, int dest);
return_t vt_checkout_from_root(char** out, struct version_tree* vt, int version);

return_t vt_push(
        int* child, struct version_tree* vt, int parent, struct delta* delta);

return_t vt_delete_version(struct version_tree* vt, int version);

return_t vt_reverse_from_root(struct version_tree* vt, int version);
