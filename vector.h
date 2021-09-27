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


/*
 * DOCUMENTATION
 * =============
 *
 * This is a set of macros and inline functions for dynamically resizable arrays in C.
 * You first use one of the following macros to define the vector type and operations:
 *
 * VECTOR_DEFINE(VectorType, function_prefix, ElementType)
 * VECTOR_DEFINE_2(VectorType, function_prefix, ElementType, ConstElementType, element_dup_func, element_free_func)
 * VECTOR_DEFINE_3(VectorType, function_prefix, ElementType, ConstElementType, element_dup_func, element_free_func, realloc, free)
 *
 * VectorType           = the name of the typedef for the vector.
 * function_prefix      = a prefix for all functions operating on this vector type.
 * ElementType          = the type of elements contained in the vector.
 * ConstElementType     = the type of unowned elements that can be fed into element_dup_func().
 *                        Note that ElementType must be assignable to ConstElementType.
 *                        Example: if ElementType=char * and element_dup_func=strdup,
 *                        you might want to make ConstElementType=const char *.
 * element_dup_func     = a function that takes a ConstElementType and returns a duplicated
 *                        instance of an ElementType. Called when elements need to be copied.
 * element_free_func    = a void-returning function that takes an ElementType. It is called to
 *                        free elements removed from the vector.
 * realloc,free         = memory allocation functions compatible to realloc(3) and free(3).
 *
 *
 * These macros will define some typedefs like
 *      typedef ElementType VectorType__Element;
 *      typedef ConstElementType VectorType__ConstElement;
 *      typedef VectorType__Element *VectorType;
 *      typedef struct {
 *          size_t capacity;
 *          size_t length;
 *          VectorType__Element data[];
 *      } *VectorType__Impl;
 *
 * You then declare and initialize a vector variable like this:
 *      VectorType vector = NULL;
 *
 * The vector header as well as all the data is allocated in one contiguous memory block, and
 * realloc(3)'ed as needed to grow the vector. The variable of type VectorType will
 * point to the first element stored into the vector. You can access the i-th element
 * in vector ‘vector’ using vector[i].
 *
 * All of the following functions will treat a NULL as if it was an empty vector. Functions 
 * may fail if and only if they failed to allocate memory. If you want to get rid of this 
 * error case, you can supply a custom realloc(3) replacement that never fails.
 *
 * size_t function_prefix_length(VectorType vec)
 *      The length of the given vector.
 *
 * size_t function_prefix_capacity(VectorType vec)
 *      The capacity (= max elements for which memory has been allocated)
 *
 * void function_prefix_clear(VectorType *pvec)
 *      Remove all elements from the vector and free all allocated memory.
 *      The vector will be turned into a NULL vector.
 *
 * void function_prefix_assign(VectorType *target, VectorType source)
 *      Clears the target vector and then copies all elements from source to target vector.
 *      element_dup_func and element_free_func will be called as needed.
 *
 * void function_prefix_swap(VectorType *a, VectorType *b)
 *      Swaps the two vectors.
 *
 * ElementType *function_prefix_insert(VectorType *pvec, size_t index, ConstElementType el)
 *      Insert ‘element_dup_func(el)’ into the vector at the given index.
 *      Returns a pointer to the newly inserted element in the vector, or NULL on failure.
 *
 * ElementType *function_prefix_insert_multi(VectorType *pvec, size_t index, size_t count, ConstElementType *pel)
 *      Insert `count` elements `pel` into the vector `pvec` at `index`. If specified,
 *      element_dup_func is called for each element.
 *      Returns a pointer to the first newly inserted element in the vector, or NULL on failure.
 *
 * ElementType *function_prefix_emplace(VectorType *pvec, size_t index, ElementType el)
 *      Insert el into the vector at the given index, without calling element_dup_func()
 *      Returns a pointer to the newly inserted element in the vector, or NULL on failure.
 *
 * ElementType *function_prefix_insert_multi(VectorType *pvec, size_t index, size_t count, ElementType *pel)
 *      Insert `count` elements `pel` into the vector `pvec` at `index`, without calling element_dup_func.s
 *      Returns a pointer to the first newly inserted element in the vector, or NULL on failure.
 *
 * void function_prefix_remove(VectorType *pvec, size_t index, size_t count)
 *      Remove `count` elements from the vector `pvec` at index `index`.
 *      If specified, element_free_func() is called on each removed element.
 *
 * ElementType *function_prefix_push_back(VectorType *pvec, ConstElementType el)
 *      Insert one element at the end of the vector, calling element_dup_func() if specified.
 *      Returns a pointer to the newly inserted element in the vector, or NULL on failure.
 *
 * ElementType *function_prefix_emplace_back(VectorType *pvec, ElementType el)
 *      Insert one element at the end of the vector WITHOUT calling element_dup_func().
 *      Returns a pointer to the newly inserted element in the vector, or NULL on failure.
 *
 * ElementType function_prefix_pop_back(VectorType *pvec)
 *      Removes and returns the last element from the vector.
 *
 * VectorType function_prefix_reserve(VectorType *pvec, size_t count)
 *      Ensure that the vector's capacity is at least `count`. Returns the possibly reallocated vector
 *      (== *pvec), or NULL on failure (NOTE: you should only use the return value to check for NULL).
 */

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>

static inline size_t
_vector_next_capacity(size_t current_capacity)
{
    if (current_capacity < 2) {
        return 2;
    } else if (SIZE_MAX - current_capacity > current_capacity/2) {
        return current_capacity + current_capacity / 2;
    } else {
        return SIZE_MAX;
    }
}

static inline void *
_vector_reallocarray_with_header(void *(*realloc)(void *, size_t), void *ptr, size_t header, size_t nmemb, size_t size)
{
    if ((nmemb > 0) && ((SIZE_MAX - header) / nmemb < size)) {
        errno = ENOMEM;
        return NULL;
    } else {
        return realloc(ptr, header + nmemb * size);
    }
}

#define _VECTOR_TYPEDEFS(VectorType, ElementType, ConstElementType) \
    typedef ElementType _##VectorType##__Element; \
    typedef ConstElementType _##VectorType##__ConstElement; \
    typedef struct _##VectorType##__Impl { \
            size_t capacity; \
            size_t length; \
            _##VectorType##__Element data[]; \
        } _##VectorType##__Impl; \
    typedef _##VectorType##__Element *VectorType;

#define _VECTOR_FUNCTIONS(VectorType, function_prefix, el_dup_func, el_free_func, realloc, free) \
    static inline _##VectorType##__Impl * \
    _##function_prefix##_impl(VectorType v) \
    { \
        return (_##VectorType##__Impl *)((char *)v - offsetof(_##VectorType##__Impl, data)); \
    } \
    static inline size_t \
    function_prefix##_length(VectorType v) \
    { \
        return v ? _##function_prefix##_impl(v)->length : 0; \
    } \
    \
    static inline size_t \
    function_prefix##_capacity(VectorType v) \
    { \
        return v ? _##function_prefix##_impl(v)->capacity : 0; \
    } \
    \
    static inline VectorType \
    function_prefix##_reserve(VectorType *pvec, size_t count) \
    {   \
        if (*pvec) { \
            if (_##function_prefix##_impl(*pvec)->capacity >= count) { \
                return *pvec; \
            } else { \
                _##VectorType##__Impl *newv = (_##VectorType##__Impl *)_vector_reallocarray_with_header( \
                    realloc,  \
                    _##function_prefix##_impl(*pvec), \
                    sizeof(*newv), count, sizeof(_##VectorType##__Element)); \
                if (!newv) { \
                    return NULL; \
                } \
                memset(&newv->data[newv->capacity], 0, (count - newv->capacity) * sizeof(newv->data[0])); \
                newv->capacity = count; \
                *pvec = &newv->data[0]; \
                return &newv->data[0]; \
            } \
        } else { \
            _##VectorType##__Impl *newv = (_##VectorType##__Impl *)_vector_reallocarray_with_header( \
                    realloc,  \
                    NULL, \
                    sizeof(*newv), count, sizeof(_##VectorType##__Element)); \
            if (!newv) { \
                return NULL; \
            } \
            newv->capacity = count; \
            newv->length = 0; \
            memset(&newv->data[0], 0, newv->capacity * sizeof(newv->data[0])); \
            *pvec = &newv->data[0]; \
            return &newv->data[0]; \
        } \
    } \
    \
    static inline VectorType \
    _##function_prefix##_auto_grow(VectorType *pvec, size_t newcount) \
    { \
        size_t c = function_prefix##_capacity(*pvec); \
        while (c < newcount) { \
            c = _vector_next_capacity(c); \
        } \
        return function_prefix##_reserve(pvec, c); \
    } \
    \
    static inline _##VectorType##__Element * \
    function_prefix##_push_back(VectorType *pvec, _##VectorType##__ConstElement el) \
    { \
        VectorType v = _##function_prefix##_auto_grow(pvec, function_prefix##_length(*pvec) + 1); \
        if (!v) { \
            return NULL; \
        } \
        _##VectorType##__Impl *vi = _##function_prefix##_impl(v); \
        size_t index = vi->length; \
        vi->length++; \
        vi->data[index] = el_dup_func(el); \
        return &vi->data[index]; \
    } \
    \
    static inline _##VectorType##__Element * \
    function_prefix##_emplace_back(VectorType *pvec, _##VectorType##__Element el) \
    { \
        VectorType v = _##function_prefix##_auto_grow(pvec, function_prefix##_length(*pvec) + 1); \
        if (!v) { \
            return NULL; \
        } \
        _##VectorType##__Impl *vi = _##function_prefix##_impl(v); \
        size_t index = vi->length; \
        vi->length++; \
        vi->data[index] = el; \
        return &vi->data[index]; \
    } \
    \
    static inline _##VectorType##__Element * \
    function_prefix##_insert_zero(VectorType *pvec, size_t index, size_t count) \
    { \
        assert(index <= function_prefix##_length(*pvec)); \
        \
        VectorType v = _##function_prefix##_auto_grow(pvec, function_prefix##_length(*pvec) + count); \
        if (!v) { \
            return NULL; \
        } \
        _##VectorType##__Impl *vi = _##function_prefix##_impl(*pvec); \
        memmove(&vi->data[index+count], &vi->data[index], sizeof(_##VectorType##__Element)*(vi->length - index)); \
        memset(&vi->data[index], 0, sizeof(_##VectorType##__Element)*count); \
        vi->length += count; \
        return &vi->data[index]; \
    } \
    \
    static inline _##VectorType##__Element * \
    function_prefix##_insert_multi(VectorType *pvec, size_t index, size_t count, _##VectorType##__ConstElement const *els) \
    { \
        _##VectorType##__Element *target = function_prefix##_insert_zero(pvec, index, count); \
        if (!target) { \
            return NULL; \
        } \
        \
        for (size_t i = 0; i < count; ++i) { \
            target[i] = el_dup_func(els[i]); \
        } \
        return target; \
    } \
    static inline _##VectorType##__Element * \
    function_prefix##_emplace_multi(VectorType *pvec, size_t index, size_t count, _##VectorType##__Element const *els) \
    { \
        _##VectorType##__Element *target = function_prefix##_insert_zero(pvec, index, count); \
        if (!target) { \
            return NULL; \
        } \
        \
        for (size_t i = 0; i < count; ++i) { \
            target[i] = els[i]; \
        } \
        return target; \
    } \
    \
    static inline _##VectorType##__Element * \
    function_prefix##_insert(VectorType *pvec, size_t index, _##VectorType##__ConstElement el) \
    { \
        return function_prefix##_insert_multi(pvec, index, 1, &el); \
    } \
    \
    static inline _##VectorType##__Element * \
    function_prefix##_emplace(VectorType *pvec, size_t index, _##VectorType##__Element el) \
    { \
        return function_prefix##_emplace_multi(pvec, index, 1, &el); \
    } \
    \
    static inline void \
    function_prefix##_remove(VectorType *pvec, size_t index, size_t count) \
    { \
        if (count < 1) \
            return; \
        \
        _##VectorType##__Impl *vi = _##function_prefix##_impl(*pvec); \
        for (size_t i = 0; i < count; ++i) { \
            (void)el_free_func(vi->data[index+i]); \
        } \
        memmove(&vi->data[index], &vi->data[index+count], (vi->length - index - count)*sizeof(_##VectorType##__Element)); \
        vi->length -= count; \
        memset(&vi->data[vi->length], 0, sizeof(_##VectorType##__Element)*count); \
    } \
    \
    static inline _##VectorType##__Element \
    function_prefix##_pop_back(VectorType *pvec) \
    { \
        _##VectorType##__Impl *vi = _##function_prefix##_impl(*pvec); \
        vi->length--; \
        _##VectorType##__Element rv = vi->data[vi->length]; \
        memset(&vi->data[vi->length], 0, sizeof(_##VectorType##__Element)); \
        return rv; \
    } \
    \
    static inline _##VectorType##__Element \
    function_prefix##_item(VectorType vec, size_t index) \
    { \
        return el_dup_func(vec[index]); \
    } \
    \
    static inline void \
    function_prefix##_clear(VectorType *pvec) \
    { \
        if (!*pvec) { \
            return; \
        } \
        \
        _##VectorType##__Impl *vi = _##function_prefix##_impl(*pvec); \
        for (size_t i = 0; i < vi->length; ++i) { \
            (void)el_free_func(vi->data[i]); \
        } \
        free(vi); \
        *pvec = NULL; \
    }\
    \
    static inline void \
    function_prefix##_assign(VectorType *ptarget, VectorType source) \
    { \
        if (function_prefix##_length(*ptarget)) { \
            function_prefix##_remove(ptarget, 0, function_prefix##_length(*ptarget)); \
        } \
        if (function_prefix##_length(source)) { \
            function_prefix##_insert_zero(ptarget, 0, function_prefix##_length(source)); \
            for (size_t i = 0; i < function_prefix##_length(source); ++i) { \
                (*ptarget)[i] = el_dup_func(source[i]); \
            } \
        } \
    } \
    \
    static inline void \
    function_prefix##_swap(VectorType *a, VectorType *b) \
    { \
        VectorType tmp = *a; \
        *a = *b; \
        *b = tmp; \
    } \
    static inline void \
    function_prefix##_resize_zero(VectorType *a, size_t length)  \
    { \
        if (length > function_prefix##_length(*a)) { \
            function_prefix##_insert_zero(a, function_prefix##_length(*a), length - function_prefix##_length(*a)); \
        } else if (function_prefix##_length(*a) > length) { \
            function_prefix##_remove(a, length, function_prefix##_length(*a) - length); \
        } \
    } \
    \
    static inline void \
    _##VectorType##_autocleanup_func(VectorType *pvec) \
    { \
        function_prefix##_clear(pvec); \
    } \


#ifdef __GNUC__
#   define vector_autoclear(VectorType) __attribute__((__cleanup__(_##VectorType##_autocleanup_func))) VectorType
/* clang complains if the macro-generated functions are not used */
#   define _VECTOR_DEF_BEGIN \
        _Pragma("GCC diagnostic push") \
        _Pragma("GCC diagnostic ignored \"-Wunused-function\"")
#   define _VECTOR_DEF_END \
        _Pragma("GCC diagnostic pop")
#else
#   define _VECTOR_DEF_BEGIN
#   define _VECTOR_DEF_END
#endif

#define _vector_noop(x) do { } while (0)

#define VECTOR_DEFINE_3(VectorType, function_prefix, ElementType, ConstElementType, el_dup_func, el_free_func, realloc, free) \
    _VECTOR_DEF_BEGIN \
    _VECTOR_TYPEDEFS(VectorType, ElementType, ConstElementType) \
    _VECTOR_FUNCTIONS(VectorType, function_prefix, el_dup_func, el_free_func, realloc, free) \
    _VECTOR_DEF_END

#define VECTOR_DEFINE_2(VectorType, function_prefix, ElementType, ConstElementType, el_dup_func, el_free_func) \
    VECTOR_DEFINE_3(VectorType, function_prefix, ElementType, ConstElementType, el_dup_func, el_free_func, realloc, free)


#define VECTOR_DEFINE(VectorType, function_prefix, ElementType) \
    VECTOR_DEFINE_2(VectorType, function_prefix, ElementType, ElementType,,)


