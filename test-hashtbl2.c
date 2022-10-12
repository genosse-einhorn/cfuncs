#ifndef _GNU_SOURCE
#   define _GNU_SOURCE
#endif

#include "hashtbl2.h"

#include "str.h"

#include <assert.h>
#include <stdio.h>

//HASHTBL_DEFINE(ConstStrDictionary, const_str_dictionary, const char *, const char *, str_hash, str_equal)
//HASHTBL_DEFINE_2(WordCountDic, word_count_dic, char *, const char *, str_dup, free, int, int, /*nop*/, (void), str_hash, str_equal);

HASHTBL_DEFINE(ConstStrDictionary, const_str_dictionary,
               HASHTBL_KEY(const char *, str_hash, str_equal),
               HASHTBL_VALUE(const char *))

HASHTBL_DEFINE(WordCountDic, word_count_dic,
               HASHTBL_KEY_FULL(char *, const char *, str_dup, free, str_hash, str_equal),
               HASHTBL_VALUE(int))

static void
test_wordcount(void)
{
    WordCountDic dic;
    word_count_dic_init(&dic);

    FILE *f = fopen("wordlist.txt", "r");

    char *buf = NULL;
    size_t n = 0;
    while (getline(&buf, &n, f) >= 0) {
        str_trim_inplace(buf);

        WordCountDic_Item *item = word_count_dic_lookup(&dic, buf);
        if (item) {
            item->value++;
        } else {
            word_count_dic_set(&dic, buf, 1);
        }
    }

    free(buf);

    fclose(f);

    // remove all words with low count
    WordCountDic_Iterator it;
    word_count_dic_iterator_init(&dic, &it);
    while (!word_count_dic_iterator_at_end(&it)) {
        WordCountDic_Item *item = word_count_dic_iterator_item(&it);
        if (item->value < 53) {
            word_count_dic_iterator_delete(&it);
        }

        word_count_dic_iterator_next(&it);
    }

    // print it
    word_count_dic_iterator_init(&dic, &it);
    while (!word_count_dic_iterator_at_end(&it)) {
        WordCountDic_Item *item = word_count_dic_iterator_item(&it);
        if (item->value > 1) {
            printf("%s: %d\n", item->key, item->value);
        }

        word_count_dic_iterator_next(&it);
    }

    printf("element count: %u\n", dic.element_count);
    printf("table size %u: %u\n", dic.table_size_idx, _hashtbl_size_map[dic.table_size_idx]);

    // count collisions
    size_t collcount = 0;
    for (size_t i = 0; i < _hashtbl_size_map[dic.table_size_idx]; ++i) {
        unsigned item_i = dic.hashtbl[i];
        while (item_i != (unsigned)-1 && dic.item_storage[item_i].next != (unsigned)-1) {
            collcount++;
            item_i = dic.item_storage[item_i].next;
        }
    }

    printf("collision count: %zu\n", collcount);

    assert(word_count_dic_check_internal_sanity(&dic));

    word_count_dic_clear(&dic);
}

int main(void)
{
    ConstStrDictionary dic;
    const_str_dictionary_init(&dic);

    const_str_dictionary_set(&dic, "Hello", "World");

    ConstStrDictionary_Item *item = const_str_dictionary_lookup(&dic, "Hello");
    assert(!strcmp(item->value, "World"));

    const_str_dictionary_set(&dic, "Hello", "Hohoho");
    item = const_str_dictionary_lookup(&dic, "Hello");
    assert(!strcmp(item->value, "Hohoho"));

    assert(const_str_dictionary_check_internal_sanity(&dic));

    const_str_dictionary_clear(&dic);

    test_wordcount();
}
