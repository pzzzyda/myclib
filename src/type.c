#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "myclib/type.h"
#include "myclib/hash.h"

#define MC_DEFINE_INT_TYPE(type_name, type)                                    \
    static int type_name##_compare(void const *obj1, void const *obj2)         \
    {                                                                          \
        assert(obj1 != NULL);                                                  \
        assert(obj2 != NULL);                                                  \
        type a = *(type const *)obj1;                                          \
        type b = *(type const *)obj2;                                          \
        return a > b ? 1 : (a < b ? -1 : 0);                                   \
    }                                                                          \
    static bool type_name##_equal(void const *obj1, void const *obj2)          \
    {                                                                          \
        assert(obj1 != NULL);                                                  \
        assert(obj2 != NULL);                                                  \
        type a = *(type const *)obj1;                                          \
        type b = *(type const *)obj2;                                          \
        return a == b;                                                         \
    }                                                                          \
    static size_t type_name##_hash(void const *obj)                            \
    {                                                                          \
        assert(obj != NULL);                                                   \
        return MC_HASH(obj, sizeof(type));                                     \
    }                                                                          \
    MC_DEFINE_POD_TYPE(type_name, type, type_name##_compare,                   \
                       type_name##_equal, type_name##_hash)

MC_DEFINE_INT_TYPE(char, signed char)
MC_DEFINE_INT_TYPE(short, signed short)
MC_DEFINE_INT_TYPE(int, signed int)
MC_DEFINE_INT_TYPE(long, signed long)
MC_DEFINE_INT_TYPE(llong, signed long long)
MC_DEFINE_INT_TYPE(uchar, unsigned char)
MC_DEFINE_INT_TYPE(ushort, unsigned short)
MC_DEFINE_INT_TYPE(uint, unsigned int)
MC_DEFINE_INT_TYPE(ulong, unsigned long)
MC_DEFINE_INT_TYPE(ullong, unsigned long long)
MC_DEFINE_INT_TYPE(int8, int8_t)
MC_DEFINE_INT_TYPE(int16, int16_t)
MC_DEFINE_INT_TYPE(int32, int32_t)
MC_DEFINE_INT_TYPE(int64, int64_t)
MC_DEFINE_INT_TYPE(uint8, uint8_t)
MC_DEFINE_INT_TYPE(uint16, uint16_t)
MC_DEFINE_INT_TYPE(uint32, uint32_t)
MC_DEFINE_INT_TYPE(uint64, uint64_t)
MC_DEFINE_INT_TYPE(size, size_t)

#define MC_DEFINE_FLOAT_TYPE(type_name, type)                                  \
    static int type_name##_compare(void const *obj1, void const *obj2)         \
    {                                                                          \
        assert(obj1 != NULL);                                                  \
        assert(obj2 != NULL);                                                  \
        type a = *(type const *)obj1;                                          \
        type b = *(type const *)obj2;                                          \
        if (isnan(a) && isnan(b))                                              \
            return 0;                                                          \
        if (isnan(a))                                                          \
            return 1;                                                          \
        if (isnan(b))                                                          \
            return -1;                                                         \
        if (isinf(a) && isinf(b))                                              \
            return (signbit(a) == signbit(b)) ? 0 : (signbit(a) ? -1 : 1);     \
        if (isinf(a))                                                          \
            return signbit(a) ? -1 : 1;                                        \
        if (isinf(b))                                                          \
            return signbit(b) ? 1 : -1;                                        \
        return a > b ? 1 : (a < b ? -1 : 0);                                   \
    }                                                                          \
    static bool type_name##_equal(const void *obj1, const void *obj2)          \
    {                                                                          \
        assert(obj1 != NULL);                                                  \
        assert(obj2 != NULL);                                                  \
        return type_name##_compare(obj1, obj2) == 0;                           \
    }                                                                          \
    static size_t type_name##_hash(const void *obj)                            \
    {                                                                          \
        assert(obj != NULL);                                                   \
        return MC_HASH(obj, sizeof(type));                                     \
    }                                                                          \
    MC_DEFINE_POD_TYPE(type_name, type, type_name##_compare,                   \
                       type_name##_equal, type_name##_hash)

MC_DEFINE_FLOAT_TYPE(float, float)
MC_DEFINE_FLOAT_TYPE(double, double)
MC_DEFINE_FLOAT_TYPE(ldouble, long double)

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
