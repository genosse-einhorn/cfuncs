#pragma once

/*
 * intrusive-list.h - intrusive doubly linked list example code
 *
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

#include <stddef.h>

typedef struct list_link {
    struct list_link *next;
    struct list_link *prev;
} list_link;

static inline void
_list_hookup_new_element(list_link *before, list_link *after, list_link *newel)
{
    before->next = newel;
    after->prev = newel;
    newel->next = after;
    newel->prev = before;
}

#define DEFINE_INTRUSIVE_LIST(ContainerType, link_member, ListType, func_prefix) \
    \
    typedef struct ListType { \
        list_link link; \
    } ListType; \
    \
    static inline void \
    func_prefix##_init(ListType *list) { \
        list->link.next = &list->link; \
        list->link.prev = &list->link; \
    } \
    static inline int \
    func_prefix##_is_empty(ListType *list) { \
        return list->link.next == &list->link; \
    } \
    \
    static inline ContainerType * \
    func_prefix##_first(ListType *list) { \
        if (list->link.next == &list->link) { \
            return NULL; \
        } else { \
            return (ContainerType *)((char *)list->link.next - offsetof(ContainerType, link_member)); \
        } \
    } \
    \
    static inline ContainerType * \
    func_prefix##_last(ListType *list) { \
        if (list->link.prev == &list->link) { \
            return NULL; \
        } else { \
            return (ContainerType *)((char *)list->link.prev - offsetof(ContainerType, link_member)); \
        } \
    } \
    \
    static inline ContainerType * \
    func_prefix##_next(ListType *list, ContainerType *i) { \
        if (i->link_member.next == &list->link) { \
            return NULL; \
        } else { \
            return (ContainerType *)((char *)i->link_member.next - offsetof(ContainerType, link_member)); \
        } \
    } \
    \
    static inline ContainerType * \
    func_prefix##_prev(ListType *list, ContainerType *i) { \
        if (i->link_member.prev == &list->link) { \
            return NULL; \
        } else { \
            return (ContainerType *)((char *)i->link_member.prev - offsetof(ContainerType, link_member)); \
        } \
    } \
    \
    static inline void \
    func_prefix##_insert_front(ListType *list, ContainerType *newel) { \
        _list_hookup_new_element(&list->link, list->link.next, &newel->link_member); \
    } \
    \
    static inline void \
    func_prefix##_insert_back(ListType *list, ContainerType *newel) { \
        _list_hookup_new_element(list->link.prev, &list->link, &newel->link_member); \
    } \
    \
    static inline void \
    func_prefix##_insert_after(ContainerType *existing, ContainerType *newel) { \
        _list_hookup_new_element(&existing->link_member, existing->link_member.next, &newel->link_member); \
    } \
    \
    static inline void \
    func_prefix##_insert_before(ContainerType *existing, ContainerType *newel) { \
        _list_hookup_new_element(existing->link_member.prev, &existing->link_member, &newel->link_member); \
    } \
    \
    static inline void \
    func_prefix##_remove(ContainerType *el) { \
        el->link_member.prev->next = el->link_member.next; \
        el->link_member.next->prev = el->link_member.prev; \
        el->link_member.next = el->link_member.prev = NULL; \
    } \
    \
    static inline int \
    func_prefix##_iter(ListType *list, ContainerType **pi) { \
        if (*pi == NULL) { \
            *pi = func_prefix##_first(list); \
        } else { \
            *pi = func_prefix##_next(list, *pi); \
        } \
        return *pi != NULL; \
    } \
    \
    static inline int \
    func_prefix##_riter(ListType *list, ContainerType **pi) { \
        if (*pi == NULL) { \
            *pi = func_prefix##_last(list); \
        } else { \
            *pi = func_prefix##_prev(list, *pi); \
        } \
        return *pi != NULL; \
    } \
    \
    static inline void \
    func_prefix##_clear(ListType *list, void (*delete_func)(ContainerType *)) { \
        ContainerType *e = func_prefix##_first(list); \
        while (e) { \
            ContainerType *n = func_prefix##_next(list, e); \
            delete_func(e); \
            e = n; \
        } \
        func_prefix##_init(list); \
    } \
    \
    static inline size_t \
    func_prefix##_length(ListType *list) { \
        size_t i = 0; \
        for (list_link *p = list->link.next; p != &list->link; p = p->next) \
            ++i; \
        return i; \
    }

