#pragma once

#include <stdio.h>

#include "error.h"
#include "my_string.h"

return_t read_until(struct string* out, FILE* stream, int stop_char);

#define read_line(s, f) read_until(s, f, '\n')
#define read_all(s, f) read_until(s, f, EOF)

