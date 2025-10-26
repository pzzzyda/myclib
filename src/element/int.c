#include <string.h>
#include <assert.h>
#include "myclib/element/core.h"
#include "myclib/hash.h"

#define MC_ELEM_TYPE_INT(name, type)                                           \
    static void name##_move(void *self, void *source)                          \
    {                                                                          \
        assert(self != NULL);                                                  \
        assert(source != NULL);                                                \
        memcpy(self, source, sizeof(type));                                    \
    }                                                                          \
    static void name##_copy(void *self, const void *source)                    \
    {                                                                          \
        assert(self != NULL);                                                  \
        assert(source != NULL);                                                \
        memcpy(self, source, sizeof(type));                                    \
    }                                                                          \
    static int name##_compare(const void *self, const void *other)             \
    {                                                                          \
        assert(self != NULL);                                                  \
        assert(other != NULL);                                                 \
        type a = *(const type *)self;                                          \
        type b = *(const type *)other;                                         \
        return a > b ? 1 : (a < b ? -1 : 0);                                   \
    }                                                                          \
    static bool name##_equal(const void *self, const void *other)              \
    {                                                                          \
        assert(self != NULL);                                                  \
        assert(other != NULL);                                                 \
        type a = *(const type *)self;                                          \
        type b = *(const type *)other;                                         \
        return a == b;                                                         \
    }                                                                          \
    static size_t name##_hash(const void *self)                                \
    {                                                                          \
        assert(self != NULL);                                                  \
        return MC_HASH(self, sizeof(type));                                    \
    }                                                                          \
    MC_ELEM_TYPE_DEFINE(name, type, NULL, name##_move, name##_copy,            \
                        name##_compare, name##_equal, name##_hash)

MC_ELEM_TYPE_INT(int8, int8_t)
MC_ELEM_TYPE_INT(int16, int16_t)
MC_ELEM_TYPE_INT(int32, int32_t)
MC_ELEM_TYPE_INT(int64, int64_t)
MC_ELEM_TYPE_INT(uint8, uint8_t)
MC_ELEM_TYPE_INT(uint16, uint16_t)
MC_ELEM_TYPE_INT(uint32, uint32_t)
MC_ELEM_TYPE_INT(uint64, uint64_t)
