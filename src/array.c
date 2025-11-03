#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myclib/array.h"
#include "myclib/aligned_malloc.h"
#include "myclib/utils.h"

#define MC_ARRAY_FOR_EACH(elem, array, start, end, body)                       \
    do {                                                                       \
        size_t __elem_size = array->elem_type->size;                           \
        void *elem = mc_array_get_unchecked(array, start);                     \
        void *__end = mc_array_get_unchecked(array, end);                      \
        for (; elem < __end; elem = mc_ptr_add(elem, __elem_size))             \
            body                                                               \
    } while (0)

void mc_array_init(struct mc_array *array, struct mc_type const *elem_type)
{
    assert(array);
    assert(elem_type);
    assert(elem_type->move);
    assert(elem_type->size > 0);
    assert(mc_is_pow_of_two(elem_type->alignment));
    array->data = NULL;
    array->len = 0;
    array->capacity = 0;
    array->elem_type = elem_type;
}

static void mc_array_free_data(struct mc_array *array)
{
    if (array->capacity > 0) {
        mc_aligned_free(array->data);
        array->data = NULL;
        array->capacity = 0;
    }
}

void mc_array_destroy(struct mc_array *array)
{
    assert(array);
    mc_array_truncate(array, 0);
    mc_array_free_data(array);
    array->elem_type = NULL;
}

size_t mc_array_len(struct mc_array const *array)
{
    assert(array);
    return array->len;
}

size_t mc_array_capacity(struct mc_array const *array)
{
    assert(array);
    return array->capacity;
}

bool mc_array_is_empty(struct mc_array const *array)
{
    assert(array);
    return array->len == 0;
}

void *mc_array_get(struct mc_array *array, size_t index)
{
    assert(array);
    return index < array->len ? mc_array_get_unchecked(array, index) : NULL;
}

void *mc_array_get_unchecked(struct mc_array *array, size_t index)
{
    assert(array);
    return mc_ptr_add(array->data, array->elem_type->size * index);
}

void *mc_array_get_first(struct mc_array *array)
{
    assert(array);
    return array->len > 0 ? mc_array_get_unchecked(array, 0) : NULL;
}

void *mc_array_get_last(struct mc_array *array)
{
    assert(array);
    return array->len > 0 ? mc_array_get_unchecked(array, array->len - 1)
                          : NULL;
}

static void mc_array_grow_capacity_if_needed(struct mc_array *array)
{
    if (array->len < array->capacity)
        return;

    mc_array_reserve(array, 1);
}

static void mc_array_place_one(struct mc_array *array, size_t index, void *elem)
{
    array->elem_type->move(mc_array_get_unchecked(array, index), elem);
}

static void mc_array_place_range(struct mc_array *array, size_t index,
                                 void *elems, size_t len)
{
    if (len == 0)
        return;

    mc_move_func move = array->elem_type->move;
    size_t elem_size = array->elem_type->size;
    char *dst = mc_array_get_unchecked(array, index);
    char *src = elems;

    for (size_t i = 0; i < len; ++i) {
        move(dst, src);
        dst += elem_size;
        src += elem_size;
    }
}

void mc_array_push(struct mc_array *array, void *elem)
{
    assert(array);
    assert(elem);

    mc_array_grow_capacity_if_needed(array);

    mc_array_place_one(array, array->len, elem);

    ++array->len;
}

static void mc_array_extract_one(struct mc_array *array, size_t index,
                                 void *out_elem)
{
    void *elem = mc_array_get_unchecked(array, index);

    if (out_elem)
        array->elem_type->move(out_elem, elem);
    else if (array->elem_type->destroy)
        array->elem_type->destroy(elem);
}

static void mc_array_extract_range(struct mc_array *array, size_t index,
                                   size_t len, void *out_elems,
                                   size_t out_elems_len)
{
    if (len == 0)
        return;

    size_t elem_size = array->elem_type->size;
    char *data = mc_array_get_unchecked(array, index);
    size_t move_count = 0;

    if (out_elems && out_elems_len > 0) {
        mc_move_func move = array->elem_type->move;
        char *dst = out_elems;
        move_count = len < out_elems_len ? len : out_elems_len;
        for (size_t i = 0; i < move_count; ++i) {
            move(dst, data);
            dst += elem_size;
            data += elem_size;
        }
    }

    mc_destroy_func destroy = array->elem_type->destroy;
    if (destroy) {
        index += move_count;

        char *remaining = mc_array_get_unchecked(array, index);

        for (size_t i = move_count; i < len; ++i) {
            destroy(remaining);
            remaining += elem_size;
        }
    }
}

bool mc_array_pop(struct mc_array *array, void *out_elem)
{
    assert(array);

    if (array->len == 0)
        return false;

    mc_array_extract_one(array, --array->len, out_elem);

    return true;
}

static void mc_array_shift(struct mc_array *array, size_t dst, size_t src,
                           size_t n)
{
    if (n == 0)
        return;

    memmove(mc_array_get_unchecked(array, dst),
            mc_array_get_unchecked(array, src), n * array->elem_type->size);
}

static void mc_array_bounds_check(char const *func_name, size_t index,
                                  size_t bounds, bool allow_equal)
{
    if (!allow_equal && index >= bounds) {
        fprintf(stderr, "%s: index (is %zu) must < len (is %zu)\n", func_name,
                index, bounds);
        abort();
    }
    if (allow_equal && index > bounds) {
        fprintf(stderr, "%s: index (is %zu) must <= len (is %zu)\n", func_name,
                index, bounds);
        abort();
    }
}

void mc_array_insert(struct mc_array *array, size_t index, void *elem)
{
    assert(array);
    assert(elem);

    size_t len = array->len;

    /* Allow insertion at the end. */
    mc_array_bounds_check(__func__, index, len, true);

    mc_array_grow_capacity_if_needed(array);

    mc_array_shift(array, index + 1, index, len - index);

    mc_array_place_one(array, index, elem);

    ++array->len;
}

void mc_array_remove(struct mc_array *array, size_t index, void *out_elem)
{
    assert(array);

    size_t len = array->len;

    mc_array_bounds_check(__func__, index, len, false);

    mc_array_extract_one(array, index, out_elem);

    mc_array_shift(array, index, index + 1, len - index - 1);

    --array->len;
}

void mc_array_append_range(struct mc_array *array, void *elems,
                           size_t elems_len)
{
    assert(array);
    assert(elems_len == 0 || elems);

    if (elems_len == 0)
        return;

    mc_array_reserve(array, elems_len);

    mc_array_place_range(array, array->len, elems, elems_len);

    array->len += elems_len;
}

void mc_array_insert_range(struct mc_array *array, size_t index, void *elems,
                           size_t elems_len)
{
    assert(array);
    assert(elems_len == 0 || elems);

    if (elems_len == 0)
        return;

    size_t arr_len = array->len;

    /* Allow insertion at the end. */
    mc_array_bounds_check(__func__, index, arr_len, true);

    mc_array_reserve(array, elems_len);

    mc_array_shift(array, index + elems_len, index, arr_len - index);

    mc_array_place_range(array, index, elems, elems_len);

    array->len += elems_len;
}

void mc_array_remove_range(struct mc_array *array, size_t index, size_t len,
                           void *out_elems, size_t out_elems_len)
{
    assert(array);

    if (len == 0)
        return;

    size_t arr_len = array->len;

    mc_array_bounds_check(__func__, index, arr_len, false);

    if (index + len > arr_len)
        len = arr_len - index;

    mc_array_extract_range(array, index, len, out_elems, out_elems_len);

    mc_array_shift(array, index, index + len, arr_len - index - len);

    array->len -= len;
}

void mc_array_clear(struct mc_array *array)
{
    assert(array);
    mc_array_truncate(array, 0);
}

static void mc_array_adjust_capacity(struct mc_array *array, size_t capacity)
{
    if (capacity == 0) {
        mc_array_free_data(array);
        return;
    }

    size_t elem_align = array->elem_type->alignment;
    size_t elem_size = array->elem_type->size;
    size_t total_size = capacity * elem_size;

    void *new_data = mc_aligned_malloc(elem_align, total_size);
    if (!new_data) {
        fprintf(stderr, "memory allocation of %zu bytes failed\n", total_size);
        abort();
    }

    memcpy(new_data, array->data, array->len * elem_size);

    mc_aligned_free(array->data);
    array->data = new_data;
    array->capacity = capacity;
}

void mc_array_reserve(struct mc_array *array, size_t additional)
{
    assert(array);

    if (additional > SIZE_MAX / array->elem_type->size - array->len) {
        fprintf(stderr, "capacity overflow\n");
        abort();
    }

    mc_array_adjust_capacity(
        array, mc_max2(array->len + additional, array->capacity * 2));
}

void mc_array_reserve_exact(struct mc_array *array, size_t additional)
{
    assert(array);

    if (additional > SIZE_MAX / array->elem_type->size - array->len) {
        fprintf(stderr, "capacity overflow\n");
        abort();
    }

    mc_array_adjust_capacity(array, array->len + additional);
}

void mc_array_shrink_to_fit(struct mc_array *array)
{
    assert(array);
    mc_array_adjust_capacity(array, array->len);
}

void mc_array_shrink_to(struct mc_array *array, size_t capacity)
{
    assert(array);

    if (capacity < array->len)
        return;

    if (capacity >= array->capacity)
        return;

    mc_array_adjust_capacity(array, capacity);
}

void mc_array_truncate(struct mc_array *array, size_t len)
{
    assert(array);

    if (len >= array->len)
        return;

    mc_destroy_func destroy = array->elem_type->destroy;
    if (destroy)
        MC_ARRAY_FOR_EACH(curr, array, len, array->len, { destroy(curr); });

    array->len = len;
}

void mc_array_resize(struct mc_array *array, size_t len, void *elem)
{
    assert(array);

    mc_copy_func copy = mc_type_get_copy_forced(__func__, array->elem_type);

    if (len <= array->len) {
        mc_array_truncate(array, len);
        if (elem && array->elem_type->destroy)
            array->elem_type->destroy(elem);
        return;
    }

    assert(elem);

    mc_array_reserve(array, len - array->len);

    MC_ARRAY_FOR_EACH(curr, array, array->len, len - 1, { copy(curr, elem); });

    mc_array_place_one(array, len - 1, elem);

    array->len = len;
}

bool mc_array_contains(struct mc_array *array, void const *elem)
{
    assert(array);
    assert(elem);

    mc_equal_func eq = mc_type_get_equal_forced(__func__, array->elem_type);
    MC_ARRAY_FOR_EACH(curr, array, 0, array->len, {
        if (eq(curr, elem))
            return true;
    });

    return false;
}

void mc_array_sort(struct mc_array *array)
{
    assert(array);
    mc_compare_func cmp =
        mc_type_get_compare_forced(__func__, array->elem_type);
    mc_array_sort_with(array, cmp);
}

void mc_array_sort_with(struct mc_array *array, mc_compare_func cmp)
{
    assert(array);
    assert(cmp);

    if (array->len < 2)
        return;

    qsort(array->data, array->len, array->elem_type->size, cmp);
}

bool mc_array_binary_search(struct mc_array *array, void const *elem,
                            size_t *out_index)
{
    assert(array);
    assert(elem);

    mc_compare_func cmp =
        mc_type_get_compare_forced(__func__, array->elem_type);

    if (array->len < 1)
        return false;

    size_t lo = 0, hi = array->len - 1;
    while (lo <= hi) {
        size_t mid = lo + (hi - lo) / 2;
        int res = cmp(mc_array_get_unchecked(array, mid), elem);
        if (res < 0) {
            lo = mid + 1;
        } else if (res > 0) {
            hi = mid - 1;
        } else {
            if (out_index)
                *out_index = mid;
            return true;
        }
    }

    return false;
}

void mc_array_for_each(struct mc_array *array,
                       void (*func)(void *elem, void *user_data),
                       void *user_data)
{
    assert(array);
    assert(func);
    MC_ARRAY_FOR_EACH(curr, array, 0, array->len, { func(curr, user_data); });
}
