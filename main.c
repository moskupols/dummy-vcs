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

    delta_apply(&s, &d);
    puts(s.data);

    delta_save(&d, stdout);

    delta_free(&d);

    string_free(&s);

    return 0;
}

