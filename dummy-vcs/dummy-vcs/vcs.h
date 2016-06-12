#pragma once

#include <stdio.h>

#include "error.h"
#include "delta.h"
#include "version_tree.h"

// Структура vcs_state (Version Control System state) полностью описывает состояние нашей
// системы контроля версий.
struct vcs_state
{
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
static const struct vcs_state vcs_init = VCS_INIT;

// Все функции здесь прямо соответствуют функциям нашей системы контроля версий.

return_t vcs_open(struct vcs_state* vcs, const char* fname, int version);
return_t vcs_print(const struct vcs_state* vcs, FILE* stream);

return_t vcs_edit(struct vcs_state* vcs, size_t i, size_t j, const char* data);
return_t vcs_add(struct vcs_state* vcs, size_t i, const char* data);
return_t vcs_remove(struct vcs_state* vcs, size_t i, size_t j);

return_t vcs_push(struct vcs_state* vcs);
return_t vcs_pull(struct vcs_state* vcs, int version);
return_t vcs_delete_version(struct vcs_state* vcs, int version);
return_t vcs_rebase(struct vcs_state* vcs);

return_t vcs_save(struct vcs_state* vcs, const char* path);

void vcs_free(struct vcs_state* vcs);

