#include "version_tree.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include <windows.h>

#include "delta.h"
#include "parse.h"
#include "utils.h"

static return_t read_parent(int* out, FILE* f)
{
    assert(f);

    int p;
    if (fscanf(f, "%d\n", &p) <= 0)
        return ERR_READ;
    if (out != NULL)
        *out = p;
    return SUCCESS;
}

static return_t read_parent_from_path(int* out, const char* fname)
{
    assert(fname);

    FILE* f = fopen(fname, "r");
    if (f == NULL)
        return ERR_NO_SUCH_FILE;

    return_t ret = read_parent(out, f);
    fclose(f);
    return ret;
}

static return_t print_parent(int parent, FILE* f)
{
    assert(f);

    return 0 < fprintf(f, "%d\n", parent)
        ? SUCCESS
        : ERR_WRITE;
}

static const char* find_extension(const char* fname)
{
    const char* p = strrchr(fname, '.');
    return p != NULL ? p : fname + strlen(fname);
}

static int version_for_filename(const char* filename, const char* base_fname)
{
    assert(filename != NULL);
    assert(base_fname != NULL);

    const char* p = find_extension(filename);
    const char* q = find_extension(base_fname);
    if (p - filename != q - base_fname || strncmp(filename, base_fname, p - filename) != 0)
        return -1;
    if (strcmp(p, q) == 0)
        return 0;

    int version = 0;
    for (const char* i = p + 1; *i; ++i)
    {
        if (!isdigit(*i))
            return -1;
        version = version * 10 + (*i - '0');
    }

    return version ? version : -1;
}

static void replace_extension(char** fname, const char* new_extension)
{
    size_t suffix_pos = find_extension(*fname) - *fname;

    return_t err = string_erase(fname, suffix_pos, FICTIVE_LEN);
    assert(err == SUCCESS);

    err = string_insert(fname, suffix_pos, new_extension);
    assert(err == SUCCESS);
}

static void switch_filename_to_version(char** fname, int version)
{
    if (version == 0)
        return;

    char buf[10];
    sprintf(buf, ".%d", version);
    replace_extension(fname, buf);
}

static void set_parent(struct version_tree* vt, int child, int parent)
{
    assert(child >= 0);

    if (vt->capacity <= (size_t)child)
    {
        size_t old_capacity = vt->capacity;

        vt->capacity = max(vt->capacity, 3);
        while (vt->capacity <= (size_t)child)
            vt->capacity *= 2;

        checked_realloc((void**)&vt->parent, vt->capacity * sizeof(int));
        memset(vt->parent + old_capacity, -1, (vt->capacity - old_capacity) * sizeof(int));
    }
    vt->parent[child] = parent;
}

return_t vt_load(struct version_tree* vt, const char* base_fname)
{
    vt_free(vt);

    char* fname = string_copy_alloc(base_fname);
    replace_extension(&fname, ".*");

    WIN32_FIND_DATA file_data;
    HANDLE dir_handle = FindFirstFile(fname, &file_data);

    if (dir_handle != NULL)
    {
        do
        {
            int child = version_for_filename(file_data.cFileName, base_fname);
            if (child <= 0)
                continue;

            int parent = -1;
            if (read_parent_from_path(&parent, file_data.cFileName) == SUCCESS)
                set_parent(vt, child, parent);
        } while (FindNextFile(dir_handle, &file_data));
        FindClose(dir_handle);
    }

    string_assign_copy(&fname, base_fname);
    vt->base_fname = fname;

    return SUCCESS;
}

void vt_free(struct version_tree* vt)
{
    free(vt->base_fname);
    free(vt->parent);
    *vt = version_tree_init;
}

int vt_get_parent(struct version_tree* vt, int child)
{
    if (child <= 0 || (size_t)child >= vt->capacity)
        return -1;
    return vt->parent[child];
}

bool vt_version_is_known(struct version_tree* vt, int version)
{
    return version == 0 || vt_get_parent(vt, version) > -1;
}

int vt_find_common_ancestor(struct version_tree* vt, int a, int b)
{
    assert(vt_version_is_known(vt, a));
    assert(vt_version_is_known(vt, b));

    if (a == 0 || b == 0)
        return 0;

    for (int i = a; i >= 0; i = vt_get_parent(vt, i))
        for (int j = b; j >= 0; j = vt_get_parent(vt, j))
            if (i == j)
                return i;
    return -1;
}

static return_t save_deltas(struct version_tree* vt, int version,
    int parent, const struct delta* delta_a, const struct delta* delta_b)
{
    char* fname = string_copy_alloc(vt->base_fname);
    switch_filename_to_version(&fname, version);
    FILE* f = fopen(fname, "w");
    free(fname);
    if (f == NULL)
        return ERR_WRITE;
    print_parent(parent, f);

    return_t ret = SUCCESS;
    if (delta_a != NULL)
        ret = delta_print(delta_a, f);
    if (ret == SUCCESS && delta_b != NULL)
        ret = delta_print(delta_b, f);
    fclose(f);
    return ret;
}

static return_t load_delta(struct delta* delta, struct version_tree* vt, int version)
{
    assert(vt_version_is_known(vt, version));

    char* version_filename = string_copy_alloc(vt->base_fname);
    switch_filename_to_version(&version_filename, version);
    FILE* f = fopen(version_filename, "r");
    free(version_filename);

    return_t ret = f == NULL ? ret = ERR_NO_SUCH_FILE : SUCCESS;

    if (ret == SUCCESS)
        ret = read_parent(NULL, f);

    if (ret == SUCCESS)
        ret = delta_load(delta, f);

    if (f != NULL)
        fclose(f);

    return ret;
}

static return_t apply_upwards(char** text, struct version_tree* vt, int descendant, int ancestor)
{
    assert(vt_version_is_known(vt, ancestor));
    assert(vt_version_is_known(vt, descendant));

    if (descendant == ancestor)
        return SUCCESS;

    struct delta delta = DELTA_INIT;
    return_t ret = load_delta(&delta, vt, descendant);
    delta_reverse(&delta);

    if (ret == SUCCESS)
        ret = delta_apply(text, &delta);

    delta_free(&delta);

    return ret == SUCCESS ? apply_upwards(text, vt, vt_get_parent(vt, descendant), ancestor) : ret;
}

static return_t apply_downwards(char** text, struct version_tree* vt, int ancestor, int descendant)
{
    assert(vt_version_is_known(vt, ancestor));
    assert(vt_version_is_known(vt, descendant));

    if (ancestor == descendant)
        return SUCCESS;

    return_t ret = apply_downwards(text, vt, ancestor, vt_get_parent(vt, descendant));
    struct delta delta = DELTA_INIT;
    if (ret == SUCCESS)
        ret = load_delta(&delta, vt, descendant);
    if (ret == SUCCESS)
        ret = delta_apply(text, &delta);
    delta_free(&delta);

    return ret;
}

return_t vt_checkout_from_root(
        char** out, struct version_tree* vt, int version)
{
    if (!vt_version_is_known(vt, version))
        return ERR_INVALID_VERSION;
    char* base_text = NULL;
    return_t ret = read_file(&base_text, vt->base_fname);
    if (ret == SUCCESS)
        ret = apply_downwards(&base_text, vt, 0, version);
    if (ret == SUCCESS)
        *out = base_text;
    return ret;
}

return_t vt_apply_path(char** text, struct version_tree* vt, int start, int dest)
{
    if (!vt_version_is_known(vt, start) || !vt_version_is_known(vt, dest))
        return ERR_INVALID_VERSION;
    int common_ancestor = vt_find_common_ancestor(vt, start, dest);
    if (common_ancestor < 0)
        return ERR_INVALID_VERSION;

    char* copy = string_copy_alloc(*text);
    return_t ret = apply_upwards(&copy, vt, start, common_ancestor);
    if (ret == SUCCESS)
        ret = apply_downwards(&copy, vt, common_ancestor, dest);

    if (ret == SUCCESS) {
        free(*text);
        *text = copy;
    } else
        free(copy);
    return ret;
}

static bool file_exists(const char * path)
{
    return read_parent_from_path(NULL, path) != ERR_NO_SUCH_FILE;
}

static return_t find_new_version(int* new_version, int parent, struct version_tree* vt)
{
    char* fname = string_copy_alloc(vt->base_fname);
    for (int v = parent + 1; v < 10000000; ++v)
    {
        switch_filename_to_version(&fname, v);

        if (!file_exists(fname))
        {
            *new_version = v;
            free(fname);
            return SUCCESS;
        }
    }
    free(fname);
    return ERR_VERSIONS_LIMIT;
}

return_t vt_push(
        int* child, struct version_tree* vt, int parent, struct delta* delta)
{
    int new_version;
    return_t ret = find_new_version(&new_version, parent, vt);

    if (ret == SUCCESS)
        ret = save_deltas(vt, new_version, parent, delta, NULL);

    if (ret == SUCCESS)
    {
        *child = new_version;
        set_parent(vt, *child, parent);
    }

    return ret;
}

return_t vt_delete_version(struct version_tree* vt, int deleted)
{
    struct delta deleted_delta = DELTA_INIT;
    return_t ret = load_delta(&deleted_delta, vt, deleted);
    if (ret != SUCCESS)
        return ret;

    assert(vt_version_is_known(vt, deleted));
    int parent = vt_get_parent(vt, deleted);

    char* fname = string_copy_alloc(vt->base_fname);

    for (int i = 1; ret == SUCCESS && i < (int)vt->capacity; ++i)
        if (vt_get_parent(vt, i) == deleted)
        {
            struct delta child_delta = DELTA_INIT;
            ret = load_delta(&child_delta, vt, i);
            assert(ret == SUCCESS); // TODO handle this case

            switch_filename_to_version(&fname, i);
            ret = save_deltas(vt, i, parent, &deleted_delta, &child_delta);
            assert(ret == SUCCESS);
            set_parent(vt, i, parent);
        }
    set_parent(vt, deleted, -1);
    switch_filename_to_version(&fname, deleted);
    DeleteFile(fname);
    free(fname);
    return SUCCESS;
}

static void traverse_from_root_rec(int** path, size_t* path_len, size_t* path_capacity, struct version_tree* vt, int cur)
{
    assert(vt_version_is_known(vt, cur));
    if (cur == 0)
        return;

    traverse_from_root_rec(path, path_len, path_capacity, vt, vt_get_parent(vt, cur));

    if (*path_len == *path_capacity)
    {
        *path_capacity *= 2;
        checked_realloc((void**)path, *path_capacity);
    }
    (*path)[(*path_len)++] = cur;
}

static void traverse_from_root(int** path, size_t* path_len, struct version_tree* vt, int version)
{
    size_t capacity = 3;
    *path = checked_malloc(3 * sizeof(int));
    *path_len = 0;
    traverse_from_root_rec(path, path_len, &capacity, vt, version);
}

return_t vt_reverse_from_root(struct version_tree* vt, int version)
{
    if (!vt_version_is_known(vt, version))
        return ERR_INVALID_VERSION;

    size_t path_len;
    int* path;
    traverse_from_root(&path, &path_len, vt, version);

    return_t ret = SUCCESS;

    for (size_t i = 0; ret == SUCCESS && i * 2 + 1 < path_len; ++i)
    {
        int a = path[i], b = path[path_len - i - 1];

        struct delta delta_a = DELTA_INIT;
        if (ret == SUCCESS)
            ret = load_delta(&delta_a, vt, a);
        struct delta delta_b = DELTA_INIT;
        if (ret == SUCCESS)
            ret = load_delta(&delta_b, vt, b);

        delta_reverse(&delta_a);
        delta_reverse(&delta_b);

        if (ret == SUCCESS)
            ret = save_deltas(vt, a, vt_get_parent(vt, a), &delta_b, NULL);
        if (ret == SUCCESS)
            ret = save_deltas(vt, b, vt_get_parent(vt, b), &delta_a, NULL);
    }
    return ret;
}
