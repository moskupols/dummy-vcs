#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "vcs.h"

void sample_test()
{
    const char* init_text = "ABCDEFGH1234567890";

    system("del a.*");

    FILE* f = fopen("a.txt", "w");
    fputs(init_text, f);
    fclose(f);

    f = fopen("a.txt", "r");

    struct vcs_state vcs = VCS_INIT;

    assert(vcs_open(&vcs, "a.txt", 0) == SUCCESS);
    fclose(f);
    assert(strcmp(vcs.working_state, init_text) == 0);

    assert(vcs_edit(&vcs, 2, 3, "O") == SUCCESS);

    assert(vcs_remove(&vcs, 1, 10) == SUCCESS);
    assert(strcmp(vcs.working_state, "A34567890") == 0);

    assert(vcs_add(&vcs, 19, "aksjhda") == ERR_INVALID_RANGE);
    assert(vcs_add(&vcs, 10, "uhsdjs") == ERR_INVALID_RANGE);

    assert(vcs_add(&vcs, 9, "XYZ") == SUCCESS);
    assert(strcmp(vcs.working_state, "A34567890XYZ") == 0);

    assert(vcs_push(&vcs) == SUCCESS);
    assert(vcs.version == 1);

    assert(vcs_edit(&vcs, 0, 2, "IBKS") == SUCCESS);
    assert(strcmp(vcs.working_state, "IBKS4567890XYZ") == 0);

    assert(vcs_pull(&vcs, 2) == ERR_INVALID_VERSION);
    assert(vcs_push(&vcs) == SUCCESS);
    assert(vcs.version == 2);

    assert(vcs_pull(&vcs, 1) == SUCCESS);
    assert(strcmp(vcs.working_state, "A34567890XYZ") == 0);

    assert(vcs_add(&vcs, 12, "ZERO") == SUCCESS);
    assert(strcmp(vcs.working_state, "A34567890XYZZERO") == 0);
    assert(vcs_push(&vcs) == SUCCESS);
    assert(vcs.version == 3);

    assert(vcs_pull(&vcs, 2) == SUCCESS);
    assert(strcmp(vcs.working_state, "IBKS4567890XYZ") == 0);

    assert(vcs_delete_version(&vcs, 1) == SUCCESS);

    vcs_free(&vcs);
}

int main()
{
    sample_test();

    return 0;
}

