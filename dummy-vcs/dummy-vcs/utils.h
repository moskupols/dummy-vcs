#pragma once

#include <stdlib.h>

// Аналоги соответствующих функций из stdlib, завершающие программу,
// если память (пере)выделить не удалось: мы вряд ли можем сделать что-то
// осмысленное в этом случае.
void* checked_malloc(size_t size);
void* checked_calloc(size_t n_memb, size_t one_size);
void checked_realloc(void** inout, size_t new_size);
