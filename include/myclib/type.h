#ifndef MYCLIB_TYPE_H
#define MYCLIB_TYPE_H

#include <stddef.h>
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

/**
 * @brief Destructor: clean up resources held by an initialized object.
 *
 * This function is responsible for releasing any resources (e.g., heap memory,
 * file descriptors, locks) owned by the object pointed to by `self`.
 *
 * @param self  Pointer to an **initialized** object. Must not be NULL.
 *              The object is valid upon entry and will not be used afterward.
 *
 * @note The memory pointed to by `self` itself **must not be freed** by this
 *       function. Deallocation of the memory block (e.g., via `free()`) is the
 *       caller's responsibility.
 *
 * @note After this call, the object is considered "destroyed" and must not be
 *       accessed.
 *
 * @note Equivalent to C++ destructor: `~T()`.
 *
 * @example For a string type: free the dynamically allocated `char*` buffer,
 *          but do not free the struct containing the pointer.
 */
typedef void (*mc_destroy_func)(void *self);

/**
 * @brief Move constructor: construct `self` by transferring resources from
 *        `source`.
 *
 * Initializes `self` by taking ownership of resources from `source`.
 * After this operation, `source` is left in a valid but unspecified state.
 *
 * @param self    Pointer to **uninitialized** memory where the new object will
 *                be constructed. Must not be NULL. On entry, the memory is raw
 *                storage.
 * @param source  Pointer to an **initialized** object to move from. Must not be
 *                NULL. After return, `source` is in a valid but unspecified
 *                state. Implementations should reset owned pointers to NULL or
 *                zero.
 *
 * @note This function is responsible for fully initializing `self`.
 *
 * @note Equivalent to C++ move constructor: `T(self, std::move(other))
 *       noexcept`.
 *
 * @warning Do not access `source` after this call unless its post-moved state
 *          is documented.
 *
 * @example For a string type: copy the `char*` pointer from `source` to `self`,
 *          then set `source->ptr = NULL` to prevent double-free.
 */
typedef void (*mc_move_construct_func)(void *self, void *source);

/**
 * @brief Copy constructor: construct `self` as a logical copy of `source`.
 *
 * Initializes `self` with a copy of the value represented by `source`.
 * This may involve deep copying of owned resources.
 *
 * @param self    Pointer to **uninitialized** memory where the new object will
 *                be constructed. Must not be NULL. On entry, the memory is raw
 *                storage.
 * @param source  Pointer to an **initialized** object to copy from. Must not be
 *                NULL.
 *
 * @note This function is responsible for fully initializing `self`, including
 *       allocating any necessary resources (e.g., duplicating strings, cloning
 *       containers).
 *
 * @note Equivalent to C++ copy constructor: `T(self, other)`.
 *
 * @example For a string type: allocate a new buffer and copy the string content
 *          from `source`.
 */
typedef void (*mc_copy_construct_func)(void *self, const void *source);

/**
 * @brief Compare two objects to establish a total ordering.
 *
 * Defines a strict weak ordering between two objects, used for sorting and
 * ordered containers.
 *
 * @param self   Pointer to the first **initialized** object. Must not be NULL.
 * @param other  Pointer to the second **initialized** object. Must not be NULL.
 *
 * @return An integer less than 0 if `self <  other`,
 *         equal to   0 if `self == other`,
 *         greater than 0 if `self >  other`.
 *
 * @note The comparison must define a consistent and transitive ordering.
 *
 * @note If `mc_equal_func` is defined, then `compare(a, b) == 0` should imply
 *       `equal(a, b) == true`.
 *
 * @note Equivalent to C++ `operator<=>` (three-way comparison) or `operator<`
 *       in sorted containers.
 *
 * @example For integers: return `*self - *other`; for strings: use `strcmp`.
 */
typedef int (*mc_compare_func)(const void *self, const void *other);

/**
 * @brief Check if two objects are logically equal.
 *
 * Determines whether two objects represent the same value.
 *
 * @param self   Pointer to the first **initialized** object. Must not be NULL.
 * @param other  Pointer to the second **initialized** object. Must not be NULL.
 *
 * @return `true` if the objects are considered equal, `false` otherwise.
 *
 * @note Should return `true` only if the two objects have the same logical
 *       value.
 *
 * @note Typically faster than `mc_compare_func` and sufficient for hash tables
 *       and equality checks.
 *
 * @note If `equal(a, b)` is `true`, then `hash(a)` must equal `hash(b)`.
 *
 * @note Equivalent to C++ `operator==`.
 *
 * @example For integers: `*self == *other`; for strings: `strcmp(self, other)
 *          == 0`.
 */
typedef bool (*mc_equal_func)(const void *self, const void *other);

/**
 * @brief Compute a hash value for the object.
 *
 * Produces a `size_t` hash value suitable for use in hash tables.
 *
 * @param self  Pointer to an **initialized** object. Must not be NULL.
 *
 * @return A hash value that distributes uniformly across the `size_t` range.
 *
 * @note Equal objects must produce the same hash:
 *       If `equal(a, b)` is `true`, then `hash(a) == hash(b)` must hold.
 *
 * @note Avoid hashing pointer addresses unless the pointer's target content
 *       defines equality. Instead, hash the **logical value** of the object.
 *
 * @note The hash function should be fast and minimize collisions.
 *
 * @note Equivalent to C++ `std::hash<T>{}(obj)`.
 *
 * @example For a string: iterate over characters and compute FNV-1a or djb2
 *          hash.
 */
typedef size_t (*mc_hash_func)(const void *self);

struct mc_type {
    const char *name;
    size_t alignment;                 /* alignof(T) */
    size_t size;                      /* sizeof(T) */
    mc_destroy_func destroy;          /* Nullable */
    mc_move_construct_func move_ctor; /* Non-Nullable */
    mc_copy_construct_func copy_ctor; /* Nullable */
    mc_compare_func compare;          /* Nullable */
    mc_equal_func equal;              /* Nullable */
    mc_hash_func hash;                /* Nullable */
};

/**
 * @brief Declare a function that returns the `mc_type` metadata for a type.
 *
 * This macro is used in **header files** to declare the type metadata accessor
 * function. It should be paired with `MC_DEFINE_TYPE` or `MC_DEFINE_POD_TYPE`
 * in the implementation.
 *
 * @param type_name The name of the type (without quotes). Will be used to form
 *                  the function name.
 *
 * @return A pointer to a constant `struct mc_type` describing the type.
 *
 * @note The returned pointer is valid for the entire program lifetime.
 * @note This macro expands to a function declaration; it does not define the
 *       function.
 *
 * @example
 *   // In my_type.h
 *   MC_DECLARE_TYPE(vector3);
 *
 *   // Expands to:
 *   // const struct mc_type *vector3_get_mc_type(void);
 */
#define MC_DECLARE_TYPE(type_name)                                             \
    const struct mc_type *type_name##_get_mc_type(void)

/**
 * @brief Define a custom type with full lifecycle and comparison operations.
 *
 * This macro is used in **source files** to define the `mc_type` metadata for a
 * type. It creates a static `struct mc_type` instance and a getter function
 * that returns it.
 *
 * @param type_name       The name of the type (symbol, not string). Used to
 *                        name the metadata.
 * @param type            The actual C type (e.g., `struct vector`, `int`,
 *                        `char*`).
 * @param destroy_func    Function to clean up resources (can be NULL for POD
 *                        types).
 * @param move_ctor_func  Function to move-construct from a source object
 *                        (required).
 * @param copy_ctor_func  Function to copy-construct from a source object
 *                        (required).
 * @param compare_func    Function to compare two objects (can be NULL if
 *                        ordering not needed).
 * @param equal_func      Function to check equality (required for hash tables).
 * @param hash_func       Function to compute hash (required for hash tables).
 *
 * @note This macro must be used at file scope (not inside a function).
 * @note The generated function is `static inline` in the sense that it's a
 *       regular function, but the metadata is `static const` and unique per
 *       translation unit.
 * @note All function pointers are stored directly; no indirection or vtable
 *       overhead.
 *
 * @warning Do not pass `NULL` for `move_ctor_func` or `copy_ctor_func` unless
 *          you know the type system will never attempt to construct objects
 *          (e.g., only using pointers).
 *
 * @example
 *   // In my_type.c
 *   MC_DEFINE_TYPE(string, struct string,
 *                  string_destroy,
 *                  string_move_construct,
 *                  string_copy_construct,
 *                  string_compare,
 *                  string_equal,
 *                  string_hash);
 */
#define MC_DEFINE_TYPE(type_name, type, destroy_func, move_ctor_func,          \
                       copy_ctor_func, compare_func, equal_func, hash_func)    \
    const struct mc_type *type_name##_get_mc_type(void)                        \
    {                                                                          \
        static const struct mc_type type_name##_mc_type = {                    \
            .name = #type_name,                                                \
            .alignment = alignof(type),                                        \
            .size = sizeof(type),                                              \
            .destroy = destroy_func,                                           \
            .move_ctor = move_ctor_func,                                       \
            .copy_ctor = copy_ctor_func,                                       \
            .compare = compare_func,                                           \
            .equal = equal_func,                                               \
            .hash = hash_func,                                                 \
        };                                                                     \
        return &type_name##_mc_type;                                           \
    }

/**
 * @brief Define a Plain Old Data (POD) type with default bitwise move/copy
 * semantics.
 *
 * This macro is a convenience wrapper for types that:
 * - Have no owned resources (so no custom destructor needed)
 * - Can be safely moved/copied with `memcpy`
 * - Are complete types at the point of definition
 *
 * It automatically generates trivial `move` and `copy` functions using
 * `memcpy`, and sets `destroy` to `NULL`.
 *
 * @param type_name    The name of the type (symbol).
 * @param type         The actual C type (must be complete and POD-compatible).
 * @param compare_func Function for ordering (can be NULL).
 * @param equal_func   Function for equality (required for hash tables).
 * @param hash_func    Function for hashing (required for hash tables).
 *
 * @note The type must be complete (`sizeof(type)` must be valid).
 * @note The type should not contain pointers to owned memory, virtual
 *       functions, or non-trivial state.
 * @note The generated `move` and `copy` functions use `assert` to check for
 *       NULL pointers.
 * @note This macro expands to two `static` helper functions and a call to
 *       `MC_DEFINE_TYPE`.
 *
 * @example
 *   // For a simple struct
 *   struct vector3 { float x, y, z; };
 *
 *   static int vec3_compare(const void *a, const void *b) { ... }
 *   static bool vec3_equal(const void *a, const void *b) { ... }
 *   static size_t vec3_hash(const void *a) { ... }
 *
 *   // In vector3.c
 *   MC_DEFINE_POD_TYPE(vector3, struct vector3, vec3_compare, vec3_equal,
 *   vec3_hash);
 *
 * @see MC_DEFINE_TYPE for non-POD types.
 */
#define MC_DEFINE_POD_TYPE(type_name, type, compare_func, equal_func,          \
                           hash_func)                                          \
    static void type_name##_move(void *self, void *source)                     \
    {                                                                          \
        assert(self);                                                          \
        assert(source);                                                        \
        memcpy(self, source, sizeof(type));                                    \
    }                                                                          \
    static void type_name##_copy(void *self, const void *source)               \
    {                                                                          \
        assert(self);                                                          \
        assert(source);                                                        \
        memcpy(self, source, sizeof(type));                                    \
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
MC_DECLARE_TYPE(str); /* The string literal type. e.g. "hello" */

/**
 * @brief Get the destroy function for a type.
 *
 * This function retrieves the destroy function for the specified type. If the
 * type does not have a destroy function, it panics.
 *
 * @param caller The name of the caller function (for error reporting).
 * @param type   The type for which to retrieve the destroy function.
 *
 * @return The destroy function for the specified type.
 *
 * @note This function panics if the type does not have a destroy function.
 */
mc_destroy_func mc_type_get_destroy_forced(const char *caller,
                                           const struct mc_type *type);

/**
 * @brief Get the move constructor function for a type.
 *
 * This function retrieves the move constructor function for the specified type.
 * If the type does not have a move constructor function, it panics.
 *
 * @param caller The name of the caller function (for error reporting).
 * @param type   The type for which to retrieve the move constructor function.
 *
 * @return The move constructor function for the specified type.
 *
 * @note This function panics if the type does not have a move constructor
 *       function.
 */
mc_move_construct_func mc_type_get_move_ctor_forced(const char *caller,
                                                    const struct mc_type *type);

/**
 * @brief Get the copy constructor function for a type.
 *
 * This function retrieves the copy constructor function for the specified type.
 * If the type does not have a copy constructor function, it panics.
 *
 * @param caller The name of the caller function (for error reporting).
 * @param type   The type for which to retrieve the copy constructor function.
 *
 * @return The copy constructor function for the specified type.
 *
 * @note This function panics if the type does not have a copy constructor
 *       function.
 */
mc_copy_construct_func mc_type_get_copy_ctor_forced(const char *caller,
                                                    const struct mc_type *type);

/**
 * @brief Get the compare function for a type.
 *
 * This function retrieves the compare function for the specified type. If the
 * type does not have a compare function, it panics.
 *
 * @param caller The name of the caller function (for error reporting).
 * @param type   The type for which to retrieve the compare function.
 *
 * @return The compare function for the specified type.
 *
 * @note This function panics if the type does not have a compare function.
 */
mc_compare_func mc_type_get_compare_forced(const char *caller,
                                           const struct mc_type *type);

/**
 * @brief Get the equal function for a type.
 *
 * This function retrieves the equal function for the specified type. If the
 * type does not have an equal function, it panics.
 *
 * @param caller The name of the caller function (for error reporting).
 * @param type   The type for which to retrieve the equal function.
 *
 * @return The equal function for the specified type.
 *
 * @note This function panics if the type does not have an equal function.
 */
mc_equal_func mc_type_get_equal_forced(const char *caller,
                                       const struct mc_type *type);

/**
 * @brief Get the hash function for a type.
 *
 * This function retrieves the hash function for the specified type. If the
 * type does not have a hash function, it panics.
 *
 * @param caller The name of the caller function (for error reporting).
 * @param type   The type for which to retrieve the hash function.
 *
 * @return The hash function for the specified type.
 *
 * @note This function panics if the type does not have a hash function.
 */
mc_hash_func mc_type_get_hash_forced(const char *caller,
                                     const struct mc_type *type);

#endif
