#pragma once

#include <stdio.h>

#include "my_string.h"
#include "error.h"

typedef char delta_line_type_t;
#define DELTA_ADD '+'
#define DELTA_ERASE '-'

// Структура delta_line описывает одно изменение текста: вставку или удаление подстроки.
// При удалении запоминается вся подтрока, а не только её длина, чтобы изменение можно было
// проделать в обратную сторону.
struct delta_line
{
    size_t pos;
    char* text;

    delta_line_type_t type;
};
#define DELTA_LINE_INIT { 0, NULL, '?' }
static const struct delta_line delta_line_init = DELTA_LINE_INIT;

struct delta_line delta_line_new( size_t pos, char* text, delta_line_type_t type );

// Структура delta описывает последовательность изменений текста. По сути представляет из
// себя самописный аналог std::vector<struct delta_line> из C++.
struct delta
{
    struct delta_line* lines;
    size_t len;
    size_t capacity;
};
#define DELTA_INIT { NULL, 0, 0 }
static const struct delta delta_init = DELTA_INIT;

// Добавить одно изменение в конец последовательности. Аналог std::vector::push_back.
void delta_append(struct delta* delta, struct delta_line line);

// Построчная загрузка изменений из файла. Номер родительской версии в первой строке не
// ожидается, он должен быть уже пропущен.
return_t delta_load(struct delta* out, FILE* stream);

// Освобождает память и присваивает delta_line_init.
void delta_line_free(struct delta_line* line);
// Освобождает память и присваивает delta_init.
void delta_free(struct delta* delta);

// Вывод одного изменения в таком формате, чтобы потом можно было прочитать через
// delta_load.
return_t delta_line_print(const struct delta_line* line, FILE* stream);

// Последовательно применяет delta_line_print ко всем изменениям.
return_t delta_print(const struct delta* delta, FILE* stream);

// Разворачивает последовательность изменений. При этом порядок изменений 
// меняется на обратный, добавления превращаются в удаления, удаления превращаются в
// добавления.
void delta_reverse(struct delta* delta);

// Применить последовательность изменений к строке.
return_t delta_apply(char** text, const struct delta* delta);
