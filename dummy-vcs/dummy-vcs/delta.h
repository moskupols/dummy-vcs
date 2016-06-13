#pragma once

#include <stdio.h>

#include "my_string.h"
#include "error.h"

typedef char delta_line_type_t;
#define DELTA_ADD '+'
#define DELTA_ERASE '-'

// ��������� delta_line ��������� ���� ��������� ������: ������� ��� �������� ���������.
// ��� �������� ������������ ��� ��������, � �� ������ � �����, ����� ��������� ����� ����
// ��������� � �������� �������.
struct delta_line
{
    size_t pos; // ������� ������ ������� ��� ��������
    char* text;

    delta_line_type_t type; // ������ ���� ���� '+', ���� '-'.
};
// ���� ������ ��� ������������� ����� ����������. � type ��������������� '?', ����� ������ ����
// ������ ���������� ��� �� ���������� (����� ��������� ����� �� assert ��� ����������)
#define DELTA_LINE_INIT { 0, NULL, '?' }
// ��������� ��� ���������� �������� �� ��������� ������
static const struct delta_line delta_line_init = DELTA_LINE_INIT;

// ������ ��������� ��� �������� � ��������� delta_line, ����� ������ �� ����������
struct delta_line delta_line_new( size_t pos, char* text, delta_line_type_t type );

// ��������� delta ��������� ������������������ ��������� ������. �� ���� ������������ ��
// ���� ���������� ������ std::vector<struct delta_line> �� C++.
struct delta
{
    struct delta_line* lines;
    size_t len;      // ���������� ����������� ���������
    size_t capacity; // �� ������� ��������� �������� ������
};
// ��� ������������� ����� ���������� ���� struct delta
#define DELTA_INIT { NULL, 0, 0 }
// ��������� ��� ���������� �������� �� ��������� ������
static const struct delta delta_init = DELTA_INIT;

// �������� ���� ��������� � ����� ������������������. ������ std::vector::push_back.
void delta_append(struct delta* delta, struct delta_line line);

// ���������� �������� ��������� �� �����. ����� ������������ ������ � ������ ������ ��
// ���������, �� ������ ���� ��� ��������.
return_t delta_load(struct delta* out, FILE* stream);

// ����������� ������ � ����������� delta_line_init.
void delta_line_free(struct delta_line* line);
// ����������� ������ � ����������� delta_init.
void delta_free(struct delta* delta);

// ����� ������ ��������� � ����� �������, ����� ����� ����� ���� ��������� �����
// delta_load.
return_t delta_line_print(const struct delta_line* line, FILE* stream);

// ��������������� ��������� delta_line_print �� ���� ����������.
return_t delta_print(const struct delta* delta, FILE* stream);

// ������������� ������������������ ���������. ��� ���� ������� ��������� 
// �������� �� ��������, ���������� ������������ � ��������, �������� ������������ �
// ����������.
void delta_reverse(struct delta* delta);

// ��������� ������������������ ��������� � ������.
return_t delta_apply(char** text, const struct delta* delta);
