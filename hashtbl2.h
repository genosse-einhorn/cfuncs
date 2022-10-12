#pragma once
/*
 * Copyright © 2021 Jonas Kümmerlin <jonas@kuemmerlin.eu>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 */

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

/* Macro-based generic hash map for C
 *
 * How-To:
 *      // Define type and functions
 *      HASHTBL_DEFINE(MyTable, my_table,
 *                     HASHTBL_KEY(const char *, str_hash, str_equal),
 *                     HASHTBL_VALUE(int))
 *
 *      // Create a map
 *      MyTable ht;
 *      my_table_init(&ht);
 *
 *      // Add some things
 *      my_table_set(&ht, "Hello World", 42);
 *      my_table_set(&ht, "Goodbye World", 21);
 *      my_table_set(&ht, "What’s up, World", 10948104);
 *
 *      // Delete stuff
 *      my_table_remove(&ht, "Goodbye World");
 *
 *      // Lookup stuff
 *      MyTable_Item *i;
 *
 *      i = my_table_lookup(&ht, "Hello World");
 *      assert(i != NULL);
 *      assert(!strcmp(i->key, "Hello World"));
 *      assert(i->value == 42);
 *
 *      i = my_table_lookup(&ht, "Goodbye World");
 *      assert(i == NULL);
 *
 *      // Cleanup
 *      my_table_clear(&ht);
 *
 * Limits:
 *      - only supports up to UINT_MAX-2 elements
 *      - item pointers are potentially invalid after adding more elements
 *      - will never shrink when when removing elements
 *      - uses separate chaining, performance can often be better with open addressing
 *      - not safe against algorithmic complexity attacks (unless your hash function is really clever somehow)
 *
 * Reference Docs:
 *
 *      HASHTBL_DEFINE(TypeName, function_prefix, KEY_SPEC, VALUE_SPEC)
 *      HASHTBL_DEFINE_FULL(TypeName, function_prefix, KEY_SPEC, VALUE_SPEC, reallocarray_fun, free_fun)
 *          Defines types and functions for a hashtable. KEY_SPEC is one of HASHTBL_KEY*
 *          and VALUE_SPEC is one of HASHTBL_VALUE*, as shown below. reallocarray_fun
 *          is a function compatible to reallocarray(3), and free_fun its counterpart
 *          like free(3).
 *
 *      HASHTBL_KEY(Type, hash_func, equal_func)
 *      HASHTBL_KEY_FULL(Type, ConstType, dup_func, free_func, hash_func, equal_func)
 *
 *      HASHTBL_VALUE(Type)
 *      HASHTBL_VALUE_FULL(Type, ConstType, dup_func, free_func)
 *
 *      void
 *      function_prefix_init(TypeName *tbl)
 *          Initializes a hash table
 *
 *      void
 *      function_prefix_init_reserve(TypeName *tbl, unsigned count)
 *          Initializes a hash table, sized for `count` elements
 *
 *      void
 *      function_prefix_clear(TypeName *tbl)
 *          Frees up all memory allocated for the hash table. If specified,
 *          the `key_free_func` and `value_free_func` will be called for each item
 *
 *      TypeName_Item *
 *      function_prefix_item(TypeName *tbl, ConstKeyType key)
 *          Looks up the given key inside the hash table and returns a pointer to
 *          the internal item structure, or NULL if the key is not found.
 *          The TypeName_Item struct contains `key` and `value` members.
 *          NOTE: the returned TypeName_Item* pointer is only valid until the
 *          hash table is modified. Adding or removing elements may move the
 *          items around in memory, rendering any pointers invalid.
 *
 *      TypeName_Item *
 *      function_prefix_set(TypeName *tbl, ConstKeyType key, ConstValueType value)
 *          Insert the given value into the hash table, potentially overriding
 *          the old value for the given key. If specified, `key_dup_func`,
 *          `value_dup_func` and `value_free_func` will be called as needed.
 *          Returns a pointer to the internal item structure for the key (see
 *          also function_prefix_item()), or NULL if the insertion failed
 *          (this can happen if and only if there is no memory available or the
 *          hash table has reached its maximum size).
 *
 *      void
 *      function_prefix_remove(TypeName *tbl, ConstKeyType key)
 *          Remove the item for the given key from the hash table. If specified,
 *          `key_free_func` and `value_free_func` will be called for the removed
 *          key and value.
 */

#define HASHTBL_KEY(Type, hash_func, equal_func) \
    HASHTBL__INTERNAL_KEY_FULL(Type, Type, /*nop*/, (void), hash_func, equal_func)

#define HASHTBL_KEY_FULL(Type, ConstType, dup_func, free_func, hash_func, equal_func) \
    HASHTBL__INTERNAL_KEY_FULL(Type, ConstType, dup_func, free_func, hash_func, equal_func)

#define HASHTBL__INTERNAL_KEY_FULL(Type, ConstType, dup_func, free_func, hash_func, equal_func) \
    Type, ConstType, dup_func, free_func, hash_func, equal_func

#define HASHTBL_VALUE(Type) \
    HASHTBL__INTERNAL_VALUE_FULL(Type, Type, /*nop*/, (void))

#define HASHTBL_VALUE_FULL(Type, ConstType, dup_func, free_func) \
    HASHTBL__INTERNAL_VALUE_FULL(Type, ConstType, dup_func, free_func)

#define HASHTBL__INTERNAL_VALUE_FULL(Type, ConstType, dup_func, free_func) \
    Type, ConstType, dup_func, free_func

#define HASHTBL_DEFINE(TblTypeName, function_prefix, KEY_SPEC, VALUE_SPEC) \
    HASHTBL__INTERNAL_DEFINE(TblTypeName, function_prefix, KEY_SPEC, VALUE_SPEC, reallocarray, free)

#define HASHTBL_DEFINE_FULL(TblTypeName, function_prefix, KEY_SPEC, VALUE_SPEC, reallocarray_func, free_func) \
    HASHTBL__INTERNAL_DEFINE(TblTypeName, function_prefix, KEY_SPEC, VALUE_SPEC, reallocarray_func, free_func)

#define HASHTBL__INTERNAL_DEFINE(TblTypeName, function_prefix, KeyType, ConstKeyType, key_dup_func, key_free_func, key_hash_func, key_equal_func, ValueType, ConstValueType, value_dup_func, value_free_func, reallocarray, free) \
    \
    typedef KeyType         TblTypeName##_Key; \
    typedef ConstKeyType    TblTypeName##_ConstKey; \
    typedef ValueType       TblTypeName##_Value; \
    typedef ConstValueType  TblTypeName##_ConstValue; \
    typedef struct TblTypeName##_Item { \
        unsigned hash; \
        unsigned next; \
        TblTypeName##_Key   key; \
        TblTypeName##_Value value; \
    } TblTypeName##_Item; \
    typedef struct { \
        unsigned element_count; \
        unsigned table_size_idx; \
        unsigned item_storage_allocated; \
        unsigned item_storage_used; \
        unsigned item_storage_firstfree; \
        unsigned *hashtbl; \
        TblTypeName##_Item *item_storage; \
    } TblTypeName; \
    \
    static inline void \
    function_prefix##_init(TblTypeName *tbl) \
    { \
        tbl->element_count = 0; \
        tbl->table_size_idx = 0; \
        tbl->item_storage_allocated = 0; \
        tbl->item_storage_used = 0; \
        tbl->item_storage_firstfree = (unsigned)-1; \
        tbl->hashtbl = NULL; \
        tbl->item_storage = NULL; \
    } \
    \
    static inline void \
    function_prefix##_clear(TblTypeName *tbl) \
    { \
        for (unsigned i = 0; i < tbl->item_storage_used; ++i) { \
            if (tbl->item_storage[i].next == (unsigned)-2) \
                continue; \
            \
            key_free_func(tbl->item_storage[i].key); \
            value_free_func(tbl->item_storage[i].value); \
        } \
        \
        free(tbl->item_storage); \
        free(tbl->hashtbl); \
        function_prefix##_init(tbl); \
    } \
    \
    static inline unsigned \
    function_prefix##_size(TblTypeName *tbl) \
    { \
        return tbl->element_count; \
    } \
    \
    static inline unsigned \
    function_prefix##_internal_index_for_hash(TblTypeName *tbl, unsigned hash) \
    { \
        return (hash * 11) % _hashtbl_size_map[tbl->table_size_idx]; \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_lookup_with_hash(TblTypeName *tbl, unsigned hash, TblTypeName##_ConstKey key) \
    { \
        if (!tbl->hashtbl) { \
            /* XXX: degenerate case where we could not allocate the hashes */ \
            for (unsigned i = 0; i < tbl->item_storage_used; ++i) { \
                if (tbl->item_storage[i].next != (unsigned)-2 \
                    && tbl->item_storage[i].hash == hash \
                    && key_equal_func(tbl->item_storage[i].key, key)) { \
                        return &tbl->item_storage[i]; \
                } \
            } \
            return NULL; \
        } \
        \
        unsigned hash_i = function_prefix##_internal_index_for_hash(tbl, hash); \
        unsigned item_i = tbl->hashtbl[hash_i]; \
        while (item_i != (unsigned)-1) { \
            if (tbl->item_storage[item_i].hash == hash && key_equal_func(tbl->item_storage[item_i].key, key)) \
                return &tbl->item_storage[item_i]; \
            \
            item_i = tbl->item_storage[item_i].next; \
        } \
        \
        return NULL; \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_lookup(TblTypeName *tbl, TblTypeName##_ConstKey key) \
    { \
        return function_prefix##_lookup_with_hash(tbl, key_hash_func(key), key); \
    } \
    \
    static inline int \
    function_prefix##_contains(TblTypeName *tbl, TblTypeName##_ConstKey key) \
    { \
        return function_prefix##_lookup(tbl, key) != NULL; \
    } \
    \
    static inline void \
    function_prefix##_internal_recreate_hashtbl(TblTypeName *tbl) \
    { \
        free(tbl->hashtbl); \
        tbl->hashtbl = NULL; \
        if (tbl->table_size_idx >= sizeof(_hashtbl_size_map)/sizeof(_hashtbl_size_map[0])) \
            return; /* FIXME: we should never ever be here */ \
        \
        tbl->hashtbl = (unsigned *)reallocarray(NULL, _hashtbl_size_map[tbl->table_size_idx], sizeof(unsigned)); \
        if (!tbl->hashtbl) \
            return; /* FIXME!??? degenerate case where we cant alloc the hash table */ \
        \
        for (unsigned i = 0; i < _hashtbl_size_map[tbl->table_size_idx]; ++i) { \
            tbl->hashtbl[i] = (unsigned)-1; \
        } \
        \
        for (unsigned i = 0; i < tbl->item_storage_used; ++i) { \
            if (tbl->item_storage[i].next == (unsigned)-2) \
                continue; /* free item */ \
            \
            unsigned hash_i = function_prefix##_internal_index_for_hash(tbl, tbl->item_storage[i].hash); \
            tbl->item_storage[i].next = tbl->hashtbl[hash_i]; \
            tbl->hashtbl[hash_i] = i; \
        } \
    } \
    \
    static inline void \
    function_prefix##_internal_auto_grow(TblTypeName *tbl) \
    { \
        unsigned target_table_size = tbl->table_size_idx; \
        while (target_table_size < sizeof(_hashtbl_size_map)/sizeof(_hashtbl_size_map[0])  \
                &&  tbl->element_count > _hashtbl_size_map[target_table_size] - _hashtbl_size_map[target_table_size]/4) \
            target_table_size++; \
        \
        if (target_table_size != tbl->table_size_idx || !tbl->hashtbl) { \
            tbl->table_size_idx = target_table_size; \
            function_prefix##_internal_recreate_hashtbl(tbl); \
        } \
    } \
    \
    static inline unsigned \
    function_prefix##_internal_alloc_item(TblTypeName *tbl) \
    { \
        if (tbl->item_storage_firstfree != (unsigned)-1) { \
            unsigned i = tbl->item_storage_firstfree; \
            tbl->item_storage_firstfree = tbl->item_storage[i].hash; \
            return i; \
        } else if (tbl->item_storage_used < tbl->item_storage_allocated) { \
            return tbl->item_storage_used++; \
        } else { \
            if (tbl->item_storage_allocated >= (unsigned)-2) \
                return (unsigned)-1; /* absolute limit, we need indices -1 and -2 as sentinels */ \
            \
            unsigned newsize = tbl->item_storage_allocated + tbl->item_storage_allocated / 2; \
            if (newsize < 16) /* FIXME: what is a good min value here? */ \
                newsize = 16; \
            if (newsize > (unsigned)-2 || newsize < tbl->item_storage_allocated) \
                newsize = (unsigned)-2; \
            \
            void *a = reallocarray(tbl->item_storage, newsize, sizeof tbl->item_storage[0]); \
            if (a) { \
                tbl->item_storage = (TblTypeName##_Item *)a; \
                tbl->item_storage_allocated = newsize; \
                memset(&tbl->item_storage[tbl->item_storage_used], 0, (tbl->item_storage_allocated - tbl->item_storage_used)*sizeof(tbl->item_storage[0])); \
                return tbl->item_storage_used++; \
            } else { \
                /* FIXME: alloc failed, how to handle?? */ \
            } \
        } \
        return (unsigned)-1; \
    } \
    \
    static inline void \
    function_prefix##_internal_hookup_item(TblTypeName *tbl, unsigned item_i) \
    { \
        if (tbl->hashtbl) { \
            unsigned hash_i = function_prefix##_internal_index_for_hash(tbl, tbl->item_storage[item_i].hash); \
            tbl->item_storage[item_i].next = tbl->hashtbl[hash_i]; \
            tbl->hashtbl[hash_i] = item_i; \
        } else { \
            tbl->item_storage[item_i].next = (unsigned)-1; \
        } \
        tbl->element_count++; \
        function_prefix##_internal_auto_grow(tbl); \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_set_zero(TblTypeName *tbl, TblTypeName##_ConstKey key) \
    { \
        unsigned hash = key_hash_func(key); \
        TblTypeName##_Item *item = function_prefix##_lookup_with_hash(tbl, hash, key); \
        if (item) { \
            value_free_func((item)->value); \
            memset(&item->value, 0, sizeof(item->value)); \
            return item; \
        } else { \
            unsigned item_i = function_prefix##_internal_alloc_item(tbl); \
            if (item_i != (unsigned)-1) { \
                tbl->item_storage[item_i].hash = hash; \
                tbl->item_storage[item_i].key = key_dup_func(key); \
                memset(&tbl->item_storage[item_i].value, 0, sizeof tbl->item_storage[item_i].value); \
                function_prefix##_internal_hookup_item(tbl, item_i); \
                return &tbl->item_storage[item_i]; \
            } else { \
                return NULL; \
            } \
        } \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_set(TblTypeName *ptbl, TblTypeName##_ConstKey key, TblTypeName##_ConstValue value) \
    { \
        TblTypeName##_Item *item = function_prefix##_set_zero(ptbl, key); \
        if (item) { \
            item->value = value_dup_func(value); \
        } \
        return item; \
    } \
    \
    static inline void \
    function_prefix##_internal_dealloc_item(TblTypeName *tbl, unsigned item_i) \
    { \
        key_free_func(tbl->item_storage[item_i].key); \
        value_free_func(tbl->item_storage[item_i].value); \
        tbl->item_storage[item_i].next = (unsigned)-2; \
        tbl->item_storage[item_i].hash = tbl->item_storage_firstfree; \
        tbl->item_storage_firstfree = item_i; \
    } \
    \
    static inline void \
    function_prefix##_remove(TblTypeName *tbl, TblTypeName##_ConstKey key) \
    { \
        unsigned hash = key_hash_func(key); \
        \
        if (!tbl->hashtbl) { \
            /* XXX: degenerate case where we could not allocate the hashes */ \
            for (unsigned i = 0; i < tbl->item_storage_used; ++i) { \
                if (tbl->item_storage[i].next != (unsigned)-2 \
                    && tbl->item_storage[i].hash == hash \
                    && key_equal_func(tbl->item_storage[i].key, key)) { \
                        function_prefix##_internal_dealloc_item(tbl, i); \
                        tbl->element_count--; \
                        return; \
                } \
            } \
        } else { \
            unsigned hash_i = function_prefix##_internal_index_for_hash(tbl, hash); \
            unsigned *p_item_i = &tbl->hashtbl[hash_i]; \
            while (*p_item_i != (unsigned)-1) { \
                if (tbl->item_storage[*p_item_i].hash == hash && key_equal_func(tbl->item_storage[*p_item_i].key, key)) { \
                    unsigned tmp_i = *p_item_i; \
                    *p_item_i = tbl->item_storage[tmp_i].next; \
                    function_prefix##_internal_dealloc_item(tbl, tmp_i); \
                    tbl->element_count--; \
                    return; \
                } \
                \
                p_item_i = &tbl->item_storage[*p_item_i].next; \
            } \
        } \
    } \
    \
    static inline int \
    function_prefix##_check_internal_sanity(TblTypeName *tbl) \
    { \
        size_t elcount = 0; \
        for (size_t i = 0; i < _hashtbl_size_map[tbl->table_size_idx]; ++i) { \
            unsigned item_i = tbl->hashtbl[i]; \
            while (item_i != (unsigned)-1) { \
                elcount++; \
                \
                if (function_prefix##_internal_index_for_hash(tbl, tbl->item_storage[item_i].hash) != i) \
                    return 0; \
                \
                item_i = tbl->item_storage[item_i].next; \
            } \
        } \
        \
        if (elcount != tbl->element_count) \
            return 0; \
        \
        if (_hashtbl_size_map[tbl->table_size_idx] - _hashtbl_size_map[tbl->table_size_idx]/4 < elcount \
                && tbl->table_size_idx < sizeof(_hashtbl_size_map)/sizeof(_hashtbl_size_map[0]) - 1) \
            return 0; \
        \
        return 1; \
    } \
    \
    typedef struct { \
        TblTypeName *tbl; \
        unsigned i; \
    } TblTypeName##_Iterator; \
    \
    static inline void \
    function_prefix##_iterator_init(TblTypeName *tbl, TblTypeName##_Iterator *it) { \
        it->tbl = tbl; \
        it->i = 0; \
        \
        while (it->i < it->tbl->item_storage_used && it->tbl->item_storage[it->i].next == (unsigned)-2) \
            it->i++; \
    } \
    \
    static inline int \
    function_prefix##_iterator_at_end(TblTypeName##_Iterator *it) { \
        return it->i >= it->tbl->item_storage_used; \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_iterator_item(TblTypeName##_Iterator *it) { \
        return &it->tbl->item_storage[it->i]; \
    } \
    \
    static inline void \
    function_prefix##_iterator_next(TblTypeName##_Iterator *it) { \
        while (it->i < it->tbl->item_storage_used) { \
            it->i++; \
            \
            if (it->i < it->tbl->item_storage_used && it->tbl->item_storage[it->i].next != (unsigned)-2) \
                return; \
        } \
    } \
    \
    static inline void \
    function_prefix##_iterator_delete(TblTypeName##_Iterator *it) { \
        if (it->i >= it->tbl->item_storage_used || it->tbl->item_storage[it->i].next == (unsigned)-2) \
            return; /*FIXME: complain about misuse */\
        \
        if (it->tbl->hashtbl) { \
            unsigned hashtbl_i = function_prefix##_internal_index_for_hash(it->tbl, it->tbl->item_storage[it->i].hash); \
            unsigned *p_item_i = &it->tbl->hashtbl[hashtbl_i]; \
            while (*p_item_i != (unsigned)-1) { \
                if (*p_item_i == it->i) { \
                    *p_item_i = it->tbl->item_storage[it->i].next; \
                } else { \
                    p_item_i = &it->tbl->item_storage[*p_item_i].next; \
                } \
            } \
        } \
        \
        function_prefix##_internal_dealloc_item(it->tbl, it->i); \
        it->tbl->element_count--; \
    } \
    \
    static inline void \
    function_prefix##_init_reserve(TblTypeName *tbl, unsigned num_items) \
    { \
        function_prefix##_init(tbl); \
        \
        unsigned target_table_size = 0; \
        while (target_table_size < sizeof(_hashtbl_size_map)/sizeof(_hashtbl_size_map[0])  \
                &&  num_items > _hashtbl_size_map[target_table_size] - _hashtbl_size_map[target_table_size]/4) \
            target_table_size++; \
        \
        tbl->table_size_idx = target_table_size; \
        function_prefix##_internal_recreate_hashtbl(tbl); \
        tbl->item_storage = (TblTypeName##_Item *)reallocarray(NULL, num_items, sizeof tbl->item_storage[0]); \
        if (tbl->item_storage) { \
            tbl->item_storage_allocated = num_items; \
            memset(tbl->item_storage, 0, (sizeof tbl->item_storage[0]) * num_items); \
        } \
    } \
    \




static const unsigned _hashtbl_size_map[] = {
    53,
    97,
    193,
    389,
    769,
    1543,
    3079,
    6151,
    12289,
    24593,
    49157,
    98317,
    196613,
    393241,
    786433,
    1572869,
    3145739,
    6291469,
    12582917,
    25165843,
    50331653,
    100663319,
    201326611,
    402653189,
    805306457,
    1610612741
};
