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

#include "str.h"
#include "vector.h"

VECTOR_DEFINE_3(StrList, str_list, char *, const char *, str_dup, str_free, _str_xrealloc, free);


static inline void
str_list_add(StrList *l, const char *s)
{
    str_list_push_back(l, s);
}

static inline void
str_list_add_buf(StrList *l, const char *buf, int len)
{
    char *s = str_dup_buf(buf, len);
    str_list_emplace_back(l, s);
}

static inline void
str_list_set(StrList *l, size_t i, const char *s)
{
    if (i >= str_list_length(*l)) {
        str_list_add(l, s);
    } else {
        str_assign(&(*l)[i], s);
    }
}

static inline void
str_list_set_buf(StrList *l, size_t i, const char *buf, int len)
{
    if (i >= str_list_length(*l)) {
        str_list_add_buf(l, buf, len);
    } else {
        str_assign_buf(&(*l)[i], buf, len);
    }
}

static inline void
str_list_assign_strv(StrList *l, char **strv)
{
    assert(l);

    size_t c = 0;
    while (*strv) {
        str_list_set(l, c, *strv);
        strv++;
        c++;
    }

    str_list_resize_zero(l, c);
}

static inline StrList
str_list_from_strv(char **strv)
{
    StrList list = NULL;
    str_list_assign_strv(&list, strv);
    return list;
}

static inline void
str_list_assign_doublenull(StrList *l, const char *s)
{
    assert(l);

    size_t c = 0;
    while (s && *s) {
        int len = str_length(s);
        str_list_set_buf(l, c, s, len);
        s = s + len + 1;
        c++;
    }

    str_list_resize_zero(l, c);
}

static inline StrList
str_list_from_doublenull(const char *s)
{
    StrList list = NULL;
    str_list_assign_doublenull(&list, s);
    return list;
}

static inline void
str_assign_list_as_doublenull(char **out, StrList l)
{
    int size = 0;
    for (size_t i = 0; i < str_list_length(l); ++i) {
        size += str_length(l[i]) + 1;
    }

    str_realloc(out, size);
    char *p = *out;
    for (size_t i = 0; i < str_list_length(l); ++i) {
        const char *s = l[i] ? l[i] : "";
        while (*s)
            *p++ = *s++;

        *p++ = '\0';
    }
}

static inline char *
str_list_as_doublenull(StrList l)
{
    char *r = NULL;
    str_assign_list_as_doublenull(&r, l);
    return r;
}

static inline void
str_list_assign_split(StrList *list, const char *str, const char *separator)
{
    int len = str_length(str);
    int sep_len = str_length(separator);
    if (len == 0) {
        // empty string -> empty list
        str_list_resize_zero(list, 0);
    } else if (sep_len == 0) {
        // split every char in its own
        str_list_resize_zero(list, (size_t)len);
        for (int i = 0; i < len; ++i) {
            str_assign_buf(&(*list)[i], &str[i], 1);
        }
    } else {
        // split at each separator
        int i = 0;
        size_t count = 0;
        while (i < len) {
            int j = str_index_of_buf(&str[i], len - i, separator, sep_len);
            if (j < 0) {
                j = len;
            } else {
                j = j + i;
            }

            str_list_set_buf(list, count, &str[i], j - i);
            i = j + sep_len;
            ++count;
        }

        str_list_resize_zero(list, count);
    }
}

static inline StrList
str_split(const char *str, const char *separator)
{
    StrList r = NULL;
    str_list_assign_split(&r, str, separator);
    return r;
}

static inline void
str_assign_joined(char **out, StrList l, const char *separator)
{
    if (str_list_length(l) < 1) {
        str_clear(out);
    } else if (str_list_length(l) == 1) {
        str_assign(out, l[0]);
    } else {
        int length = 0;
        for (size_t i = 0; i < str_list_length(l); ++i) {
            length += str_length(l[i]);
        }

        length += (int)(str_list_length(l) - 1) * str_length(separator);

        str_realloc(out, length);

        char *p = *out;
        for (size_t i = 0; i < str_list_length(l); ++i) {
            if (i != 0) {
                const char *s = separator ? separator : "";
                while (*s)
                    *p++ = *s++;
            }

            const char *s = l[i] ? l[i] : "";
            while (*s)
                *p++ = *s++;
        }
    }
}

static inline char *
str_joined(StrList l, const char *separator)
{
    char *r = NULL;
    str_assign_joined(&r, l, separator);
    return r;
}

static inline size_t
str_list_env_index(StrList l, const char *key, char **pvalue)
{
    key = key ? key : "";

    for (size_t i = 0; i < str_list_length(l); ++i) {
        if (!l[i])
            continue;

        size_t j = 0;
        while (key[j] && (_str_ascii_upper(l[i][j]) == _str_ascii_upper(key[j])))
            j++;

        if (!key[j] && l[i][j] == '=') {
            // found it!
            if (pvalue)
                *pvalue = &l[i][j+1];

            return i;
        }
    }

    return (size_t)-1;
}

static inline void
str_assign_list_env_value(char **out, StrList l, const char *key)
{
    char *val = NULL;
    size_t i = str_list_env_index(l, key, &val);
    if (i != (size_t)-1) {
        str_assign(out, val);
    } else {
        str_clear(out);
    }
}

static inline char *
str_list_env_value(StrList l, const char *key)
{
    char *r = NULL;
    str_assign_list_env_value(&r, l, key);
    return r;
}

static inline void
str_list_set_env_value(StrList *plist, const char *key, const char *value)
{
    key = key ? key : "";
    value = value ? value : "";

    int keylen = str_length(key);
    int valuelen = str_length(value);

    size_t index = str_list_env_index(*plist, key, NULL);
    if (index != (size_t)-1) {
        str_realloc(&(*plist)[index], keylen + valuelen + 1);
        memcpy((*plist)[index] + keylen + 1, value, (size_t)valuelen);
    } else {
        char *r = NULL;
        str_realloc(&r, keylen + valuelen + 1);
        memcpy(r, key, (size_t)keylen);
        r[keylen] = '=';
        memcpy(r + keylen + 1, value, (size_t)valuelen);
        str_list_emplace_back(plist, r);
    }
}

static inline void
str_list_unset_env_value(StrList *plist, const char *key)
{
    key = key ? key : "";

    size_t index = str_list_env_index(*plist, key, NULL);
    if (index != (size_t)-1) {
        str_list_remove(plist, index, 1);
    }
}

static inline void
str_list_assign_lines(StrList *plist, const char *lines)
{
    size_t c = 0;
    for (;;) {
        size_t i = 0;
        while (lines[i] && lines[i] != '\n')
            ++i;

        if (lines[i] == '\n' && i > 0 && lines[i-1] == '\r')
            str_list_set_buf(plist, c++, lines, (int)i-1);
        else
            str_list_set_buf(plist, c++, lines, (int)i);

        if (!lines[i])
            break;

        lines = lines + i + 1;
    }

    str_list_resize_zero(plist, c);
}

static inline StrList
str_list_from_lines(const char *lines)
{
    StrList r = NULL;
    str_list_assign_lines(&r, lines);
    return r;
}
