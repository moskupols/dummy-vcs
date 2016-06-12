#include "version_tree.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <stdio.h>

#include "delta.h"

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

int get_parent(struct vcs_state* vcs, int v)
{
    if (v <= 0)
        return -1;

    FILE* f = fopen(filename_for_version(vcs->base_filename, v), "r");
    if (f == NULL)
        return -1;

    int ret = -1;
    delta_load_parent(&ret, f);
    fclose(f);
    return ret;
}

