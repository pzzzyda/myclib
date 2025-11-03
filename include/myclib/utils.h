#ifndef MYCLIB_UTILS_H
#define MYCLIB_UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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

static inline size_t mc_max2(size_t a, size_t b)
{
    return a > b ? a : b;
}

static inline bool mc_is_pow_of_two(size_t n)
{
    return n != 0 && (n & (n - 1)) == 0;
}

static inline size_t mc_next_pow_of_two(size_t n)
{
    if (n == 0)
        return 1;
    if (n == 1)
        return 1;

#if SIZE_MAX == UINT64_MAX
    if (n > (size_t)1 << 63)
        return SIZE_MAX;
#else
    if (n > (size_t)1 << 31)
        return SIZE_MAX;
#endif

    n -= 1;

    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
#if SIZE_MAX == UINT64_MAX
    n |= n >> 32;
#endif
    return n + 1;
}

#endif
