#pragma once

#include <stdlib.h>

#include "error.h"

return_t _my_realloc(void** inout, size_t new_size);

#define my_realloc(ptr, size) (_my_realloc((void**)(ptr), (size)))

#define min(a, b) ((a) < (b) ? (a) : (b))
#define max(a, b) ((a) > (b) ? (a) : (b))

