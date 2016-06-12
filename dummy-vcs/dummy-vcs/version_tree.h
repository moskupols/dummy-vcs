#pragma once

#include <stdio.h>

#include "error.h"
#include "delta.h"

// ��������� version_tree ��������� ���������� ������ ������ �����.
struct version_tree
{
    // ���� � ������� ������ �����
    char* base_fname;

    // ������ �������: parent[a] �������� ������������ ������ ������ a,
    // ���� -1, ���� ������ a ������������ ��� a=0.
    // ����� � capacity �������� ���������� std::vector<int>
    int* parent;
    size_t capacity;
};
#define VERSION_TREE_INIT { NULL, NULL, 0 }
static const struct version_tree version_tree_init = VERSION_TREE_INIT;

// ��������� ������ ������ ������. ���������� ��� open.
return_t vt_load(struct version_tree* out, const char* base_fname);
// ���������� � �������� ���������.
void vt_free(struct version_tree* vt);

// �������� ����� ������������ ������ ��� -1, ���� � ���.
int vt_get_parent(struct version_tree* vt, int child);
// �������� ������� � ������ ���������� ������
bool vt_version_is_known(struct version_tree* vt, int version);

// ���������� ���������� ������ ������ ���� ������, ����� ��� pull
int vt_find_common_ancestor(struct version_tree* vt, int a, int b);

// ���������������� ���������� ���� ��������� �� ���� � ������ ����� ����� ���������.
// �� ���� ����� ��������� ������������.
return_t vt_apply_path(char** text, struct version_tree* vt, int start, int dest);
// �������� ������ ���� ������� �� ���� �� �����. ���������� � open.
return_t vt_checkout_from_root(char** out, struct version_tree* vt, int version);

// ���������� ����� ������
return_t vt_push(
        int* child, struct version_tree* vt, int parent, struct delta* delta);

// �������� ������
return_t vt_delete_version(struct version_tree* vt, int version);

// �������� ��������� �� ���� �� ����� �� �������, ����� ��� rebase.
return_t vt_reverse_from_root(struct version_tree* vt, int version);
