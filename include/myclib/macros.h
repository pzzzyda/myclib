#ifndef MYCLIB_MACROS_H
#define MYCLIB_MACROS_H

#define MC_STATIC static

#if defined(__x86_64__) || defined(__aarch64__) || defined(__ppc64__)
#define MC_ARCH_64 1
#elif || defined(__i386__) || defined(__arm__) || defined(__ppc__)
#define MC_ARCH_32 1
#else
#error "Unknown architecture!"
#endif

/**
 * Only works for statically allocated arrays.
 */
#define MC_ARRAY_LEN(arr) (sizeof(arr) / sizeof((arr)[0]))

#define MC_ARRAY_INDEX(arr, elem_size, index)                                  \
    ((void *)((char *)(arr) + (elem_size) * (index)))

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

#endif
