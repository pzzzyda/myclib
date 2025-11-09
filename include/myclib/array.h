#ifndef MYCLIB_ARRAY_H
#define MYCLIB_ARRAY_H

#include "myclib/type.h"

struct mc_array {
    struct mc_type const *elem_type;
    void *data;
    size_t len;
    size_t capacity;
};

#define MC_ARRAY_INITIALIZER(type)                                             \
    {.elem_type = type, .data = NULL, .len = 0, .capacity = 0}

MC_DECLARE_TYPE(mc_array);

void mc_array_init(struct mc_array *array, struct mc_type const *elem_type);
void mc_array_with_capacity(struct mc_array *array,
                            struct mc_type const *elem_type, size_t capacity);
void mc_array_from(struct mc_array *array, struct mc_type const *elem_type,
                   void *elems, size_t elems_len);

void mc_array_destroy(struct mc_array *array);

size_t mc_array_len(struct mc_array const *array);
size_t mc_array_capacity(struct mc_array const *array);
bool mc_array_is_empty(struct mc_array const *array);

void *mc_array_get(struct mc_array const *array, size_t index);
void *mc_array_get_unchecked(struct mc_array const *array, size_t index);
void *mc_array_get_first(struct mc_array const *array);
void *mc_array_get_last(struct mc_array const *array);

void mc_array_push(struct mc_array *array, void *elem);
bool mc_array_pop(struct mc_array *array, void *out_elem);
void mc_array_insert(struct mc_array *array, size_t index, void *elem);
void mc_array_remove(struct mc_array *array, size_t index, void *out_elem);
void mc_array_append_range(struct mc_array *array, void *elems,
                           size_t elems_len);
void mc_array_insert_range(struct mc_array *array, size_t index, void *elems,
                           size_t elems_len);
void mc_array_remove_range(struct mc_array *array, size_t index, size_t len,
                           void *out_elems, size_t out_elems_len);
void mc_array_clear(struct mc_array *array);

void mc_array_reserve(struct mc_array *array, size_t additional);
void mc_array_reserve_exact(struct mc_array *array, size_t additional);
void mc_array_shrink_to_fit(struct mc_array *array);
void mc_array_shrink_to(struct mc_array *array, size_t capacity);
void mc_array_truncate(struct mc_array *array, size_t len);
void mc_array_resize(struct mc_array *array, size_t len, void *elem);

bool mc_array_contains(struct mc_array const *array, void const *elem);
void *mc_array_find(struct mc_array const *array, void const *elem);
void *mc_array_find_if(struct mc_array const *array,
                       bool (*pred)(void const *, void const *user_data),
                       void const *user_data);
bool mc_array_binary_search(struct mc_array const *array, void const *elem,
                            size_t *out_index);

void mc_array_sort(struct mc_array const *array);
void mc_array_sort_with(struct mc_array const *array, mc_compare_func cmp);

void mc_array_for_each(struct mc_array const *array,
                       void (*func)(void *elem, void *user_data),
                       void *user_data);

void mc_array_move(struct mc_array *dst, struct mc_array *src);
void mc_array_copy(struct mc_array *dst, struct mc_array const *src);
int mc_array_compare(struct mc_array const *array1,
                     struct mc_array const *array2);
bool mc_array_equal(struct mc_array const *array1,
                    struct mc_array const *array2);
size_t mc_array_hash(struct mc_array const *array);

#endif
