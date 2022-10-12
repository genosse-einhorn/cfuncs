#define _POSIX_C_SOURCE 200809L

#include "hashtbl.h"

#include "str.h"

#include <assert.h>
#include <stdio.h>

HASHTBL_DEFINE(ConstStrDictionary, const_str_dictionary, const char *, const char *, str_hash, str_equal)
HASHTBL_DEFINE_2(WordCountDic, word_count_dic, char *, const char *, str_dup, free, int, int, /*nop*/, (void), str_hash, str_equal);

static void
test_wordcount(void)
{
    WordCountDic dic = word_count_dic_create();

    FILE *f = fopen("wordlist.txt", "r");

    char *buf = NULL;
    size_t n = 0;
    while (getline(&buf, &n, f) >= 0) {
        str_trim_inplace(buf);

        WordCountDic_Item *item = word_count_dic_lookup_or_insert(&dic, buf, 0);
        item->value++;
    }

    free(buf);

    fclose(f);

    // FIXME! implement and use iterator here
    for (size_t i = 0; i < _hashtbl_size_map[dic->table_size]; ++i) {
        WordCountDic_Item *item = dic->items[i];
        while (item) {
            if (item->value > 1) {
                printf("%s: %d\n", item->key, item->value);
            }

            item = item->next;
        }
    }

    printf("element count: %zu\n", dic->element_count);
    printf("table size %zu: %zu\n", dic->table_size, _hashtbl_size_map[dic->table_size]);

    // count collisions
    size_t collcount = 0;
    for (size_t i = 0; i < _hashtbl_size_map[dic->table_size]; ++i) {
        WordCountDic_Item *item = dic->items[i];
        while (item && item->next) {
            collcount++;
            item = item->next;
        }
    }

    printf("collision count: %zu\n", collcount);

    assert(word_count_dic_check_internal_sanity(dic));

    word_count_dic_free(dic);
}

int main(void)
{
    ConstStrDictionary dic = const_str_dictionary_create();

    const_str_dictionary_set(&dic, "Hello", "World");

    ConstStrDictionary_Item *item = const_str_dictionary_lookup(dic, "Hello");
    assert(!strcmp(item->value, "World"));

    const_str_dictionary_set(&dic, "Hello", "Hohoho");
    item = const_str_dictionary_lookup(dic, "Hello");
    assert(!strcmp(item->value, "Hohoho"));

    assert(const_str_dictionary_check_internal_sanity(dic));

    const_str_dictionary_free(dic);

    test_wordcount();
}
