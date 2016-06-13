#pragma once

#include <stdio.h>

#include "my_string.h"
#include "error.h"

typedef char delta_line_type_t;
#define DELTA_ADD '+'
#define DELTA_ERASE '-'

// —труктура delta_line описывает одно изменение текста: вставку или удаление подстроки.
// ѕри удалении запоминаетс€ вс€ подтрока, а не только еЄ длина, чтобы изменение можно было
// проделать в обратную сторону.
struct delta_line
{
    size_t pos; // ѕозици€ начала вставки или удалени€
    char* text;

    delta_line_type_t type; // ќб€зан быть либо '+', либо '-'.
};
// Ётот макрос дл€ инициализации новой переменной. ¬ type устанавливаетс€ '?', чтобы нельз€ было
// забыть установить тип до сохранени€ (иначе программа упадЄт по assert при сохранении)
#define DELTA_LINE_INIT { 0, NULL, '?' }
//  онстанта дл€ присвоени€ значени€ по умолчанию заново
static const struct delta_line delta_line_init = DELTA_LINE_INIT;

// ѕросто упаковать три значени€ в структуру delta_line, нова€ строка не выдел€етс€
struct delta_line delta_line_new( size_t pos, char* text, delta_line_type_t type );

// —труктура delta описывает последовательность изменений текста. ѕо сути представл€ет из
// себ€ самописный аналог std::vector<struct delta_line> из C++.
struct delta
{
    struct delta_line* lines;
    size_t len;      //  оличество добавленных изменений
    size_t capacity; // Ќа сколько изменений выделено пам€ти
};
// ƒл€ инициализации новой переменной типа struct delta
#define DELTA_INIT { NULL, 0, 0 }
//  онстанта дл€ присвоени€ значени€ по умолчанию заново
static const struct delta delta_init = DELTA_INIT;

// ƒобавить одно изменение в конец последовательности. јналог std::vector::push_back.
void delta_append(struct delta* delta, struct delta_line line);

// ѕострочна€ загрузка изменений из файла. Ќомер родительской версии в первой строке не
// ожидаетс€, он должен быть уже пропущен.
return_t delta_load(struct delta* out, FILE* stream);

// ќсвобождает пам€ть и присваивает delta_line_init.
void delta_line_free(struct delta_line* line);
// ќсвобождает пам€ть и присваивает delta_init.
void delta_free(struct delta* delta);

// ¬ывод одного изменени€ в таком формате, чтобы потом можно было прочитать через
// delta_load.
return_t delta_line_print(const struct delta_line* line, FILE* stream);

// ѕоследовательно примен€ет delta_line_print ко всем изменени€м.
return_t delta_print(const struct delta* delta, FILE* stream);

// –азворачивает последовательность изменений. ѕри этом пор€док изменений 
// мен€етс€ на обратный, добавлени€ превращаютс€ в удалени€, удалени€ превращаютс€ в
// добавлени€.
void delta_reverse(struct delta* delta);

// ѕрименить последовательность изменений к строке.
return_t delta_apply(char** text, const struct delta* delta);
