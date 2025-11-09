#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "myclib/type.h"
#include "myclib/hash.h"

#define MC_DEFINE_INT_TYPE(type_name, type)                                    \
    static int type_name##_compare(void const *dst, void const *src)           \
    {                                                                          \
        assert(dst != NULL);                                                   \
        assert(src != NULL);                                                   \
        type a = *(type const *)dst;                                           \
        type b = *(type const *)src;                                           \
        return a > b ? 1 : (a < b ? -1 : 0);                                   \
    }                                                                          \
    static bool type_name##_equal(void const *dst, void const *src)            \
    {                                                                          \
        assert(dst != NULL);                                                   \
        assert(src != NULL);                                                   \
        type a = *(type const *)dst;                                           \
        type b = *(type const *)src;                                           \
        return a == b;                                                         \
    }                                                                          \
    static size_t type_name##_hash(void const *obj)                            \
    {                                                                          \
        assert(obj != NULL);                                                   \
        return MC_HASH(obj, sizeof(type));                                     \
    }                                                                          \
    MC_DEFINE_POD_TYPE(type_name, type, type_name##_compare,                   \
                       type_name##_equal, type_name##_hash)

static int str_compare(void const *obj1, void const *obj2)
{
    assert(obj1);
    assert(obj2);
    char const *const *s1 = obj1;
    char const *const *s2 = obj2;
    return strcmp(*s1, *s2);
}

static bool str_equal(void const *obj1, void const *obj2)
{
    assert(obj1);
    assert(obj2);
    char const *const *s1 = obj1;
    char const *const *s2 = obj2;
    return strcmp(*s1, *s2) == 0;
}

static size_t str_hash(void const *obj)
{
    assert(obj);
    char const *const *s = obj;
    return MC_HASH(*s, strlen(*s));
}

MC_DEFINE_INT_TYPE(int8, int8_t)
MC_DEFINE_INT_TYPE(int16, int16_t)
MC_DEFINE_INT_TYPE(int32, int32_t)
MC_DEFINE_INT_TYPE(int64, int64_t)
MC_DEFINE_INT_TYPE(uint8, uint8_t)
MC_DEFINE_INT_TYPE(uint16, uint16_t)
MC_DEFINE_INT_TYPE(uint32, uint32_t)
MC_DEFINE_INT_TYPE(uint64, uint64_t)
MC_DEFINE_POD_TYPE(str, char *, str_compare, str_equal, str_hash)

#define MC_TYPE_FORCED_FUNC_GETTER(func_type, field)                           \
    func_type mc_type_get_##field##_forced(char const *caller,                 \
                                           struct mc_type const *type)         \
    {                                                                          \
        assert(type);                                                          \
        if (type->field)                                                       \
            return type->field;                                                \
        fprintf(stderr, "%s: type %s did not implement %s function\n", caller, \
                type->name, #func_type);                                       \
        abort();                                                               \
    }

MC_TYPE_FORCED_FUNC_GETTER(mc_cleanup_func, cleanup)
MC_TYPE_FORCED_FUNC_GETTER(mc_move_func, move)
MC_TYPE_FORCED_FUNC_GETTER(mc_copy_func, copy)
MC_TYPE_FORCED_FUNC_GETTER(mc_compare_func, compare)
MC_TYPE_FORCED_FUNC_GETTER(mc_equal_func, equal)
MC_TYPE_FORCED_FUNC_GETTER(mc_hash_func, hash)
