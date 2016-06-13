#pragma once

// см. http://vld.codeplex.com/
// Если установить библиотеку с сайта и раскомментировать эту строку, по
// окончании отладки выведется отчёт об утечках памяти.
//#include <vld.h>


#include <stdlib.h>

// Аналоги соответствующих функций из stdlib, завершающие программу,
// если память (пере)выделить не удалось: мы вряд ли можем сделать что-то
// осмысленное в этом случае.
void* checked_malloc(size_t size);
void* checked_calloc(size_t n_memb, size_t one_size);
void checked_realloc(void** inout, size_t new_size);
