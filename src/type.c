#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "myclib/type.h"
#include "myclib/hash.h"

#define MC_DEFINE_INT_TYPE(type_name, type)                                    \
    static int type_name##_compare(const void *self, const void *other)        \
    {                                                                          \
        assert(self != NULL);                                                  \
        assert(other != NULL);                                                 \
        type a = *(const type *)self;                                          \
        type b = *(const type *)other;                                         \
        return a > b ? 1 : (a < b ? -1 : 0);                                   \
    }                                                                          \
    static bool type_name##_equal(const void *self, const void *other)         \
    {                                                                          \
        assert(self != NULL);                                                  \
        assert(other != NULL);                                                 \
        type a = *(const type *)self;                                          \
        type b = *(const type *)other;                                         \
        return a == b;                                                         \
    }                                                                          \
    static size_t type_name##_hash(const void *self)                           \
    {                                                                          \
        assert(self != NULL);                                                  \
        return MC_HASH(self, sizeof(type));                                    \
    }                                                                          \
    MC_DEFINE_POD_TYPE(type_name, type, type_name##_compare,                   \
                       type_name##_equal, type_name##_hash)

static void str_move(void *self, void *source)
{
    assert(self);
    assert(source);
    memcpy(self, source, sizeof(const char *));
}

static void str_copy(void *self, const void *source)
{
    assert(self);
    assert(source);
    memcpy(self, source, sizeof(const char *));
}

static int str_compare(const void *self, const void *other)
{
    assert(self);
    assert(other);
    const char *const *s = self;
    const char *const *o = other;
    return strcmp(*s, *o);
}

static bool str_equal(const void *self, const void *other)
{
    assert(self);
    assert(other);
    const char *const *s = self;
    const char *const *o = other;
    return strcmp(*s, *o) == 0;
}

static size_t str_hash(const void *self)
{
    assert(self);
    const char *const *s = self;
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
MC_DEFINE_TYPE(str, const char *, NULL, str_move, str_copy, str_compare,
               str_equal, str_hash)

#define MC_TYPE_FORCED_FUNC_GETTER(func_type, field)                           \
    func_type mc_type_get_##field##_forced(const char *caller,                 \
                                           const struct mc_type *type)         \
    {                                                                          \
        assert(type);                                                          \
        if (type->field)                                                       \
            return type->field;                                                \
        fprintf(stderr, "%s: type %s did not implement %s function\n", caller, \
                type->name, #func_type);                                       \
        abort();                                                               \
    }

MC_TYPE_FORCED_FUNC_GETTER(mc_destroy_func, destroy)
MC_TYPE_FORCED_FUNC_GETTER(mc_move_construct_func, move_ctor)
MC_TYPE_FORCED_FUNC_GETTER(mc_copy_construct_func, copy_ctor)
MC_TYPE_FORCED_FUNC_GETTER(mc_compare_func, compare)
MC_TYPE_FORCED_FUNC_GETTER(mc_equal_func, equal)
MC_TYPE_FORCED_FUNC_GETTER(mc_hash_func, hash)
