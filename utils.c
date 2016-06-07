#include "utils.h"

#include <stdlib.h>
#include <errno.h>
#include <assert.h>

return_t _my_realloc(void** inout, size_t new_size)
{
    void* new_ptr = realloc(*inout, new_size);
    if (new_ptr == NULL)
    {
        assert(errno == ENOMEM);
        return ERR_NO_MEMORY;
    }
    *inout = new_ptr;
    return SUCCESS;
}
