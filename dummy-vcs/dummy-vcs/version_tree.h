#pragma once

#include <stdio.h>

#include "error.h"
#include "delta.h"

// Структура version_tree описывает собственно дерево версий файла.
struct version_tree
{
    // Путь к нулевой версии файла
    char* base_fname;

    // Массив предков: parent[a] содержит родительскую версию версии a,
    // либо -1, если версия a отстутствует или a=0.
    // Вкупе с capacity выделяется аналогично std::vector<int>
    int* parent;
    size_t capacity;
};
#define VERSION_TREE_INIT { NULL, NULL, 0 }
static const struct version_tree version_tree_init = VERSION_TREE_INIT;

// Загрузить дерево версий заново. Вызывается при open.
return_t vt_load(struct version_tree* out, const char* base_fname);
// Освободить и очистить структуру.
void vt_free(struct version_tree* vt);

// Получить номер родительской версии или -1, если её нет.
int vt_get_parent(struct version_tree* vt, int child);
// Предикат наличия в дереве конкретной версии
bool vt_version_is_known(struct version_tree* vt, int version);

// Нахождение ближайшего общего предка двух вершин, нужно для pull
int vt_find_common_ancestor(struct version_tree* vt, int a, int b);

// Последовательное применение всех изменений на пути в дереве между двумя вершинами.
// На пути вверх изменения откатываются.
return_t vt_apply_path(char** text, struct version_tree* vt, int start, int dest);
// Загрузка версии путём прохода по пути от корня. Вызывается в open.
return_t vt_checkout_from_root(char** out, struct version_tree* vt, int version);

// Сохранение новой версии
return_t vt_push(
        int* child, struct version_tree* vt, int parent, struct delta* delta);

// Удаление версии
return_t vt_delete_version(struct version_tree* vt, int version);

// Разворот изменений на пути от корня до вершины, нужно для rebase.
return_t vt_reverse_from_root(struct version_tree* vt, int version);
