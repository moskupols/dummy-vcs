#include <stdio.h>

#include "my_string.h"
#include "parse.h"
#include "delta.h"

int main()
{
    struct delta d = DELTA_NULL;
    d.parent = 0;

    struct string s;
    string_copy_cstr_alloc(&s, "012345678");

    delta_load(&d, stdin);

    struct string t;
    delta_apply_alloc(&t, &d, &s);
    puts(t.data);

    delta_save(&d, stdout);

    delta_free(&d);

    d.parent = 0;
    delta_calc(&d, &s, &t);
    delta_save(&d, stdout);
    delta_free(&d);

    string_free(&s);
    string_free(&t);

    return 0;
}

