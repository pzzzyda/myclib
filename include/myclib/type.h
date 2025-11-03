#ifndef MYCLIB_TYPE_H
#define MYCLIB_TYPE_H

#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

typedef void (*mc_destroy_func)(void *obj);

typedef void (*mc_move_func)(void *dst, void *src);

typedef void (*mc_copy_func)(void *dst, const void *src);

typedef int (*mc_compare_func)(void const *obj1, void const *obj2);

typedef bool (*mc_equal_func)(void const *obj1, void const *obj2);

typedef size_t (*mc_hash_func)(void const *obj);

struct mc_type {
    char const *name;
    size_t alignment;
    size_t size;
    mc_destroy_func destroy;
    mc_move_func move;
    mc_copy_func copy;
    mc_compare_func compare;
    mc_equal_func equal;
    mc_hash_func hash;
};

#define MC_DECLARE_TYPE(type_name)                                             \
    struct mc_type const *type_name##_get_mc_type(void)

#define MC_DEFINE_TYPE(type_name, type, destroy_func, move_func, copy_func,    \
                       compare_func, equal_func, hash_func)                    \
    struct mc_type const *type_name##_get_mc_type(void)                        \
    {                                                                          \
        static struct mc_type const type_name##_mc_type = {                    \
            .name = #type_name,                                                \
            .alignment = alignof(type),                                        \
            .size = sizeof(type),                                              \
            .destroy = destroy_func,                                           \
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

MC_DECLARE_TYPE(int8);
MC_DECLARE_TYPE(int16);
MC_DECLARE_TYPE(int32);
MC_DECLARE_TYPE(int64);
MC_DECLARE_TYPE(uint8);
MC_DECLARE_TYPE(uint16);
MC_DECLARE_TYPE(uint32);
MC_DECLARE_TYPE(uint64);
MC_DECLARE_TYPE(str);

mc_destroy_func mc_type_get_destroy_forced(char const *caller,
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
