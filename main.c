#include <stdio.h>

#include "my_string.h"
#include "parse.h"
#include "delta.h"
#include "vcs.h"

int main()
{
    struct string s = STRING_NULL;
    string_assign_cstr(&s, "main.c");

    struct vcs_state vcs = VCS_NULL;
    vcs_open(&vcs, &s, 0);
    vcs_print(&vcs, stdout);

    string_assign_cstr(&s, "cool!");
    vcs_edit(&vcs, 3, 3, &s);

    vcs_print(&vcs, stdout);

    vcs_free(&vcs);

    return 0;
}

