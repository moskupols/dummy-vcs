#include <stdio.h>

#include "my_string.h"
#include "parse.h"
#include "delta.h"
#include "vcs.h"

int main()
{
    struct string s = string_from_cstr("main.c");

    struct vcs_state vcs = VCS_NULL;
    vcs_open(&vcs, &s, 0);

    s = string_from_cstr("cool!");
    vcs_edit(&vcs, 3, 3, &s);

    vcs_print(&vcs, stdout);

    vcs_push(&vcs);

    vcs_free(&vcs);

    return 0;
}

