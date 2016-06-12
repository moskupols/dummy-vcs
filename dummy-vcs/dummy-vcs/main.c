#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#include "vcs.h"
#include "parse.h"

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

    while (!feof(stdin) && !ferror(stdin))
    {
        if (scanf("%30s", command_buf) < 0)
            continue;

        if (strcmp(command_buf, "open") == 0)
        {
            fgets(open_buf, 300, stdin);
            sscanf(open_buf, "%s", fname);
            int version = 0;
            sscanf(open_buf, "%d", &version);

            handle_result(vcs_open(&vcs, fname, version));
        }
        else if (strcmp(command_buf, "print") == 0)
        {
            handle_result(vcs_print(&vcs, stdout));
            printf("\n");
        }
        else if (strcmp(command_buf, "edit") == 0)
        {
            int i, j;
            char* data;
            scanf("%d %d ", &i, &j);
            read_line(&data, stdin);
            handle_result(vcs_edit(&vcs, i, j, data));
        }
        else if (strcmp(command_buf, "add") == 0)
        {
            int i;
            char* data;
            scanf("%d ", &i);
            read_line(&data, stdin);
            handle_result(vcs_add(&vcs, i, data));
        }
        else if (strcmp(command_buf, "remove") == 0)
        {
            int i, j;
            scanf("%d %d", &i, &j);
            handle_result(vcs_remove(&vcs, i, j));
        }
        else if (strcmp(command_buf, "push") == 0)
        {
            handle_result(vcs_push(&vcs));
            printf("%d\n", vcs.version);
        }
        else if (strcmp(command_buf, "pull") == 0)
        {
            int v;
            scanf("%d", &v);
            handle_result(vcs_pull(&vcs, v));
        }
        else if (strcmp(command_buf, "delete_version") == 0)
        {
            int v;
            scanf("%d", &v);
            handle_result(vcs_delete_version(&vcs, v));
        }
        else if (strcmp(command_buf, "rebase") == 0)
        {
            handle_result(vcs_rebase(&vcs));
        }
        else if (strcmp(command_buf, "save") == 0)
        {
            scanf("%300s", fname);
            handle_result(vcs_save(&vcs, fname));
        }
        else
        {
            printf("Unknown command %s\n", command_buf);
        }
    }
    vcs_free(&vcs);

    return 0;
}
