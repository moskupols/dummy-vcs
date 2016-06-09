#include "utils.h"

#include <stdlib.h>
#include <errno.h>
#include <assert.h>

static void* check_alloc_result(void* ptr)
{
    if (ptr == NULL)
    {
        assert(errno == ENOMEM);
        exit(ENOMEM);
    }
    return ptr;
}

void checked_realloc(void** inout, size_t new_size)
{
    *inout = check_alloc_result(realloc(*inout, new_size));
}

void* checked_malloc(size_t size)
{
    return check_alloc_result(malloc(size));
}

void* checked_calloc(size_t n_memb, size_t size)
{
    return check_alloc_result(calloc(n_memb, size));
}

