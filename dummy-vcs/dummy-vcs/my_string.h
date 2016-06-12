#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "error.h"

// Здесь описаны некоторые функции, упрощающие работу со строками.
// Как обычно, строки ожидаются с нулевым символом на конце.

#define FICTIVE_LEN ((size_t)-1)

// Структура для описания подстроки.
struct substr
{
    const char* str;
    size_t pos;
    size_t len;
};

// Убедиться, что на строку выделено достаточно места для size символов
void string_reserve(char** s, size_t size);

// Скопировать строку в динамически выделенную память
char* string_copy_alloc(const char* source);
// Скопировать префикс строки в динамически выделенную память
char* string_copy_n_alloc(const char* source, size_t n);

// Убедиться, что в dest достаточно места и скопировать туда source
void string_assign_copy(char** dest, const char* source);

// Проверить, что в строке с длины old_len можно выделить подстроку, начинающуюся
// в позиции pos и длины new_len. Если new_len == FICTIVE_LEN, считать, что подстрока
// берётся до конца строки. Если new_len_p != NULL, записать точную длину подстроки
// по этому адресу.
bool check_substr(size_t old_len, size_t pos, size_t new_len, size_t* new_len_p);

// Сконструировать подстроку
struct substr string_substr(const char* string, size_t pos, size_t len);

// Вставить одну строку внутрь другой, при необходимости перевыделив память
return_t string_insert(char** into, size_t pos, const char* what);
// Удалить подстроку
return_t string_erase(char** from, size_t pos, size_t len);

// Выделить память и скопировать туда содержимое подстроки
char* substr_to_string_alloc(const struct substr* sub);

