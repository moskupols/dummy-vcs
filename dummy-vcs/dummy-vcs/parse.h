#pragma once

#include <stdio.h>

#include "error.h"

// ������ �����, ���� �� �� ��������, ���� ���� �� ���������� ������ stop_char
// ������ std::getline(std::istream&, std::string&, int)
return_t read_until(char** out, FILE* stream, int stop_char);

#define read_line(s, f) read_until(s, f, '\n')
#define read_all(s, f) read_until(s, f, EOF)

// �������� ����, ������� �� ������� ����, � ���� ������.
return_t read_file(char** out, const char* path);

