#ifndef _GNU_SOURCE
// for strdup
#define _GNU_SOURCE
#endif

#include "intrusive-list.h"

#include <assert.h>
#include <malloc.h>
#include <string.h>

typedef struct {
    char *title;
    char *text;
    list_link linking;
} ListTest;

DEFINE_INTRUSIVE_LIST(ListTest, linking, ListTestList, list_test_list)

static void
list_test_free(ListTest *t)
{
    free(t->title);
    free(t->text);
    free(t);
}

int
main(void)
{
    ListTestList list;
    list_test_list_init(&list);

    assert(list_test_list_is_empty(&list));
    assert(list_test_list_length(&list) == 0);

    ListTest *i = (ListTest *)malloc(sizeof(ListTest));
    i->title = strdup("Hello World!");
    i->text = strdup("");
    list_test_list_insert_front(&list, i);

    i = (ListTest *)malloc(sizeof(ListTest));
    i->title = strdup("Before!");
    i->text = strdup("Erster!");
    list_test_list_insert_front(&list, i);

    i = (ListTest *)malloc(sizeof(ListTest));
    i->title = strdup("Letzter!");
    i->text = NULL;
    list_test_list_insert_back(&list, i);

    ListTest *j = (ListTest *)malloc(sizeof(ListTest));
    j->title = strdup("Vorletzter!");
    j->text = NULL;
    list_test_list_insert_before(i, j);

    j = (ListTest *)malloc(sizeof(ListTest));
    j->title = strdup("Allerletzter!");
    j->text = NULL;
    list_test_list_insert_after(i, j);

    assert(!list_test_list_is_empty(&list));
    assert(list_test_list_length(&list) == 5);

    ListTest *k = NULL;
    int c = 0;
    while (list_test_list_iter(&list, &k)) {
        if (c == 0) assert(!strcmp(k->title, "Before!"));
        if (c == 1) assert(!strcmp(k->title, "Hello World!"));
        if (c == 2) assert(!strcmp(k->title, "Vorletzter!"));
        if (c == 3) assert(!strcmp(k->title, "Letzter!"));
        if (c == 4) assert(!strcmp(k->title, "Allerletzter!"));
        assert(c < 5);
        ++c;
    }

    k = list_test_list_next(&list, list_test_list_first(&list));
    list_test_list_remove(k);
    list_test_free(k);

    assert(list_test_list_length(&list) == 4);

    k = NULL;
    c = 0;
    while (list_test_list_riter(&list, &k)) {
        if (c == 3) assert(!strcmp(k->title, "Before!"));
        if (c == 2) assert(!strcmp(k->title, "Vorletzter!"));
        if (c == 1) assert(!strcmp(k->title, "Letzter!"));
        if (c == 0) assert(!strcmp(k->title, "Allerletzter!"));
        assert(c < 4);
        ++c;
    }

    list_test_list_clear(&list, list_test_free);
    assert(list_test_list_is_empty(&list));
    assert(list_test_list_length(&list) == 0);
}
