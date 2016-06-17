// 4_laba.cpp: определяет точку входа для консольного приложения.
//

// #include "stdafx.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <ctype.h>
#include <errno.h>

/*----------------------------------------------------------------ERRORS------------------------------------------------*/
typedef enum //ошибки
{
    // Нет ошибки
    SUCCESS,

    // Попытка обратиться за границу текста
    ERR_INVALID_RANGE,

    // Ошибка в сохранённых изменениях
    ERR_INVALID_DELTA,

    // Операция невозможно из-за неподходящего номера версии
    ERR_INVALID_VERSION,

    // Не удалось открыть файл
    ERR_NO_SUCH_FILE,

    // Файл удалось открыть, но произошла ошибка при чтении:
    // например, он неожиданно оборвался
    ERR_READ,

    // Не получилось записать в файл
    ERR_WRITE,

    // Не удалось найти свободный номер версии
    ERR_VERSIONS_LIMIT,

    // Не открыто никакого файла
    ERR_NOT_OPEN,

    // Попытка удалить текущую версию
    ERR_DELETE_CURRENT
} return_t;

const char* error_description(return_t r)
{
    switch (r) {
        case SUCCESS: return "SUCCESS!";
        case ERR_INVALID_RANGE: return "Invalid edit bounds";
        case ERR_INVALID_DELTA: return "Invalid delta";
        case ERR_INVALID_VERSION: return "Invalid version";
        case ERR_NO_SUCH_FILE: return "Couldn't open file";
        case ERR_READ: return "Error while reading file";
        case ERR_WRITE: return "Error while writing changes";
        case ERR_VERSIONS_LIMIT: return "Versions limit exceeded";
        case ERR_NOT_OPEN: return "No file is open at the moment. Try `open a.txt'.";
        case ERR_DELETE_CURRENT: return "Couldn't delete current version. Use `pull 0' to switch to root version.";
        default: return "Error";
    }
}

/*----------------------------------------------------------------MEMORY CHECK------------------------------------------------*/
// Аналоги соответствующих функций из stdlib, завершающие программу,
// если память (пере)выделить не удалось: мы вряд ли можем сделать что-то
// осмысленное в этом случае.
void* check_alloc_result(void* ptr)
{
    if (ptr == NULL) {
        assert(errno == ENOMEM);
        exit(ENOMEM);
    }
    return ptr;
}

void checked_realloc(void** inout, size_t new_size)
{
    *inout = check_alloc_result(realloc(*inout, new_size));
}

void* checked_malloc(size_t size)
{
    return check_alloc_result(malloc(size));
}

void* checked_calloc(size_t n_memb, size_t size)
{
    return check_alloc_result(calloc(n_memb, size));
}


/*----------------------------------------------------------------STRINGS------------------------------------------------*/
// Здесь описаны некоторые функции, упрощающие работу со строками.
// Как обычно, строки ожидаются с нулевым символом на конце.

// Фиктивная длина, используется для удобного описания подстроки,
// продолжающейся до конца строки, то есть суффикса.
// Обрабатывается в check_substr
#define FICTIVE_LEN ((size_t)-1)

// Убедиться, что на строку выделено достаточно места для size символов
void string_reserve(char** s, size_t new_len)
{
    assert(*s != NULL);

    size_t old_len = strlen(*s);
    if (old_len >= new_len) // строка и так не короче
        return;

    checked_realloc((void**)s, new_len + 1);
    memset(*s + old_len, 0, new_len - old_len + 1); // установим нули везде в новой памяти
}

// Скопировать префикс строки в динамически выделенную память
char* string_copy_n_alloc(const char* source, size_t n)
{
    assert(source != NULL);

    char* ret = (char*)checked_malloc(n + 1);
    strncpy(ret, source, n);
    ret[n] = '\0';
    return ret;
}

// Скопировать строку в динамически выделенную память
char* string_copy_alloc(const char* source)
{
    return string_copy_n_alloc(source, strlen(source));
}

// Убедиться, что в dest достаточно места и скопировать туда source
void string_assign_copy(char** dest, const char* source)
{
    string_reserve(dest, strlen(source));
    strcpy(*dest, source);
}

// Проверить, что в строке длины mylen можно выделить подстроку, начинающуюся
// в позиции pos и длины new_len. Если new_len == FICTIVE_LEN, считать, что подстрока
// берётся до конца строки. Если new_len_p != NULL, записать точную длину подстроки
// по этому адресу.
bool check_substr(size_t mylen, size_t pos, size_t new_len, size_t* new_len_p)
{
    if (pos > mylen)
        return false;

    if (new_len == FICTIVE_LEN)
        new_len = mylen - pos;

    if (pos + new_len > mylen)
        return false;

    if (new_len_p != NULL)
        *new_len_p = new_len;

    return true;
}

// Выделить копию подстроки
char* string_substr_alloc(const char* str, size_t pos, size_t len)
{
    assert(check_substr(strlen(str), pos, len, &len));
    return string_copy_n_alloc(str + pos, len);
}

// Вставить одну строку внутрь другой, при необходимости перевыделив память
return_t string_insert(char** into, size_t pos, const char* what)
{
    size_t old_len = strlen(*into);
    size_t add_len = strlen(what);

    if (pos > old_len)
        return ERR_INVALID_RANGE;

    string_reserve(into, old_len + add_len); // убедимся, что памяти достаточно

    size_t suffix_len = old_len - pos;
    char* suffix = *into + pos;

    memmove(suffix + add_len, suffix, suffix_len); // сдвинем суффикс вправо
    memcpy(suffix, what, add_len); // скопируем what туда, где раньше начинался суффикс
    (*into)[old_len + add_len] = '\0';

    return SUCCESS;
}

// Удалить подстроку
return_t string_erase(char** from, size_t pos, size_t len)
{
    size_t old_len = strlen(*from);
    if (!check_substr(old_len, pos, len, &len))
        return ERR_INVALID_RANGE;

    char* erased = *from + pos;
    size_t suffix_len = old_len - pos - len;

    memmove(erased, erased + len, suffix_len); // Сдвинем суффикс туда, где раньше была удалённая подстрока
    (*from)[old_len - len] = '\0';

    return SUCCESS;
}


/*----------------------------------------------------------------PARSE------------------------------------------------*/

// Читать поток, пока он не кончится, либо пока не встретится символ stop_char
// Аналог std::getline(std::istream&, std::string&, int)
return_t read_until(char** out, FILE* stream, int stop_char)
{
    assert(out != NULL);

    size_t used = 0; // Сколько символов прочитано и записано
    size_t capacity = 8; // Сколько выделено памяти

    char* buf = (char*)checked_malloc(capacity);

    char c;
    while ((c = fgetc(stream)) != stop_char && !feof(stream) && !ferror(stream)) {
        buf[used++] = c;
        if (used >= capacity) // Если записано столько символов, сколько выделено памяти, то уже надо
            // выделить ещё, потому что мы собираемся ещё дописать '\0'
        {
            capacity *= 2;
            checked_realloc((void**)&buf, capacity);
        }
    }

    buf[used] = '\0';
    *out = buf;
    return SUCCESS;
}

#define read_line(s, f) read_until(s, f, '\n')
#define read_all(s, f) read_until(s, f, EOF)

// Прочесть файл, лежащий по данному пути, в одну строку.
return_t read_file(char** out, const char* path)
{
    FILE* f = fopen(path, "r");
    if (f == NULL)
        return ERR_NO_SUCH_FILE;
    return_t ret = read_all(out, f);
    fclose(f);
    return ret;
}


/*----------------------------------------------------------------DELTA------------------------------------------------*/
typedef char delta_line_type_t;
#define DELTA_ADD '+'
#define DELTA_ERASE '-'

// Структура delta_line описывает одно изменение текста: вставку или удаление подстроки.
// При удалении запоминается вся подтрока, а не только её длина, чтобы изменение можно было
// проделать в обратную сторону.
struct delta_line {
    size_t pos; // Позиция начала вставки или удаления
    char* text;

    delta_line_type_t type; // Обязан быть либо '+', либо '-'.
};

// Этот макрос для инициализации новой переменной. В type устанавливается '?', чтобы нельзя было
// забыть установить тип до сохранения (иначе программа упадёт по assert при сохранении)
#define DELTA_LINE_INIT { 0, NULL, '?' }

// Константа для присвоения значения по умолчанию заново
const struct delta_line delta_line_init = DELTA_LINE_INIT;

// Структура delta описывает последовательность изменений текста. По сути представляет из
// себя самописный аналог std::vector<struct delta_line> из C++.
struct delta {
    struct delta_line* lines;
    size_t len;      // Количество добавленных изменений
    size_t capacity; // На сколько изменений выделено памяти
};

// Для инициализации новой переменной типа struct delta
#define DELTA_INIT { NULL, 0, 0 }

// Константа для присвоения значения по умолчанию заново
const struct delta delta_init = DELTA_INIT;

// Просто упаковать три значения в структуру delta_line, новая строка не выделяется
struct delta_line delta_line_new(size_t pos, char* text, delta_line_type_t type)
{
    struct delta_line ret;
    ret.pos = pos;
    ret.text = text;
    ret.type = type;
    return ret;
}

// Освобождает память и присваивает delta_line_init.
void delta_line_free(struct delta_line* line)
{
    free(line->text);
    *line = delta_line_init;
}

// Освобождает память и присваивает delta_init.
void delta_free(struct delta* delta)
{
    for (size_t i = 0; i < delta->len; ++i)
        delta_line_free(delta->lines + i);
    free(delta->lines);
    *delta = delta_init;
}

// Добавить одно изменение в конец последовательности. Аналог std::vector::push_back.
// Это единственное место, где перевыделяется память под изменения.
void delta_append(struct delta* delta, struct delta_line line)
{
    if (delta->len == delta->capacity) // Занята вся выделенная память
    {
        if (delta->capacity == 0) // Это будет первое изменение
            delta->capacity = 4;  // выделим сначала немного памяти
        else
            delta->capacity *= 2; // Это уже не первое перевыделение, увеличиваем экспоненциально
        checked_realloc(
            (void**)&delta->lines,
            delta->capacity * sizeof(struct delta_line));
    }
    delta->lines[delta->len++] = line;
}

return_t delta_line_load(struct delta_line* out, FILE* stream)
{
    char type;
    if (fscanf(stream, "%c ", &type) <= 0) // Читаем тип изменения
        return ERR_READ;

    if (type != DELTA_ADD && type != DELTA_ERASE)
        return ERR_INVALID_DELTA;

    size_t pos;
    if (fscanf(stream, "%Iu", &pos) <= 0) // Читаем позицию начала изменения
        return ERR_INVALID_DELTA;

    out->type = type;
    out->pos = pos;
    fgetc(stream); // Пропуск пробела
    return
        SUCCESS == read_line(&out->text, stream) // Читаем текст изменения до конца строки
        ? SUCCESS
        : ERR_INVALID_DELTA;
}

// Построчная загрузка изменений из файла. Номер родительской версии в первой строке не
// ожидается, он должен быть уже пропущен.
return_t delta_load(struct delta* out, FILE* stream)
{
    assert(out != NULL);
    assert(stream != NULL);

    struct delta new_delta = DELTA_INIT;
    struct delta_line next_line = DELTA_LINE_INIT;

    return_t ret;
    // Читаем изменения построчно, пока получается
    while ((ret = delta_line_load(&next_line, stream)) == SUCCESS)
        delta_append(&new_delta, next_line);

    // Если не прочитался даже тип, но просто потому что файл закончился, то так и должно быть
    if (ret == ERR_READ && feof(stream))
        ret = SUCCESS;

    if (ret == SUCCESS) // Всё хорошо, подменяем дельту по указанному адресу
    {
        delta_free(out);
        *out = new_delta;
    } else // иначе освобождаем то, что успели прочитать, по указанному адресу не трогаем
        delta_free(&new_delta);

    return ret;
}

// Вывод одного изменения в таком формате, чтобы потом можно было прочитать через delta_load.
return_t delta_line_print(const struct delta_line* line, FILE* stream)
{
    assert(line->type == DELTA_ADD || line->type == DELTA_ERASE);
    return
        0 < fprintf_s(stream, "%c %Iu %s\n", line->type, line->pos, line->text)
        ? SUCCESS
        : ERR_WRITE;
}

// Последовательно применяет delta_line_print ко всем изменениям.
return_t delta_print(const struct delta* delta, FILE* stream)
{
    return_t ret = SUCCESS;
    for (size_t i = 0; ret == SUCCESS && i < delta->len; ++i)
        ret = delta_line_print(delta->lines + i, stream);
    return ret;
}

// Разворачивает последовательность изменений. При этом порядок изменений 
// меняется на обратный, добавления превращаются в удаления, удаления превращаются в
// добавления.
void delta_reverse(struct delta* delta)
{
    for (size_t i = 0; i < delta->len; ++i) // Заменяем типы на обратные
    {
        struct delta_line* line = delta->lines + i;
        line->type = (line->type == DELTA_ADD ? DELTA_ERASE : DELTA_ADD);
    }
    for (size_t i = 0; i * 2 + 1 < delta->len; ++i) // Разворачиваем порядок изменений
    {
        struct delta_line temp = delta->lines[i];
        delta->lines[i] = delta->lines[delta->len - i - 1];
        delta->lines[delta->len - i - 1] = temp;
    }
}

// Применить последовательность изменений к строке.
return_t delta_apply(char** text, const struct delta *delta)
{
    assert(text != NULL);

    return_t ret = SUCCESS;

    for (size_t i = 0; ret == SUCCESS && i < delta->len; ++i) {
        struct delta_line* line = delta->lines + i;

        assert(line->type == DELTA_ADD || line->type == DELTA_ERASE);
        if (line->type == DELTA_ADD)
            ret = string_insert(text, line->pos, line->text);
        else
            ret = string_erase(text, line->pos, strlen(line->text));
        assert((*text)[strlen(*text)] == '\0');
    }

    return ret;
}

/*----------------------------------------------------------------TREE------------------------------------------------*/
// Структура version_tree описывает собственно дерево версий файла.
struct version_tree {
    // Путь к нулевой версии файла
    char* base_fname;

    // Массив предков: parent[a] содержит родительскую версию версии a,
    // либо -1, если версия a отсутствует или a=0.
    // Вкупе с capacity выделяется аналогично std::vector<int>
    int* parent;
    size_t capacity;
};
#define VERSION_TREE_INIT { NULL, NULL, 0 }
const struct version_tree version_tree_init = VERSION_TREE_INIT;

// Прочитать номер родительской версии из открытого файла
return_t read_parent(int* out, FILE* f)
{
    assert(f);

    int p;
    if (fscanf(f, "%d\n", &p) <= 0)
        return ERR_READ;
    if (out != NULL)
        *out = p;
    return SUCCESS;
}

// Прочитать номер родительской версии по данному пути
return_t read_parent_from_path(int* out, const char* fname)
{
    assert(fname);

    FILE* f = fopen(fname, "r");
    if (f == NULL)
        return ERR_NO_SUCH_FILE;

    return_t ret = read_parent(out, f);
    fclose(f);
    return ret;
}

// Вывести номер родительской версии в открытый файл
return_t print_parent(int parent, FILE* f)
{
    assert(f);

    return 0 < fprintf(f, "%d\n", parent)
        ? SUCCESS
        : ERR_WRITE;
}

// Найти в названии файла место (точку), с которого начинается расширение.
// Если расширения нет, вернуть конец строки.
const char* find_extension(const char* fname)
{
    const char* p = strrchr(fname, '.');
    return p != NULL ? p : fname + strlen(fname);
}

// Заменить расширение в названии файла (либо добавить, если его нет)
void replace_extension(char** fname, const char* new_extension)
{
    size_t suffix_pos = find_extension(*fname) - *fname;

    return_t err = string_erase(fname, suffix_pos, FICTIVE_LEN);
    assert(err == SUCCESS);

    err = string_insert(fname, suffix_pos, new_extension);
    assert(err == SUCCESS);
}

// Проверить, что filename может быть именем файла, описывающего версию файла base_fname.
// Если может, вернуть номер версии, иначе вернуть -1.
int version_for_filename(const char* filename, const char* base_fname)
{
    assert(filename != NULL);
    assert(base_fname != NULL);

    const char* p = find_extension(filename);
    const char* q = find_extension(base_fname);
    if (p - filename != q - base_fname || strncmp(filename, base_fname, p - filename) != 0)
        return -1;
    if (strcmp(p, q) == 0)
        return 0;

    int version = 0;
    for (const char* i = p + 1; *i; ++i) {
        if (!isdigit(*i))
            return -1;
        version = version * 10 + (*i - '0');
    }

    return version ? version : -1;
}

// Сменить версию при известном названии файла старой версии
void switch_filename_to_version(char** fname, int version)
{
    if (version == 0)
        return;

    char buf[10];
    sprintf(buf, ".%d", version);
    replace_extension(fname, buf);
}

// Установить родителя в дереве, при необходимости перевыделив массив родителей
void set_parent(struct version_tree* vt, int child, int parent)
{
    assert(child >= 0);

    if (vt->capacity <= (size_t)child) {
        size_t old_capacity = vt->capacity;

        vt->capacity = max(vt->capacity, 3);
        while (vt->capacity <= (size_t)child)
            vt->capacity *= 2;

        checked_realloc((void**)&vt->parent, vt->capacity * sizeof(int));
        memset(vt->parent + old_capacity, -1, (vt->capacity - old_capacity) * sizeof(int));
    }
    vt->parent[child] = parent;
}

// Освободить и очистить структуру.
void vt_free(struct version_tree* vt)
{
    free(vt->base_fname);
    free(vt->parent);
    *vt = version_tree_init;
}

// Загрузить дерево версий заново. Вызывается при open.
return_t vt_load(struct version_tree* vt, const char* base_fname)
{
    vt_free(vt);

    char* fname = string_copy_alloc(base_fname);
    replace_extension(&fname, ".*");

    WIN32_FIND_DATAA file_data;
    HANDLE dir_handle = FindFirstFileA(fname, &file_data);

    if (dir_handle != NULL) {
        do {
            int child = version_for_filename(file_data.cFileName, base_fname);
            if (child <= 0)
                continue;

            int parent = -1;
            if (read_parent_from_path(&parent, file_data.cFileName) == SUCCESS)
                set_parent(vt, child, parent);
        } while (FindNextFileA(dir_handle, &file_data));
        FindClose(dir_handle);
    }

    string_assign_copy(&fname, base_fname);
    vt->base_fname = fname;

    return SUCCESS;
}

// Получить номер родительской версии или -1, если её нет.
int vt_get_parent(struct version_tree* vt, int child)
{
    if (child <= 0 || (size_t)child >= vt->capacity)
        return -1;
    return vt->parent[child];
}

// Предикат наличия в дереве конкретной версии
bool vt_version_is_known(struct version_tree* vt, int version)
{
    return version == 0 || vt_get_parent(vt, version) > -1;
}

// Нахождение ближайшего общего предка двух вершин, нужно для pull
int vt_find_common_ancestor(struct version_tree* vt, int a, int b)
{
    assert(vt_version_is_known(vt, a));
    assert(vt_version_is_known(vt, b));

    if (a == 0 || b == 0)
        return 0;

    for (int i = a; i >= 0; i = vt_get_parent(vt, i))
        for (int j = b; j >= 0; j = vt_get_parent(vt, j))
            if (i == j)
                return i;
    return -1;
}

// Сохранить одну или две дельты (больше не бывает надо) в файл, соответствующий версии version
return_t save_deltas(const char* base_fname, int version,
    int parent, const struct delta* delta_a, const struct delta* delta_b)
{
    char* fname = string_copy_alloc(base_fname);
    switch_filename_to_version(&fname, version);
    FILE* f = fopen(fname, "w");
    free(fname);
    if (f == NULL)
        return ERR_WRITE;
    print_parent(parent, f);

    return_t ret = SUCCESS;
    if (delta_a != NULL)
        ret = delta_print(delta_a, f);
    if (ret == SUCCESS && delta_b != NULL)
        ret = delta_print(delta_b, f);
    fclose(f);
    return ret;
}

// Загрузить дельту, соответствующую одной конкретной версии
return_t load_delta(struct delta* delta, struct version_tree* vt, int version)
{
    assert(vt_version_is_known(vt, version));

    char* version_filename = string_copy_alloc(vt->base_fname);
    switch_filename_to_version(&version_filename, version);
    FILE* f = fopen(version_filename, "r");
    free(version_filename);

    return_t ret = f == NULL ? ret = ERR_NO_SUCH_FILE : SUCCESS;

    if (ret == SUCCESS)
        ret = read_parent(NULL, f);

    if (ret == SUCCESS)
        ret = delta_load(delta, f);

    if (f != NULL)
        fclose(f);

    return ret;
}

// Применить к тексту изменения на пути вверх от descendant к ancestor.
// Так как идём вверх, изменения по сути откатываются.
return_t apply_upwards(char** text, struct version_tree* vt, int descendant, int ancestor)
{
    assert(vt_version_is_known(vt, ancestor));
    assert(vt_version_is_known(vt, descendant));

    if (descendant == ancestor)
        return SUCCESS;

    struct delta delta = DELTA_INIT;
    return_t ret = load_delta(&delta, vt, descendant);
    delta_reverse(&delta);

    if (ret == SUCCESS)
        ret = delta_apply(text, &delta);

    delta_free(&delta);

    return ret == SUCCESS ? apply_upwards(text, vt, vt_get_parent(vt, descendant), ancestor) : ret;
}

// Применить к дереву изменения на пути вниз от ancestor к descendant
return_t apply_downwards(char** text, struct version_tree* vt, int ancestor, int descendant)
{
    assert(vt_version_is_known(vt, ancestor));
    assert(vt_version_is_known(vt, descendant));

    if (ancestor == descendant)
        return SUCCESS;

    return_t ret = apply_downwards(text, vt, ancestor, vt_get_parent(vt, descendant));
    struct delta delta = DELTA_INIT;
    if (ret == SUCCESS)
        ret = load_delta(&delta, vt, descendant);
    if (ret == SUCCESS)
        ret = delta_apply(text, &delta);
    delta_free(&delta);

    return ret;
}

// Загрузить текст версии, загрузив для этого текст нулевой версии и пройдя путь от корня
return_t vt_checkout_from_root(char** out, struct version_tree* vt, int version)
{
    if (!vt_version_is_known(vt, version))
        return ERR_INVALID_VERSION;
    char* base_text = NULL;
    return_t ret = read_file(&base_text, vt->base_fname);
    if (ret == SUCCESS)
        ret = apply_downwards(&base_text, vt, 0, version);
    if (ret == SUCCESS)
        *out = base_text;
    return ret;
}

// Последовательное применение всех изменений на пути в дереве между двумя вершинами.
// На пути вверх изменения откатываются.
return_t vt_apply_path(char** text, struct version_tree* vt, int start, int dest)
{
    if (!vt_version_is_known(vt, start) || !vt_version_is_known(vt, dest))
        return ERR_INVALID_VERSION;
    int common_ancestor = vt_find_common_ancestor(vt, start, dest);
    if (common_ancestor < 0)
        return ERR_INVALID_VERSION;

    char* copy = string_copy_alloc(*text);
    return_t ret = apply_upwards(&copy, vt, start, common_ancestor);
    if (ret == SUCCESS)
        ret = apply_downwards(&copy, vt, common_ancestor, dest);

    if (ret == SUCCESS) {
        free(*text);
        *text = copy;
    } else
        free(copy);
    return ret;
}

// Проверить, что файл есть
bool file_exists(const char * path)
{
    return read_parent_from_path(NULL, path) != ERR_NO_SUCH_FILE;
}

// Найти ещё не использованный номер версии
return_t find_new_version(int* new_version, int parent, struct version_tree* vt)
{
    char* fname = string_copy_alloc(vt->base_fname);
    for (int v = parent + 1; v < 10000000; ++v) {
        switch_filename_to_version(&fname, v);

        if (!file_exists(fname)) {
            *new_version = v;
            free(fname);
            return SUCCESS;
        }
    }
    free(fname);
    return ERR_VERSIONS_LIMIT;
}

// Зафиксировать дельту в новой версии
return_t vt_push(int* child, struct version_tree* vt, int parent, struct delta* delta)
{
    int new_version;
    return_t ret = find_new_version(&new_version, parent, vt);

    if (ret == SUCCESS)
        ret = save_deltas(vt->base_fname, new_version, parent, delta, NULL);

    if (ret == SUCCESS) {
        *child = new_version;
        set_parent(vt, *child, parent);
    }

    return ret;
}

// Удалить версию
return_t vt_delete_version(struct version_tree* vt, int deleted)
{
    struct delta deleted_delta = DELTA_INIT;
    return_t ret = load_delta(&deleted_delta, vt, deleted);
    if (ret != SUCCESS)
        return ret;

    assert(vt_version_is_known(vt, deleted));
    int parent = vt_get_parent(vt, deleted);

    char* fname = string_copy_alloc(vt->base_fname);

    for (int i = 1; ret == SUCCESS && i < (int)vt->capacity; ++i)
        if (vt_get_parent(vt, i) == deleted) {
        struct delta child_delta = DELTA_INIT;
        ret = load_delta(&child_delta, vt, i);
        assert(ret == SUCCESS);

        switch_filename_to_version(&fname, i);
        ret = save_deltas(vt->base_fname, i, parent, &deleted_delta, &child_delta);
        assert(ret == SUCCESS);
        set_parent(vt, i, parent);
        }
    set_parent(vt, deleted, -1);
    switch_filename_to_version(&fname, deleted);
    DeleteFileA(fname);
    free(fname);
    return SUCCESS;
}

// Рекурсивная часть выписывания пути от корня
void traverse_from_root_rec(int** path, size_t* path_len, size_t* path_capacity, struct version_tree* vt, int cur)
{
    assert(vt_version_is_known(vt, cur));

    if (cur != 0)
        traverse_from_root_rec(path, path_len, path_capacity, vt, vt_get_parent(vt, cur));

    if (*path_len == *path_capacity) {
        *path_capacity *= 2;
        checked_realloc((void**)path, *path_capacity * sizeof(int));
    }
    (*path)[(*path_len)++] = cur;
}

// Выписать путь от корня к вершине в массив, на ходу динамически перевыделяя его
void traverse_from_root(int** path, size_t* path_len, struct version_tree* vt, int version)
{
    size_t capacity = 3;
    *path = (int*)checked_malloc(3 * sizeof(int));
    *path_len = 0;
    traverse_from_root_rec(path, path_len, &capacity, vt, version);
}

// Старый порядок версий: 0-1-2-3-4
// Новый порядок версий:  0-3-2-1-4 , но 0 и 4 поменялись состояниями
// Новое состояние 0 запишется в vcs_rebase
return_t vt_reverse_on_path_to_root(struct version_tree* vt, int version)
{
    if (!vt_version_is_known(vt, version))
        return ERR_INVALID_VERSION;
    if (version == 0)
        return SUCCESS;

    size_t path_len;
    int* path;
    traverse_from_root(&path, &path_len, vt, version);

    struct delta prev_delta = DELTA_INIT;
    return_t ret = load_delta(&prev_delta, vt, path[path_len - 1]);
    int parent = 0;

    for (size_t i = path_len - 2; ret == SUCCESS && i >= 1; --i) { // Сохраняем 3-2-1
        struct delta next_delta = DELTA_INIT;
        ret = load_delta(&next_delta, vt, path[i]);
        if (ret == SUCCESS) {
            delta_reverse(&prev_delta);
            ret = save_deltas(vt->base_fname, path[i], parent, &prev_delta, NULL);
        }
        delta_free(&prev_delta);
        prev_delta = next_delta;
        parent = path[i];
    }
    if (ret == SUCCESS) { // Сохраняем 4
        delta_reverse(&prev_delta);
        ret = save_deltas(vt->base_fname, path[path_len - 1], parent, &prev_delta, NULL);
    }

    delta_free(&prev_delta);
    free(path);

    return ret;
}


/*---------------------------------------------------------------- VERSION SYSTEM STATE------------------------------------------------*/
// Структура vcs_state (Version Control System state) полностью описывает состояние нашей
// системы контроля версий.
struct vcs_state {
    // Исходный текст версии version
    char* clean_state;
    // Текущий текст, возможно, ещё не сохранённый
    char* working_state;

    // Последовательность изменений, переводящая clean_state в working_state
    struct delta changes;

    // Сохранённое дерево коммитов и путь к файлу версии 0, по сути не зависит от остальных полей
    struct version_tree vt;
    // Текущая версия
    int version;
};
#define VCS_INIT { NULL, NULL, DELTA_INIT, VERSION_TREE_INIT, -1 }
const struct vcs_state vcs_init = VCS_INIT;

// Все функции (кроме vcs_free и vcs_is_open) здесь прямо соответствуют командам нашей системы контроля версий.

void vcs_free(struct vcs_state* vcs)
{
    free(vcs->clean_state);
    free(vcs->working_state);
    delta_free(&vcs->changes);
    vt_free(&vcs->vt);
    *vcs = vcs_init;
}

bool vcs_is_open(const struct vcs_state* vcs)
{
    return vcs->working_state != NULL;
}

return_t vcs_open(struct vcs_state* vcs, const char* fname, int version)
{
    assert(vcs != NULL);
    assert(fname != NULL);

    struct vcs_state new_vcs = VCS_INIT;

    new_vcs.version = version;
    return_t ret = vt_load(&new_vcs.vt, fname);

    if (ret == SUCCESS)
        ret = vt_checkout_from_root(&new_vcs.clean_state, &new_vcs.vt, version);

    if (ret == SUCCESS) {
        new_vcs.working_state = string_copy_alloc(new_vcs.clean_state);

        vcs_free(vcs);
        *vcs = new_vcs;
    } else
        vcs_free(&new_vcs);

    return ret;
}

return_t vcs_print(const struct vcs_state* vcs, FILE* stream)
{
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;
    return fputs(vcs->working_state, stream) >= 0 ? SUCCESS : ERR_WRITE;
}

return_t vcs_add(struct vcs_state* vcs, size_t i, const char* data)
{
    assert(vcs != NULL);
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;

    return_t ret = string_insert(&vcs->working_state, i, data);
    if (ret == SUCCESS)
        delta_append(&vcs->changes,
        delta_line_new(i, string_copy_alloc(data), DELTA_ADD));
    return ret;
}

return_t vcs_remove(struct vcs_state* vcs, size_t i, size_t j)
{
    assert(vcs != NULL);
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;

    if (check_substr(strlen(vcs->working_state), i, j - i, NULL)) {
        char* substr = string_substr_alloc(vcs->working_state, i, j - i);
        delta_append(&vcs->changes, delta_line_new(i, substr, DELTA_ERASE));
    }
    return string_erase(&vcs->working_state, i, j - i);
}

return_t vcs_edit(struct vcs_state* vcs, size_t i, size_t j, const char* data)
{
    assert(vcs != NULL);
    assert(data != NULL);
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;

    size_t len = j - i;
    if (i >= j || !check_substr(strlen(vcs->working_state), i, len, NULL))
        return ERR_INVALID_RANGE;

    return_t ret = vcs_remove(vcs, i, j);
    assert(ret == SUCCESS);
    ret = vcs_add(vcs, i, data);
    assert(ret == SUCCESS);

    return SUCCESS;
}

return_t vcs_push(struct vcs_state* vcs)
{
    assert(vcs != NULL);
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;

    return_t ret = vt_push(&vcs->version, &vcs->vt, vcs->version, &vcs->changes);

    if (ret == SUCCESS) {
        string_assign_copy(&vcs->clean_state, vcs->working_state);
        delta_free(&vcs->changes);
    }

    return ret;
}

return_t vcs_pull(struct vcs_state* vcs, int version)
{
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;

    return_t ret = vt_apply_path(&vcs->clean_state, &vcs->vt, vcs->version, version);
    if (ret == SUCCESS) {
        string_assign_copy(&vcs->working_state, vcs->clean_state);
        delta_free(&vcs->changes);
        vcs->version = version;
    }
    return ret;
}

return_t vcs_delete_version(struct vcs_state* vcs, int version)
{
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;
    if (version == 0)
        return ERR_INVALID_VERSION;
    if (version == vcs->version)
        return ERR_DELETE_CURRENT;
    return vt_delete_version(&vcs->vt, version);
}

return_t vcs_save(struct vcs_state* vcs, const char* path)
{
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;

    FILE* f = fopen(path, "w");
    if (f == NULL)
        return ERR_WRITE;
    return_t ret = vcs_print(vcs, f);
    fclose(f);
    return ret;
}

return_t vcs_rebase(struct vcs_state* vcs)
{
    if (!vcs_is_open(vcs))
        return ERR_NOT_OPEN;

    return_t ret = vt_reverse_on_path_to_root(&vcs->vt, vcs->version);
    if (ret != SUCCESS)
        return ret;

    ret = vcs_save(vcs, vcs->vt.base_fname);
    if (ret != SUCCESS)
        return ret;

    string_assign_copy(&vcs->working_state, vcs->clean_state);
    delta_free(&vcs->changes);
    vcs->version = 0;
    return SUCCESS;
}

const char* init_text = "ABCDEFGH1234567890";

// Подготовка a.txt, как в описании задачи
void prepare_a()
{
    system("del a.*");

    FILE* f = fopen("a.txt", "w");
    fputs(init_text, f);
    fclose(f);
}

// Проверка соответствия примеру из описания задачи
void sample_test()
{
    prepare_a();

    FILE* f = fopen("a.txt", "r");

    struct vcs_state vcs = VCS_INIT;

    assert(vcs_open(&vcs, "a.txt", 0) == SUCCESS);
    fclose(f);
    assert(strcmp(vcs.working_state, init_text) == 0);

    assert(vcs_edit(&vcs, 2, 3, "O") == SUCCESS);

    assert(vcs_remove(&vcs, 1, 10) == SUCCESS);
    assert(strcmp(vcs.working_state, "A34567890") == 0);

    assert(vcs_add(&vcs, 19, "aksjhda") == ERR_INVALID_RANGE);
    assert(vcs_add(&vcs, 10, "uhsdjs") == ERR_INVALID_RANGE);

    assert(vcs_add(&vcs, 9, "XYZ") == SUCCESS);
    assert(strcmp(vcs.working_state, "A34567890XYZ") == 0);

    assert(vcs_push(&vcs) == SUCCESS);
    assert(vcs.version == 1);

    assert(vcs_edit(&vcs, 0, 2, "IBKS") == SUCCESS);
    assert(strcmp(vcs.working_state, "IBKS4567890XYZ") == 0);

    assert(vcs_pull(&vcs, 2) == ERR_INVALID_VERSION);
    assert(vcs_push(&vcs) == SUCCESS);
    assert(vcs.version == 2);

    assert(vcs_pull(&vcs, 1) == SUCCESS);
    assert(strcmp(vcs.working_state, "A34567890XYZ") == 0);

    assert(vcs_add(&vcs, 12, "ZERO") == SUCCESS);
    assert(strcmp(vcs.working_state, "A34567890XYZZERO") == 0);
    assert(vcs_push(&vcs) == SUCCESS);
    assert(vcs.version == 3);

    assert(vcs_rebase(&vcs) == SUCCESS);
    assert(vcs.version == 0);
    //    assert(vcs_delete_version(&vcs, 1) == SUCCESS);

    vcs_free(&vcs);
}

void handle_result(return_t r)
{
    if (r != SUCCESS)
        puts(error_description(r));
}

int main()
{
    sample_test();

    char command_buf[30];
    char open_buf[300];
    char fname[300];
    struct vcs_state vcs = VCS_INIT;

    while (!feof(stdin) && !ferror(stdin)) {
        if (scanf("%30s", command_buf) < 0)
            continue;

        if (strcmp(command_buf, "open") == 0) {
            fgets(open_buf, 300, stdin);
            sscanf(open_buf, "%s", fname);
            int version = 0;
            sscanf(open_buf, "%d", &version);

            handle_result(vcs_open(&vcs, fname, version));
        } else if (strcmp(command_buf, "print") == 0) {
            handle_result(vcs_print(&vcs, stdout));
            printf("\n");
        } else if (strcmp(command_buf, "edit") == 0) {
            int i, j;
            char* data;
            scanf("%d %d ", &i, &j);
            read_line(&data, stdin);
            handle_result(vcs_edit(&vcs, i, j, data));
            free(data);
        } else if (strcmp(command_buf, "add") == 0) {
            int i;
            char* data;
            scanf("%d ", &i);
            read_line(&data, stdin);
            handle_result(vcs_add(&vcs, i, data));
            free(data);
        } else if (strcmp(command_buf, "remove") == 0) {
            int i, j;
            scanf("%d %d", &i, &j);
            handle_result(vcs_remove(&vcs, i, j));
        } else if (strcmp(command_buf, "push") == 0) {
            handle_result(vcs_push(&vcs));
            printf("%d\n", vcs.version);
        } else if (strcmp(command_buf, "pull") == 0) {
            int v;
            scanf("%d", &v);
            handle_result(vcs_pull(&vcs, v));
        } else if (strcmp(command_buf, "delete_version") == 0) {
            int v;
            scanf("%d", &v);
            handle_result(vcs_delete_version(&vcs, v));
        } else if (strcmp(command_buf, "rebase") == 0) {
            handle_result(vcs_rebase(&vcs));
        } else if (strcmp(command_buf, "save") == 0) {
            scanf("%300s", fname);
            handle_result(vcs_save(&vcs, fname));
        } else {
            printf("Unknown command %s\n", command_buf);
        }
    }
    vcs_free(&vcs);

    return 0;
}
