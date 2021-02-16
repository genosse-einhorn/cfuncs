/* Partially based on test/array-test.c from GLib (https://gitlab.gnome.org/GNOME/glib/)
 * GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *               1997-2018  the GLib Team and others
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */

#define _XOPEN_SOURCE 700

#undef NDEBUG
#include <assert.h>

#include <stdio.h>

#include "vector.h"


VECTOR_DEFINE(IntVector, int_vector, int);
VECTOR_DEFINE_2(StringVector, str_vector, char *, const char *, strdup, free);

static inline void
assert_int_vector_equal(IntVector a, const int *expected, size_t len)
{
    assert(int_vector_length(a) == len);

    for (size_t i = 0; i < len; ++i) {
        assert(a[i] == expected[i]);
    }
}

static void
test_append_val()
{
    IntVector v = NULL;

    for (int i = 0; i < 1000; ++i) {
        int_vector_push_back(&v, i);
    }

    assert(int_vector_length(v) == 1000);

    for (int i = 0; i < 1000; ++i) {
        assert(v[i] == i);
    }

    int_vector_clear(&v);
    assert(!v);
}

static void
test_prepend_val()
{
    IntVector v = NULL;

    for (int i = 0; i < 100; i++)
        int_vector_insert(&v, 0, i);

    assert(int_vector_length(v) == 100);

    for (int i = 0; i < 100; i++)
        assert(v[i] == 100 - i - 1);

    int_vector_clear(&v);
    assert(!v);
}

static void
test_prepend_multi()
{
    IntVector v = NULL;
    const int vals[] = { 0, 1, 2, 3, 4 };
    const int expected_vals1[] = { 0, 1 };
    const int expected_vals2[] = { 2, 0, 1 };
    const int expected_vals3[] = { 3, 4, 2, 0, 1 };

    // two values
    int_vector_insert_multi(&v, 0, 2, vals);
    assert_int_vector_equal(v, expected_vals1, sizeof(expected_vals1)/sizeof(expected_vals1[0]));

    // single value
    int_vector_insert_multi(&v, 0, 1, &vals[2]);
    assert_int_vector_equal(v, expected_vals2, sizeof(expected_vals2)/sizeof(expected_vals2[0]));

    // now some more
    int_vector_insert_multi(&v, 0, 2, &vals[3]);
    assert_int_vector_equal(v, expected_vals3, sizeof(expected_vals3)/sizeof(expected_vals3[0]));

    // prepend zero values
    int_vector_insert_multi(&v, 0, 0, vals);
    assert_int_vector_equal(v, expected_vals3, sizeof(expected_vals3)/sizeof(expected_vals3[0]));

    // prepend no values with NULL pointer
    int_vector_insert_multi(&v, 0, 0, NULL);
    assert_int_vector_equal(v, expected_vals3, sizeof(expected_vals3)/sizeof(expected_vals3[0]));

    int_vector_clear(&v);
    assert(!v);
}

static void
test_insert_multi()
{
    IntVector v = NULL;
    const int vals[] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    const int expected_vals1[] = { 0, 1 };
    const int expected_vals2[] = { 0, 2, 3, 1 };
    const int expected_vals3[] = { 0, 2, 3, 1, 4 };
    const int expected_vals4[] = { 5, 0, 2, 3, 1, 4 };

    /* Insert several values at the beginning. */
    int_vector_insert_multi(&v, 0, 2, vals);
    assert_int_vector_equal(v, expected_vals1, sizeof(expected_vals1)/sizeof(expected_vals1[0]));

    /* Insert some more part-way through. */
    int_vector_insert_multi(&v, 1, 2, &vals[2]);
    assert_int_vector_equal(v, expected_vals2, sizeof(expected_vals2)/sizeof(expected_vals2[0]));

    /* And at the end. */
    int_vector_insert(&v, int_vector_length(v), vals[4]);
    assert_int_vector_equal(v, expected_vals3, sizeof(expected_vals3)/sizeof(expected_vals3[0]));

    /* Then back at the beginning again. */
    int_vector_insert(&v, 0, vals[5]);
    assert_int_vector_equal(v, expected_vals4, sizeof(expected_vals4)/sizeof(expected_vals4[0]));

    /* Insert zero elements. */
    int_vector_insert_multi(&v, 0, 0, vals);
    assert_int_vector_equal(v, expected_vals4, sizeof(expected_vals4)/sizeof(expected_vals4[0]));

    /* Insert zero elements with a %NULL pointer. */
    int_vector_insert_multi(&v, 4, 0, NULL);
    assert_int_vector_equal(v, expected_vals4, sizeof(expected_vals4)/sizeof(expected_vals4[0]));

    int_vector_clear(&v);
}


static void
test_remove()
{
    vector_autoclear(IntVector) v = NULL;

    for (int i = 0; i < 100; i++)
        int_vector_push_back(&v, i);

    assert(int_vector_length(v) == 100);

    int_vector_remove(&v, 31, 4);

    assert(int_vector_length(v) == 96);

    int prev = -1;
    for (int i = 0; i < (int)int_vector_length(v); i++) {
        int cur = v[i];
        assert(cur < 31 || cur > 34);
        assert(prev < cur);
        prev = cur;
    }

    /* Ensure the entire array can be cleared, even when empty. */
    int_vector_remove(&v, 0, int_vector_length(v));
    assert(int_vector_length(v) == 0);

    int_vector_remove(&v, 0, int_vector_length(v));
    assert(int_vector_length(v) == 0);
}

static void
test_overflow()
{
    IntVector v = NULL;

    int_vector_reserve(&v, SIZE_MAX/sizeof(int));
    assert(!v);

    int_vector_push_back(&v, 1);
    int_vector_push_back(&v, 2);
    int_vector_push_back(&v, 3);
    assert(int_vector_length(v) == 3);
    assert(int_vector_capacity(v) >= 3);

    // failing to allocate more keeps existing elements intact
    int_vector_reserve(&v, SIZE_MAX/sizeof(int));
    assert(int_vector_length(v) == 3);
    assert(int_vector_capacity(v) >= 3);

    assert_int_vector_equal(v, (const int[]){ 1, 2, 3 }, 3);

    int_vector_clear(&v);
}

static void
test_push_pop()
{
    IntVector v = NULL;

    for (int i = 0; i < 1000; ++i) {
        int_vector_push_back(&v, i);
    }

    assert(int_vector_length(v) == 1000);

    for (int i = 0; i < 1000; ++i) {
        assert(v[i] == i);
    }

    int i = int_vector_pop_back(&v);
    assert(i == 999);
    assert(int_vector_length(v) == 999);

    int_vector_clear(&v);
    assert(!v);
}

static void
test_str()
{
    StringVector v = NULL;

    str_vector_push_back(&v, "Hello World!");
    str_vector_push_back(&v, "Goodbye, World!");

    assert(str_vector_length(v) == 2);
    assert(!strcmp("Hello World!", v[0]));

    char *g = str_vector_pop_back(&v);
    assert(!strcmp("Goodbye, World!", g));
    free(g);
    assert(str_vector_length(v) == 1);

    str_vector_emplace_back(&v, strdup("Goodbye, World, again!"));

    str_vector_clear(&v);
}

static void
test_assign(void)
{
    IntVector v1 = NULL;
    IntVector v2 = NULL;

    int_vector_push_back(&v2, 1);
    int_vector_push_back(&v2, 2);
    int_vector_push_back(&v2, 3);

    int_vector_assign(&v1, v2);

    assert_int_vector_equal(v1, (const int[]){ 1, 2, 3 }, 3);

    int_vector_push_back(&v1, 4);
    int_vector_push_back(&v1, 5);

    int_vector_assign(&v2, v1);

    assert_int_vector_equal(v2, (const int[]){ 1, 2, 3, 4, 5 }, 5);

    int_vector_clear(&v1);
    int_vector_clear(&v2);
}

int main(void)
{
    test_append_val();
    test_prepend_val();
    test_prepend_multi();
    test_insert_multi();
    test_remove();
    test_overflow();
    test_push_pop();
    test_str();
    test_assign();

    return 0;
}
