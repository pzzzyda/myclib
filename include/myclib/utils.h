#ifndef MYCLIB_UTILS_H
#define MYCLIB_UTILS_H

#include <stdint.h>
#include <stddef.h>

#define __MC_JOIN_UNDERSCORE0()
#define __MC_JOIN_UNDERSCORE1(a) a
#define __MC_JOIN_UNDERSCORE2(a, b) a##_##b
#define __MC_JOIN_UNDERSCORE3(a, b, c) a##_##b##_##c
#define __MC_JOIN_UNDERSCORE4(a, b, c, d) a##_##b##_##c##_##d
#define __MC_JOIN_UNDERSCORE5(a, b, c, d, e) a##_##b##_##c##_##d##_##e
#define __MC_JOIN_UNDERSCORE6(a, b, c, d, e, f) a##_##b##_##c##_##d##_##e##_##f
#define __MC_GET_JOIN_UNDERSCORE_MACRO(a, b, c, d, e, f, name, ...) name
#define MC_JOIN_UNDERSCORE(...)                                                \
    __MC_GET_JOIN_UNDERSCORE_MACRO(                                            \
        __VA_ARGS__, __MC_JOIN_UNDERSCORE6, __MC_JOIN_UNDERSCORE5,             \
        __MC_JOIN_UNDERSCORE4, __MC_JOIN_UNDERSCORE3, __MC_JOIN_UNDERSCORE2,   \
        __MC_JOIN_UNDERSCORE1, __MC_JOIN_UNDERSCORE0)                          \
    (__VA_ARGS__)

static inline void *mc_ptr_add(void *ptr, size_t offset)
{
    return (void *)((uintptr_t)ptr + offset);
}

static inline void *mc_ptr_sub(void *ptr, size_t offset)
{
    return (void *)((uintptr_t)ptr - offset);
}

static inline ptrdiff_t mc_ptr_diff(void *ptr1, void *ptr2)
{
    return (intptr_t)ptr1 - (intptr_t)ptr2;
}

static inline size_t mc_ptr_diff_abs(void *ptr1, void *ptr2)
{
    return ptr1 > ptr2 ? (uintptr_t)ptr1 - (uintptr_t)ptr2
                       : (uintptr_t)ptr2 - (uintptr_t)ptr1;
}

static inline size_t mc_max2(size_t a, size_t b)
{
    return a > b ? a : b;
}

static inline size_t mc_min2(size_t a, size_t b)
{
    return a < b ? a : b;
}

#endif
