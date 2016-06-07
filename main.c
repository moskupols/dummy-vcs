#include <stdio.h>

#include "my_string.h"
#include "parse.h"
#include "delta.h"

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

    return 0;
}

