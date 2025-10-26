#ifndef MYCLIB_ELEMENT_CORE_H
#define MYCLIB_ELEMENT_CORE_H

#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>

/**
 * @brief Destroy function of the element type. (destructor)
 * @param self The object to destroy.
 * @note Do not free the memory of self.
 */
typedef void (*mc_destroy_func)(void *self);

/**
 * @brief Move function of the element type. (move constructor)
 * @param self The object to move.
 * @param source The source object to move from.
 * @note The self is an uninitialized object.
 */
typedef void (*mc_move_func)(void *self, void *source);

/**
 * @brief Copy function of the element type. (copy constructor)
 * @param self The object to copy.
 * @param source The source object to copy from.
 * @note The self is an initialized object.
 */
typedef void (*mc_copy_func)(void *self, const void *source);

/**
 * @brief Compare function of the element type.
 * @param self The object to compare.
 * @param other The other object to compare with.
 * @return An integer less than, equal to, or greater than zero if self is
 * found, respectively, to be less than, to match, or be greater than other.
 */
typedef int (*mc_compare_func)(const void *self, const void *other);

/**
 * @brief Equal function of the element type.
 * @param self The object to compare.
 * @param other The other object to compare with.
 * @return True if self is equal to other, false otherwise.
 */
typedef bool (*mc_equal_func)(const void *self, const void *other);

/**
 * @brief Hash function of the element type.
 * @param self The object to hash.
 * @return The hash value of self.
 */
typedef size_t (*mc_hash_func)(const void *self);

struct mc_elem_type {
    const char *name;
    size_t alignment;        /* alignof(T) (Should be a power of 2.) */
    size_t size;             /* sizeof(T) */
    mc_destroy_func destroy; /* nullable */
    mc_move_func move;       /* Must be non-NULL. */
    mc_copy_func copy;       /* nullable */
    mc_compare_func compare; /* nullable */
    mc_equal_func equal;     /* nullable */
    mc_hash_func hash;       /* nullable */
};

#define MC_ELEM_TYPE_DECLARE(name)                                             \
    const struct mc_elem_type *name##_get_mc_elem_type(void)

#define MC_ELEM_TYPE_DEFINE(name, type, destroy, move, copy, compare, equal,   \
                            hash)                                              \
    static const struct mc_elem_type name##_mc_elem_type = {                   \
        #name, alignof(type), sizeof(type), destroy, move,                     \
        copy,  compare,       equal,        hash};                             \
    const struct mc_elem_type *name##_get_mc_elem_type(void)                   \
    {                                                                          \
        return &name##_mc_elem_type;                                           \
    }

MC_ELEM_TYPE_DECLARE(int8);
MC_ELEM_TYPE_DECLARE(int16);
MC_ELEM_TYPE_DECLARE(int32);
MC_ELEM_TYPE_DECLARE(int64);
MC_ELEM_TYPE_DECLARE(uint8);
MC_ELEM_TYPE_DECLARE(uint16);
MC_ELEM_TYPE_DECLARE(uint32);
MC_ELEM_TYPE_DECLARE(uint64);
MC_ELEM_TYPE_DECLARE(static_str);

#endif
