#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "error.h"

// ����� ������� ��������� �������, ���������� ������ �� ��������.
// ��� ������, ������ ��������� � ������� �������� �� �����.

// ��������� �����, ������������ ��� �������� �������� ���������,
// �������������� �� ����� ������, �� ���� ��������.
// �������������� � check_substr
#define FICTIVE_LEN ((size_t)-1)

// ���������, ��� �� ������ �������� ���������� ����� ��� size ��������
void string_reserve(char** s, size_t size);

// ����������� ������ � ����������� ���������� ������
char* string_copy_alloc(const char* source);
// ����������� ������� ������ � ����������� ���������� ������
char* string_copy_n_alloc(const char* source, size_t n);

// ���������, ��� � dest ���������� ����� � ����������� ���� source
void string_assign_copy(char** dest, const char* source);

// ���������, ��� � ������ ����� old_len ����� �������� ���������, ������������
// � ������� pos � ����� new_len. ���� new_len == FICTIVE_LEN, �������, ��� ���������
// ������ �� ����� ������. ���� new_len_p != NULL, �������� ������ ����� ���������
// �� ����� ������.
bool check_substr(size_t old_len, size_t pos, size_t new_len, size_t* new_len_p);

// �������� ����� ���������
char* string_substr_alloc(const char* string, size_t pos, size_t len);

// �������� ���� ������ ������ ������, ��� ������������� ����������� ������
return_t string_insert(char** into, size_t pos, const char* what);
// ������� ���������
return_t string_erase(char** from, size_t pos, size_t len);
