#pragma once

#include <stdio.h>

#include "error.h"

// Читать поток, пока он не кончится, либо пока не встретится символ stop_char
// Аналог std::getline(std::istream&, std::string&, int);
return_t read_until(char** out, FILE* stream, int stop_char);

#define read_line(s, f) read_until(s, f, '\n')
#define read_all(s, f) read_until(s, f, EOF)

// Прочесть файл, лежащий по данному пути, в одну строку.
return_t read_file(char** out, const char* path);

