#include "vcs.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int revision_for_filename(const struct string* filename)
{
    char *p = filename->data + filename->len;
    while (p != filename->data && isdigit(*(p-1)))
        --p;
    if (*p == '\0')
        return 0;
    return atoi(p);
}

return_t filename_for_revision(struct string* filename, int revision)
{
    char *p = filename->data + filename->len;
    while (p != filename->data && *p != '.')
        --p;

    size_t suffix_pos = p - filename->data;
    if (suffix_pos == 0)
        suffix_pos = filename->len;

    return_t ret = string_reserve(filename, suffix_pos + 6);
    if (ret == SUCCESS)
        ret = string_erase(filename, suffix_pos, FICTIVE_LEN);

    if (ret == SUCCESS)
        sprintf(filename->data + suffix_pos, ".%d", revision);

    return ret;
}

