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

#define HASHTBL_DEFINE(TblTypeName, function_prefix, KeyType, ValueType, key_hash_func, key_equal_func) \
    HASHTBL_DEFINE_2(TblTypeName, function_prefix, KeyType, KeyType, /* nop */,(void), ValueType, ValueType, /* nop */, (void), key_hash_func, key_equal_func)

#define HASHTBL_DEFINE_2(TblTypeName, function_prefix, KeyType, ConstKeyType, key_dup_func, key_free_func, ValueType, ConstValueType, value_dup_func, value_free_func, key_hash_func, key_equal_func) \
    HASHTBL_DEFINE_3(TblTypeName, function_prefix, KeyType, ConstKeyType, key_dup_func, key_free_func, ValueType, ConstValueType, value_dup_func, value_free_func, key_hash_func, key_equal_func, realloc, free)

#define HASHTBL_DEFINE_3(TblTypeName, function_prefix, KeyType, ConstKeyType, key_dup_func, key_free_func, ValueType, ConstValueType, value_dup_func, value_free_func, key_hash_func, key_equal_func, realloc, free) \
    \
    typedef KeyType         TblTypeName##_Key; \
    typedef ConstKeyType    TblTypeName##_ConstKey; \
    typedef ValueType       TblTypeName##_Value; \
    typedef ConstValueType  TblTypeName##_ConstValue; \
    typedef struct TblTypeName##_Item { \
        unsigned hash; \
        TblTypeName##_Key   key; \
        TblTypeName##_Value value; \
        struct TblTypeName##_Item *next; \
    } TblTypeName##_Item; \
    typedef struct { \
        size_t element_count; \
        size_t table_size; \
        TblTypeName##_Item *items[]; \
    } *TblTypeName; \
    \
    static inline TblTypeName \
    function_prefix##_create(void) \
    { \
        /* FIXME! check overflow */ \
        TblTypeName rv = (TblTypeName)realloc(NULL, sizeof(*rv) + sizeof(rv->items[0]) * _hashtbl_size_map[0]); \
        if (rv) { \
            rv->element_count = 0; \
            rv->table_size = 0; \
            for (size_t i = 0; i < _hashtbl_size_map[rv->table_size]; ++i) \
                rv->items[i] = NULL; \
        } \
        return rv; \
    } \
    \
    static inline void \
    function_prefix##_free(TblTypeName tbl) \
    { \
        for (size_t i = 0; i < _hashtbl_size_map[tbl->table_size]; ++i) { \
            TblTypeName##_Item *tmp = tbl->items[i]; \
            TblTypeName##_Item *next = NULL; \
            while (tmp) { \
                next = tmp->next; \
                key_free_func(tmp->key); \
                value_free_func(tmp->value); \
                free(tmp); \
                tmp = next; \
            } \
        } \
        free(tbl); \
    } \
    static inline size_t \
    function_prefix##_index_for_hash(TblTypeName tbl, unsigned hash) \
    { \
        return ((size_t)hash * 11) % _hashtbl_size_map[tbl->table_size]; \
    } \
    \
    static inline TblTypeName##_Item ** \
    function_prefix##_lookup_p_by_hash(TblTypeName tbl, unsigned hash, TblTypeName##_ConstKey key) \
    { \
        size_t index =  function_prefix##_index_for_hash(tbl, hash); \
        TblTypeName##_Item **pitem = &tbl->items[index]; \
        while (*pitem) { \
            if ((*pitem)->hash == hash && key_equal_func((*pitem)->key, key)) \
                break; \
            \
            pitem = &(*pitem)->next; \
        } \
        return pitem; \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_lookup_by_hash(TblTypeName tbl, unsigned hash, TblTypeName##_ConstKey key) \
    { \
        return *function_prefix##_lookup_p_by_hash(tbl, hash, key); \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_lookup(TblTypeName tbl, TblTypeName##_ConstKey key) \
    { \
        return function_prefix##_lookup_by_hash(tbl, key_hash_func(key), key); \
    } \
    \
    static inline int \
    function_prefix##_contains(TblTypeName tbl, TblTypeName##_ConstKey key) \
    { \
        return function_prefix##_lookup(tbl, key) != NULL; \
    } \
    \
    static inline void \
    function_prefix##_reposition_items(TblTypeName tbl, size_t old_table_size) \
    { \
        for (size_t i = 0; i < _hashtbl_size_map[old_table_size]; ++i) { \
            TblTypeName##_Item *tmp = tbl->items[i]; \
            TblTypeName##_Item *next = NULL; \
            tbl->items[i] = NULL; \
            while (tmp) { \
                next = tmp->next; \
                \
                size_t pos = function_prefix##_index_for_hash(tbl, tmp->hash); \
                tmp->next = tbl->items[pos]; \
                tbl->items[pos] = tmp; \
                \
                tmp = next; \
            } \
        } \
    } \
    \
    static inline void \
    function_prefix##_auto_grow(TblTypeName *ptbl) \
    { \
        size_t target_table_size = (*ptbl)->table_size; \
        while (target_table_size < sizeof(_hashtbl_size_map)/sizeof(_hashtbl_size_map[0])  \
                &&  (*ptbl)->element_count > _hashtbl_size_map[target_table_size] - _hashtbl_size_map[target_table_size]/4) \
            target_table_size++; \
        \
        if (target_table_size != (*ptbl)->table_size) { \
            size_t old_table_size = (*ptbl)->table_size; \
            /* FIXME: handle overflow */ \
            TblTypeName tmp = (TblTypeName)realloc(*ptbl, sizeof(**ptbl) + sizeof(TblTypeName##_Item) * _hashtbl_size_map[target_table_size]); \
            if (tmp) { \
                *ptbl = tmp; \
                (*ptbl)->table_size = target_table_size; \
                for (size_t i = _hashtbl_size_map[old_table_size]; i < _hashtbl_size_map[target_table_size]; ++i) \
                    (*ptbl)->items[i] = NULL; \
                \
                function_prefix##_reposition_items(*ptbl, old_table_size); \
            } \
        } \
    } \
    \
    static inline void \
    function_prefix##_auto_shrink(TblTypeName *ptbl) \
    { \
        size_t target_table_size = (*ptbl)->table_size; \
        while (target_table_size > 0 && (*ptbl)->element_count < _hashtbl_size_map[target_table_size]/4) \
            target_table_size--; \
        \
        if (target_table_size != (*ptbl)->table_size) { \
            size_t old_table_size = (*ptbl)->table_size; \
            (*ptbl)->table_size = target_table_size; \
            function_prefix##_reposition_items(*ptbl, old_table_size); \
            /* FIXME: handle overflow */ \
            TblTypeName tmp = (TblTypeName)realloc(*ptbl, sizeof(**ptbl) + sizeof(TblTypeName##_Item) * _hashtbl_size_map[target_table_size]); \
            if (tmp) \
                *ptbl = tmp; \
        } \
    } \
    static inline TblTypeName##_Item * \
    function_prefix##_internal_add_item(TblTypeName *ptbl, TblTypeName##_Item **pitem, unsigned hash, TblTypeName##_ConstKey key) \
    { \
        TblTypeName##_Item *r = *pitem = (TblTypeName##_Item *)realloc(NULL, sizeof(TblTypeName##_Item)); \
        if (r) { \
            r->hash = hash; \
            r->key = key_dup_func(key); \
            memset(&r->value, 0, sizeof(r->value)); \
            r->next = NULL; \
            \
            (*ptbl)->element_count++; \
            function_prefix##_auto_grow(ptbl); \
        } \
        return r; \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_set_zero(TblTypeName *ptbl, TblTypeName##_ConstKey key) \
    { \
        unsigned hash = key_hash_func(key); \
        TblTypeName##_Item **pitem = function_prefix##_lookup_p_by_hash(*ptbl, hash, key); \
        if (*pitem) { \
            value_free_func((*pitem)->value); \
            memset(&(*pitem)->value, 0, sizeof((*pitem)->value)); \
            return *pitem; \
        } else { \
            return function_prefix##_internal_add_item(ptbl, pitem, hash, key); \
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
    static inline TblTypeName##_Item * \
    function_prefix##_lookup_or_insert_zero(TblTypeName *ptbl, TblTypeName##_ConstKey key) \
    { \
        unsigned hash = key_hash_func(key); \
        TblTypeName##_Item **pitem = function_prefix##_lookup_p_by_hash(*ptbl, hash, key); \
        if (*pitem) { \
            return *pitem; \
        } else { \
            return function_prefix##_internal_add_item(ptbl, pitem, hash, key); \
        } \
    } \
    \
    static inline TblTypeName##_Item * \
    function_prefix##_lookup_or_insert(TblTypeName *ptbl, TblTypeName##_ConstKey key, TblTypeName##_ConstValue value) \
    { \
        unsigned hash = key_hash_func(key); \
        TblTypeName##_Item **pitem = function_prefix##_lookup_p_by_hash(*ptbl, hash, key); \
        if (*pitem) { \
            return *pitem; \
        } else { \
            TblTypeName##_Item *i = function_prefix##_internal_add_item(ptbl, pitem, hash, key); \
            if (i) { \
                i->value = value_dup_func(value); \
            } \
            return i; \
        } \
    } \
    \
    static inline void \
    function_prefix##_remove(TblTypeName *ptbl, TblTypeName##_ConstKey key) \
    { \
        unsigned hash = key_hash_func(key); \
        TblTypeName##_Item **pitem = function_prefix##_lookup_p_by_hash(*ptbl, hash, key); \
        if (*pitem) { \
            TblTypeName##_Item *tmp = *pitem; \
            *pitem = (*pitem)->next; \
            key_free_func(tmp->key); \
            value_free_func(tmp->value); \
            free(tmp); \
            \
            (*ptbl)->element_count--; \
            function_prefix##_auto_shrink(ptbl); \
        } \
    } \
    \
    static inline int \
    function_prefix##_check_internal_sanity(TblTypeName tbl) \
    { \
        size_t elcount = 0; \
        for (size_t i = 0; i < _hashtbl_size_map[tbl->table_size]; ++i) { \
            TblTypeName##_Item *item = tbl->items[i]; \
            while (item) { \
                elcount++; \
                \
                if (function_prefix##_index_for_hash(tbl, item->hash) != i) \
                    return 0; \
                \
                item = item->next; \
            } \
        } \
        \
        if (elcount != tbl->element_count) \
            return 0; \
        \
        if (_hashtbl_size_map[tbl->table_size] - _hashtbl_size_map[tbl->table_size]/4 < elcount \
                && tbl->table_size < sizeof(_hashtbl_size_map)/sizeof(_hashtbl_size_map[0]) - 1) \
            return 0; \
        \
        if (_hashtbl_size_map[tbl->table_size]/4 > elcount && tbl->table_size != 0) \
            return 0; \
        \
        return 1; \
    } \
    \



static const size_t _hashtbl_size_map[] = {
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
