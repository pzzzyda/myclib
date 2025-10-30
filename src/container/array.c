#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myclib/container/array.h"
#include "myclib/aligned_malloc.h"
#include "myclib/utils.h"

#define FOR_EACH(elem, self, start, end, body)                                 \
    do {                                                                       \
        size_t __elem_size = self->elem_type->size;                            \
        void *elem = mc_array_get_unchecked(self, start);                      \
        void *__end = mc_array_get_unchecked(self, end);                       \
        for (; elem < __end; elem = mc_ptr_add(elem, __elem_size))             \
            body                                                               \
    } while (0)

void mc_array_init(struct mc_array *self, const struct mc_type *elem_type)
{
    assert(self);
    assert(elem_type);
    assert(elem_type->move_ctor);
    assert(elem_type->size > 0);
    assert(mc_is_pow_of_two(elem_type->alignment));
    self->data = NULL;
    self->len = 0;
    self->capacity = 0;
    self->elem_type = elem_type;
}

static void shrink_to_empty(struct mc_array *self)
{
    if (self->capacity > 0) {
        mc_aligned_free(self->data);
        self->data = NULL;
        self->capacity = 0;
    }
}

void mc_array_destroy(struct mc_array *self)
{
    assert(self);
    mc_array_truncate(self, 0);
    shrink_to_empty(self);
    self->elem_type = NULL;
}

size_t mc_array_len(const struct mc_array *self)
{
    assert(self);
    return self->len;
}

size_t mc_array_capacity(const struct mc_array *self)
{
    assert(self);
    return self->capacity;
}

bool mc_array_is_empty(const struct mc_array *self)
{
    assert(self);
    return self->len == 0;
}

void *mc_array_get(struct mc_array *self, size_t index)
{
    assert(self);
    return index < self->len ? mc_array_get_unchecked(self, index) : NULL;
}

void *mc_array_get_unchecked(struct mc_array *self, size_t index)
{
    assert(self);
    return mc_ptr_add(self->data, self->elem_type->size * index);
}

void *mc_array_get_first(struct mc_array *self)
{
    assert(self);
    return self->len > 0 ? mc_array_get_unchecked(self, 0) : NULL;
}

void *mc_array_get_last(struct mc_array *self)
{
    assert(self);
    return self->len > 0 ? mc_array_get_unchecked(self, self->len - 1) : NULL;
}

static void grow_if_needed(struct mc_array *self)
{
    if (self->len < self->capacity)
        return;

    mc_array_reserve(self, 1);
}

static void place_one(struct mc_array *self, size_t index, void *elem)
{
    self->elem_type->move_ctor(mc_array_get_unchecked(self, index), elem);
}

static void place_range(struct mc_array *self, size_t index, void *elems,
                        size_t len)
{
    if (len == 0)
        return;

    mc_move_construct_func move_ctor = self->elem_type->move_ctor;
    size_t elem_size = self->elem_type->size;
    char *dst = mc_array_get_unchecked(self, index);
    char *src = elems;

    for (size_t i = 0; i < len; ++i) {
        move_ctor(dst, src);
        dst += elem_size;
        src += elem_size;
    }
}

void mc_array_push(struct mc_array *self, void *elem)
{
    assert(self);
    assert(elem);

    grow_if_needed(self);

    place_one(self, self->len, elem);

    ++self->len;
}

static void extract_one(struct mc_array *self, size_t index, void *out_elem)
{
    void *elem = mc_array_get_unchecked(self, index);

    if (out_elem)
        self->elem_type->move_ctor(out_elem, elem);
    else if (self->elem_type->destroy)
        self->elem_type->destroy(elem);
}

static void extract_range(struct mc_array *self, size_t index, size_t len,
                          void *out_elems, size_t out_elems_len)
{
    if (len == 0)
        return;

    size_t elem_size = self->elem_type->size;
    char *data = mc_array_get_unchecked(self, index);
    size_t move_count = 0;

    if (out_elems && out_elems_len > 0) {
        mc_move_construct_func move_ctor = self->elem_type->move_ctor;
        char *dst = out_elems;
        move_count = len < out_elems_len ? len : out_elems_len;
        for (size_t i = 0; i < move_count; ++i) {
            move_ctor(dst, data);
            dst += elem_size;
            data += elem_size;
        }
    }

    mc_destroy_func destroy = self->elem_type->destroy;
    if (destroy) {
        index += move_count;

        char *remaining = mc_array_get_unchecked(self, index);

        for (size_t i = move_count; i < len; ++i) {
            destroy(remaining);
            remaining += elem_size;
        }
    }
}

bool mc_array_pop(struct mc_array *self, void *out_elem)
{
    assert(self);

    if (self->len == 0)
        return false;

    extract_one(self, self->len--, out_elem);

    return true;
}

static void shift(struct mc_array *self, size_t dst, size_t src, size_t n)
{
    if (n == 0)
        return;

    memmove(mc_array_get_unchecked(self, dst),
            mc_array_get_unchecked(self, src), n * self->elem_type->size);
}

static void bounds_check(const char *func_name, size_t index, size_t bounds,
                         bool allow_equal)
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

void mc_array_insert(struct mc_array *self, size_t index, void *elem)
{
    assert(self);
    assert(elem);

    size_t len = self->len;

    /* Allow insertion at the end. */
    bounds_check(__func__, index, len, true);

    grow_if_needed(self);

    shift(self, index + 1, index, len - index);

    place_one(self, index, elem);

    ++self->len;
}

void mc_array_remove(struct mc_array *self, size_t index, void *out_elem)
{
    assert(self);

    size_t len = self->len;

    bounds_check(__func__, index, len, false);

    extract_one(self, index, out_elem);

    shift(self, index, index + 1, len - index - 1);

    --self->len;
}

void mc_array_append_range(struct mc_array *self, void *elems, size_t elems_len)
{
    assert(self);
    assert(elems_len == 0 || elems);

    if (elems_len == 0)
        return;

    mc_array_reserve(self, elems_len);

    place_range(self, self->len, elems, elems_len);

    self->len += elems_len;
}

void mc_array_insert_range(struct mc_array *self, size_t index, void *elems,
                           size_t elems_len)
{
    assert(self);
    assert(elems_len == 0 || elems);

    if (elems_len == 0)
        return;

    size_t self_len = self->len;

    /* Allow insertion at the end. */
    bounds_check(__func__, index, self_len, true);

    mc_array_reserve(self, elems_len);

    shift(self, index + elems_len, index, self_len - index);

    place_range(self, index, elems, elems_len);

    self->len += elems_len;
}

void mc_array_remove_range(struct mc_array *self, size_t index, size_t len,
                           void *out_elems, size_t out_elems_len)
{
    assert(self);

    if (len == 0)
        return;

    size_t self_len = self->len;

    bounds_check(__func__, index, self_len, false);

    if (index + len > self_len)
        len = self_len - index;

    extract_range(self, index, len, out_elems, out_elems_len);

    shift(self, index, index + len, self_len - index - len);

    self->len -= len;
}

void mc_array_clear(struct mc_array *self)
{
    assert(self);
    mc_array_truncate(self, 0);
}

static void set_capacity(struct mc_array *self, size_t capacity)
{
    if (capacity == 0) {
        shrink_to_empty(self);
        return;
    }

    size_t elem_align = self->elem_type->alignment;
    size_t elem_size = self->elem_type->size;
    size_t total_size = capacity * elem_size;

    void *new_data = mc_aligned_malloc(elem_align, total_size);
    if (!new_data) {
        fprintf(stderr, "memory allocation of %zu bytes failed\n", total_size);
        abort();
    }

    memcpy(new_data, self->data, self->len * elem_size);

    mc_aligned_free(self->data);
    self->data = new_data;
    self->capacity = capacity;
}

void mc_array_reserve(struct mc_array *self, size_t additional)
{
    assert(self);

    if (additional > SIZE_MAX / self->elem_type->size - self->len) {
        fprintf(stderr, "capacity overflow\n");
        abort();
    }

    set_capacity(self, mc_max2(self->len + additional, self->capacity * 2));
}

void mc_array_reserve_exact(struct mc_array *self, size_t additional)
{
    assert(self);

    if (additional > SIZE_MAX / self->elem_type->size - self->len) {
        fprintf(stderr, "capacity overflow\n");
        abort();
    }

    set_capacity(self, self->len + additional);
}

void mc_array_shrink_to_fit(struct mc_array *self)
{
    assert(self);
    set_capacity(self, self->len);
}

void mc_array_shrink_to(struct mc_array *self, size_t capacity)
{
    assert(self);

    if (capacity < self->len)
        return;

    if (capacity >= self->capacity)
        return;

    set_capacity(self, capacity);
}

void mc_array_truncate(struct mc_array *self, size_t len)
{
    assert(self);

    if (len >= self->len)
        return;

    mc_destroy_func destroy = self->elem_type->destroy;
    if (destroy)
        FOR_EACH(curr, self, len, self->len, { destroy(curr); });

    self->len = len;
}

void mc_array_resize(struct mc_array *self, size_t len, void *elem)
{
    assert(self);

    mc_copy_construct_func copy_ctor =
        mc_type_get_copy_ctor_forced(__func__, self->elem_type);

    if (len <= self->len) {
        mc_array_truncate(self, len);
        if (elem && self->elem_type->destroy)
            self->elem_type->destroy(elem);
        return;
    }

    assert(elem);

    mc_array_reserve(self, len - self->len);

    FOR_EACH(curr, self, self->len, len - 1, { copy_ctor(curr, elem); });

    place_one(self, len - 1, elem);

    self->len = len;
}

bool mc_array_contains(struct mc_array *self, const void *elem)
{
    assert(self);
    assert(elem);

    mc_equal_func eq = mc_type_get_equal_forced(__func__, self->elem_type);
    FOR_EACH(curr, self, 0, self->len, {
        if (eq(curr, elem))
            return true;
    });

    return false;
}

void mc_array_sort(struct mc_array *self)
{
    assert(self);
    mc_compare_func cmp = mc_type_get_compare_forced(__func__, self->elem_type);
    mc_array_sort_with(self, cmp);
}

void mc_array_sort_with(struct mc_array *self, mc_compare_func cmp)
{
    assert(self);
    assert(cmp);

    if (self->len < 2)
        return;

    qsort(self->data, self->len, self->elem_type->size, cmp);
}

bool mc_array_binary_search(struct mc_array *self, const void *elem,
                            size_t *out_index)
{
    assert(self);
    assert(elem);

    mc_compare_func cmp = mc_type_get_compare_forced(__func__, self->elem_type);

    if (self->len < 1)
        return false;

    size_t lo = 0, hi = self->len - 1;
    while (lo <= hi) {
        size_t mid = lo + (hi - lo) / 2;
        int res = cmp(mc_array_get_unchecked(self, mid), elem);
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

void mc_array_for_each(struct mc_array *self,
                       void (*func)(void *elem, void *user_data),
                       void *user_data)
{
    assert(self);
    assert(func);
    FOR_EACH(curr, self, 0, self->len, { func(curr, user_data); });
}
