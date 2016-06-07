#include <stdio.h>

#include "my_string.h"
#include "parse.h"
#include "delta.h"
#include "vcs.h"

int main()
{
    struct delta d = DELTA_NULL;
    d.parent = 0;

    struct string s = STRING_NULL;
    string_copy_cstr_alloc(&s, "012345678");

    delta_load(&d, stdin);

    struct string t = STRING_NULL;
    if (delta_apply_alloc(&t, &d, &s) == SUCCESS)
        puts(t.data);

    delta_save(&d, stdout);

    if (!string_is_null(&t))
    {
        d.parent = 0;
        delta_calc(&d, &s, &t);
        delta_save(&d, stdout);
    }

    delta_free(&d);
    string_free(&s);
    string_free(&t);


    string_copy_cstr_alloc(&s, "a.5");
    printf("%d\n", revision_for_filename(&s));

    filename_for_revision(&s, 8);
    printf("%s\n", s.data);

    string_assign_cstr(&s, "main.c");

    struct vcs_state vcs = VCS_NULL;
    vcs_open(&vcs, &s, 0);
    printf("%s", vcs.working_state.data);
    vcs_free(&vcs);

    return 0;
}

