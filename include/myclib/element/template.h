#ifndef MC_NAME
#error "Template name MC_NAME is not defined!"
#endif

#ifndef MC_TYPE
#error "Template type MC_TYPE is not defined!"
#endif

#ifndef MC_MOVE_IMPL
#error "Missing MC_MOVE_IMPL definition!"
#endif

#ifndef MC_STATIC
#define MC_STATIC static
#endif

#define __MC_DESTROY_FUNC MC_NAME##_destroy
#define __MC_MOVE_FUNC MC_NAME##_move
#define __MC_COPY_FUNC MC_NAME##_copy
#define __MC_COMPARE_FUNC MC_NAME##_compare
#define __MC_EQUAL_FUNC MC_NAME##_equal
#define __MC_HASH_FUNC MC_NAME##_hash
#define __MC_ELEM_TYPE_DEFINE(name, type, destroy, move, copy, compare, equal, \
                              hash)                                            \
    MC_ELEM_TYPE_DEFINE(name, type, destroy, move, copy, compare, equal, hash)

/* Destroy function of the element type. (destructor) */
#ifdef MC_DESTROY_IMPL
MC_STATIC void __MC_DESTROY_FUNC(void *__self)
{
    typedef MC_TYPE __mc_self_t;
    __mc_self_t *__self_ptr = __self;
    MC_DESTROY_IMPL(__self_ptr);
}
#else
#undef __MC_DESTROY_FUNC
#define __MC_DESTROY_FUNC NULL
#endif /* MC_DESTROY_IMPL */

/* Move function of the element type. (move constructor) */
MC_STATIC void __MC_MOVE_FUNC(void *__self, void *__source)
{
    typedef MC_TYPE __mc_self_t;
    __mc_self_t *__self_ptr = __self;
    __mc_self_t *__source_ptr = __source;
    MC_MOVE_IMPL(__self_ptr, __source_ptr);
}

/* Copy function of the element type. (copy constructor) */
#ifdef MC_COPY_IMPL
MC_STATIC void __MC_COPY_FUNC(void *__self, const void *__source)
{
    typedef MC_TYPE __mc_self_t;
    __mc_self_t *__self_ptr = __self;
    const __mc_self_t *__source_ptr = __source;
    MC_COPY_IMPL(__self_ptr, __source_ptr);
}
#else
#undef __MC_COPY_FUNC
#define __MC_COPY_FUNC NULL
#endif /* MC_COPY_IMPL */

/* Compare function of the element type. */
#ifdef MC_COMPARE_IMPL
MC_STATIC int __MC_COMPARE_FUNC(const void *__self, const void *__other)
{
    typedef MC_TYPE __mc_self_t;
    const __mc_self_t *__self_ptr = __self;
    const __mc_self_t *__other_ptr = __other;
    MC_COMPARE_IMPL(__self_ptr, __other_ptr);
}
/**
 * If MC_COMPARE_IMPL is not defined, MC_EQUAL_IMPL is defined as
 * return __MC_COMPARE_FUNC(self, other) == 0.
 */
#ifndef MC_EQUAL_IMPL
#define MC_EQUAL_IMPL(self, other) return __MC_COMPARE_FUNC(self, other) == 0
#endif
#else
#undef __MC_COMPARE_FUNC
#define __MC_COMPARE_FUNC NULL
#endif /* MC_COMPARE_IMPL */

/* Equal function of the element type. */
#ifdef MC_EQUAL_IMPL
MC_STATIC bool __MC_EQUAL_FUNC(const void *__self, const void *__other)
{
    typedef MC_TYPE __mc_self_t;
    const __mc_self_t *__self_ptr = __self;
    const __mc_self_t *__other_ptr = __other;
    MC_EQUAL_IMPL(__self_ptr, __other_ptr);
}
#else
#undef __MC_EQUAL_FUNC
#define __MC_EQUAL_FUNC NULL
#endif /* MC_EQUAL_IMPL */

/* Hash function of the element type. */
#ifdef MC_HASH_IMPL
MC_STATIC size_t __MC_HASH_FUNC(const void *__self)
{
    typedef MC_TYPE __mc_self_t;
    const __mc_self_t *__self_ptr = __self;
    MC_HASH_IMPL(__self_ptr);
}
#else
#undef __MC_HASH_FUNC
#define __MC_HASH_FUNC NULL
#endif /* MC_HASH_IMPL */

__MC_ELEM_TYPE_DEFINE(MC_NAME, MC_TYPE, __MC_DESTROY_FUNC, __MC_MOVE_FUNC,
                      __MC_COPY_FUNC, __MC_COMPARE_FUNC, __MC_EQUAL_FUNC,
                      __MC_HASH_FUNC)

#undef MC_NAME
#undef MC_TYPE

#ifdef MC_DESTROY_IMPL
#undef MC_DESTROY_IMPL
#endif

#undef MC_MOVE_IMPL

#ifdef MC_COPY_IMPL
#undef MC_COPY_IMPL
#endif

#ifdef MC_COMPARE_IMPL
#undef MC_COMPARE_IMPL
#endif

#ifdef MC_EQUAL_IMPL
#undef MC_EQUAL_IMPL
#endif

#ifdef MC_HASH_IMPL
#undef MC_HASH_IMPL
#endif

#undef __MC_DESTROY_FUNC
#undef __MC_MOVE_FUNC
#undef __MC_COPY_FUNC
#undef __MC_COMPARE_FUNC
#undef __MC_EQUAL_FUNC
#undef __MC_HASH_FUNC

#undef __MC_ELEM_TYPE_DEFINE
