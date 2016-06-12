#include "error.h"

const char* error_description(return_t r)
{
    switch (r) {
        case SUCCESS: return "SUCCESS!";
        case ERR_INVALID_RANGE: return "Invalid edit bounds";
        case ERR_INVALID_DELTA: return "Invalid delta";
        case ERR_INVALID_VERSION: return "Invalid version";
        case ERR_NO_SUCH_FILE: return "Couldn't open file";
        case ERR_READ: return "Error while reading file";
        case ERR_WRITE: return "Error while writing changes";
        case ERR_VERSIONS_LIMIT: return "Versions limit exceeded";
        default: return "Error";
    }
}

