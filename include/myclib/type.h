#ifndef MYCLIB_TYPE_H
#define MYCLIB_TYPE_H

#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef void (*mc_cleanup_func)(void *obj);

typedef void (*mc_move_func)(void *dst, void *src);

typedef void (*mc_copy_func)(void *dst, const void *src);

typedef int (*mc_compare_func)(void const *obj1, void const *obj2);

typedef bool (*mc_equal_func)(void const *obj1, void const *obj2);

typedef size_t (*mc_hash_func)(void const *obj);

struct mc_type {
    char const *name;
    size_t alignment;
    size_t size;
    mc_cleanup_func cleanup;
    mc_move_func move;
    mc_copy_func copy;
    mc_compare_func compare;
    mc_equal_func equal;
    mc_hash_func hash;
};

#define MC_DECLARE_TYPE(type_name)                                             \
    struct mc_type const *type_name##_get_mc_type(void)

#define MC_DEFINE_TYPE(type_name, type, cleanup_func, move_func, copy_func,    \
                       compare_func, equal_func, hash_func)                    \
    struct mc_type const *type_name##_get_mc_type(void)                        \
    {                                                                          \
        static struct mc_type const type_name##_mc_type = {                    \
            .name = #type_name,                                                \
            .alignment = alignof(type),                                        \
            .size = sizeof(type),                                              \
            .cleanup = cleanup_func,                                           \
            .move = move_func,                                                 \
            .copy = copy_func,                                                 \
            .compare = compare_func,                                           \
            .equal = equal_func,                                               \
            .hash = hash_func,                                                 \
        };                                                                     \
        return &type_name##_mc_type;                                           \
    }

#define MC_DEFINE_POD_TYPE(type_name, type, compare_func, equal_func,          \
                           hash_func)                                          \
    static void type_name##_move(void *dst, void *src)                         \
    {                                                                          \
        assert(dst);                                                           \
        assert(src);                                                           \
        memcpy(dst, src, sizeof(type));                                        \
    }                                                                          \
    static void type_name##_copy(void *dst, void const *src)                   \
    {                                                                          \
        assert(dst);                                                           \
        assert(src);                                                           \
        memcpy(dst, src, sizeof(type));                                        \
    }                                                                          \
    MC_DEFINE_TYPE(type_name, type, NULL, type_name##_move, type_name##_copy,  \
                   compare_func, equal_func, hash_func)

#define MC_DECLARE_EXTERNAL_TYPE(type_name)                                    \
    extern struct mc_type const type_name##_mc_type

#define MC_DEFINE_EXTERNAL_TYPE(type_name, type, cleanup_func, move_func,      \
                                copy_func, compare_func, equal_func,           \
                                hash_func)                                     \
    struct mc_type const type_name##_mc_type = {                               \
        .name = #type_name,                                                    \
        .alignment = alignof(type),                                            \
        .size = sizeof(type),                                                  \
        .cleanup = cleanup_func,                                               \
        .move = move_func,                                                     \
        .copy = copy_func,                                                     \
        .compare = compare_func,                                               \
        .equal = equal_func,                                                   \
        .hash = hash_func,                                                     \
    };

#define MC_DEFINE_EXTERNAL_POD_TYPE(type_name, type, compare_func, equal_func, \
                                    hash_func)                                 \
    static void type_name##_move(void *dst, void *src)                         \
    {                                                                          \
        assert(dst);                                                           \
        assert(src);                                                           \
        memcpy(dst, src, sizeof(type));                                        \
    }                                                                          \
    static void type_name##_copy(void *dst, void const *src)                   \
    {                                                                          \
        assert(dst);                                                           \
        assert(src);                                                           \
        memcpy(dst, src, sizeof(type));                                        \
    }                                                                          \
    MC_DEFINE_EXTERNAL_TYPE(type_name, type, NULL, type_name##_move,           \
                            type_name##_copy, compare_func, equal_func,        \
                            hash_func)

MC_DECLARE_TYPE(char);
MC_DECLARE_TYPE(short);
MC_DECLARE_TYPE(int);
MC_DECLARE_TYPE(long);
MC_DECLARE_TYPE(llong);
MC_DECLARE_TYPE(uchar);
MC_DECLARE_TYPE(ushort);
MC_DECLARE_TYPE(uint);
MC_DECLARE_TYPE(ulong);
MC_DECLARE_TYPE(ullong);
MC_DECLARE_TYPE(float);
MC_DECLARE_TYPE(double);
MC_DECLARE_TYPE(ldouble);
MC_DECLARE_TYPE(int8);
MC_DECLARE_TYPE(int16);
MC_DECLARE_TYPE(int32);
MC_DECLARE_TYPE(int64);
MC_DECLARE_TYPE(uint8);
MC_DECLARE_TYPE(uint16);
MC_DECLARE_TYPE(uint32);
MC_DECLARE_TYPE(uint64);
MC_DECLARE_TYPE(size);
MC_DECLARE_TYPE(str);

mc_cleanup_func mc_type_get_cleanup_forced(char const *caller,
                                           struct mc_type const *type);

mc_move_func mc_type_get_move_forced(char const *caller,
                                     struct mc_type const *type);

mc_copy_func mc_type_get_copy_forced(char const *caller,
                                     struct mc_type const *type);

mc_compare_func mc_type_get_compare_forced(char const *caller,
                                           struct mc_type const *type);

mc_equal_func mc_type_get_equal_forced(char const *caller,
                                       struct mc_type const *type);

mc_hash_func mc_type_get_hash_forced(char const *caller,
                                     struct mc_type const *type);

#endif
