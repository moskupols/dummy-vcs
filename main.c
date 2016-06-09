#include <stdio.h>

#include "my_string.h"
#include "parse.h"
#include "delta.h"
#include "vcs.h"

int main()
{
    struct vcs_state vcs = VCS_NULL;
    vcs_open(&vcs, "Makefile", 0);

    vcs_edit(&vcs, 3, 3, "cool!");

    vcs_print(&vcs, stdout);

    vcs_push(&vcs);

    vcs_free(&vcs);

    return 0;
}

