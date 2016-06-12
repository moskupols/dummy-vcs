#pragma once

typedef enum
{
    SUCCESS,
    ERR_INVALID_RANGE,
    ERR_INVALID_DELTA,
    ERR_INVALID_VERSION,
    ERR_NO_SUCH_FILE,
    ERR_READ,
    ERR_WRITE,
    ERR_VERSIONS_LIMIT
} return_t;

const char* error_description(return_t r);
