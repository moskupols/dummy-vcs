#include "version_tree.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include "delta.h"
#include "parse.h"

return_t read_parent(int* out, FILE* f)
{
    assert(f);

    int p;
    if (fscanf(f, "%d\n", &p) <= 0)
        return ERR_READ;
    if (out != NULL)
        *out = p;
    return SUCCESS;
}

return_t print_parent(int parent, FILE* f)
{
    assert(f);

    return 0 < fprintf(f, "%d\n", parent)
        ? SUCCESS
        : ERR_WRITE;
}

int version_for_filename(const char* filename)
{
    const char *p = filename + strlen(filename);
    while (p != filename && isdigit(*(p-1)))
        --p;
    if (*p == '\0')
        return 0;
    return atoi(p);
}

char* filename_for_version(const char* base_fname, int version)
{
    char* ret = string_copy_alloc(base_fname);

    if (version == 0)
        return ret;

    size_t len = strlen(ret);
    char *p = strrchr(ret, '.');
    size_t suffix_pos = p == NULL ? len : (size_t)(p - ret);

    return_t err = string_erase(&ret, suffix_pos, FICTIVE_LEN);
    assert(err == SUCCESS);

    char buf[10];
    sprintf(buf, ".%d", version);

    err = string_insert(&ret, suffix_pos, buf);
    assert(err == SUCCESS);

    return ret;
}

return_t version_tree_load(struct version_tree* out, const char* base_fname)
{
    version_tree_free(out);
    out->base_fname = string_copy_alloc(base_fname);
    return SUCCESS;
}

void version_tree_free(struct version_tree* vt)
{
    free(vt->base_fname);
    *vt = VERSION_TREE_INIT;
}

int version_tree_get_parent(struct version_tree* vt, int v)
{
    if (v <= 0)
        return -1;

    FILE* f = fopen(filename_for_version(vt->base_fname, v), "r");
    if (f == NULL)
        return -1;

    int ret = -1;
    read_parent(&ret, f);
    fclose(f);
    return ret;
}

return_t version_tree_checkout(
        char** out, struct version_tree* vt, int version)
{
    assert(version >= 0);

    if (version == 0)
        return read_file(out, vt->base_fname);

    return_t ret = SUCCESS;

    char* version_filename = filename_for_version(vt->base_fname, version);
    FILE* f = fopen(version_filename, "r");
    free(version_filename);

    if (f == NULL)
        ret = ERR_NO_SUCH_FILE;

    int parent = -1;
    if (ret == SUCCESS)
        ret = read_parent(&parent, f);

    char* text = NULL;
    if (ret == SUCCESS)
        ret = version_tree_checkout(&text, vt, parent);

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

    FILE* new_file;
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
    {
        ret = delta_print(delta, new_file);
        fclose(new_file);
    }

    if (ret == SUCCESS)
        *child = new_version;

    return ret;
}

