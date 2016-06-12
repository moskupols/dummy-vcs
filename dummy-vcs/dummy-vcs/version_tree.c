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

static return_t read_parent_file(int* out, const char* fname)
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

static char* filename_for_version(const char* base_fname, int version)
{
    char* ret = string_copy_alloc(base_fname);

    if (version == 0)
        return ret;

    char buf[10];
    sprintf(buf, ".%d", version);
    replace_extension(&ret, buf);

    return ret;
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

return_t version_tree_load(struct version_tree* vt, const char* base_fname)
{
    version_tree_free(vt);

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
            if (read_parent_file(&parent, file_data.cFileName) == SUCCESS)
                set_parent(vt, child, parent);
        } while (FindNextFile(dir_handle, &file_data));
        FindClose(dir_handle);
    }

    string_assign_copy(&fname, base_fname);
    vt->base_fname = fname;

    return SUCCESS;
}

void version_tree_free(struct version_tree* vt)
{
    free(vt->base_fname);
    free(vt->parent);
    *vt = version_tree_init;
}

int version_tree_get_parent(struct version_tree* vt, int child)
{
    if (child <= 0 || (size_t)child >= vt->capacity)
        return -1;
    return vt->parent[child];
}

return_t version_tree_checkout(
        char** out, struct version_tree* vt, int version)
{
    assert(version >= 0);

    if (version == 0)
        return read_file(out, vt->base_fname);

    int parent = version_tree_get_parent(vt, version);
    if (parent == -1)
        return ERR_INVALID_VERSION;

    char* text = NULL;
    return_t ret = version_tree_checkout(&text, vt, parent);

    char* version_filename = filename_for_version(vt->base_fname, version);
    FILE* f = fopen(version_filename, "r");
    free(version_filename);

    if (f == NULL)
        ret = ERR_NO_SUCH_FILE;

    if (ret == SUCCESS)
        ret = read_parent(NULL, f);
        
    struct delta delta = DELTA_INIT;
    if (ret == SUCCESS)
        ret = delta_load(&delta, f);
        
    if (ret == SUCCESS)
        ret = delta_apply(&text, &delta);

    if (ret == SUCCESS)
        *out = text;
    else
        free(text);

    delta_free(&delta);
    if (f)
        fclose(f);

    return ret;
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
        int parent, struct version_tree* vt)
{
    for (int v = parent + 1; v < 10000000; ++v)
    {
        *new_filename = filename_for_version(vt->base_fname, v);

        if (!file_exists(*new_filename))
        {
            *new_version = v;
            return SUCCESS;
        }
        free(*new_filename);
    }
    return ERR_VERSIONS_LIMIT;
}

return_t version_tree_push(
        int* child, struct version_tree* vt, int parent, struct delta* delta)
{
    int new_version;
    char* new_filename = NULL;
    return_t ret = find_new_version(&new_version, &new_filename, parent, vt);

    FILE* new_file = NULL;
    if (ret == SUCCESS)
    {
        new_file = fopen(new_filename, "w");
        free(new_filename);
        if (new_file == NULL)
            ret = ERR_WRITE;
    }

    if (ret == SUCCESS)
        ret = print_parent(parent, new_file);
        
    if (ret == SUCCESS)
        delta_print(delta, new_file);

    if (new_file != NULL)
        fclose(new_file);

    if (ret == SUCCESS)
    {
        *child = new_version;
        set_parent(vt, *child, parent);
    }

    return ret;
}
