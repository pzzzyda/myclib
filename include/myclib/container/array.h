#ifndef MYCLIB_CONTAINER_ARRAY_H
#define MYCLIB_CONTAINER_ARRAY_H

#include "myclib/type.h"

/**
 * @struct mc_array
 *
 * @brief A generic dynamic array.
 *
 * Stores elements of a uniform type described by @ref mc_type.
 * Memory is managed automatically with growth strategy.
 */
struct mc_array {
    void *data;
    size_t len;
    size_t capacity;
    const struct mc_type *elem_type;
};

/**
 * @brief Initialize an empty array with the given element type.
 *
 * The array must be destroyed with @ref mc_array_destroy to prevent memory
 * leaks.
 *
 * @param self        Pointer to the array to initialize. Must not be NULL.
 * @param elem_type   Pointer to the type metadata for the element type. Must
 *                    not be NULL. Must describe a valid, fully defined type
 *                    (size > 0, alignment power of two, etc.).
 */
void mc_array_init(struct mc_array *self, const struct mc_type *elem_type);

/**
 * @brief Destroy the array and all its elements.
 *
 * Calls the element destructor (@ref mc_type::destroy) on each element,
 * then frees the internal buffer. After this call, the array is uninitialized
 * and must be re-initialized before use.
 *
 * @param self        Pointer to the array to destroy. Must not be NULL.
 */
void mc_array_destroy(struct mc_array *self);

/**
 * @brief Get the current number of elements in the array.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @return            The number of elements currently stored.
 */
size_t mc_array_len(const struct mc_array *self);

/**
 * @brief Get the current capacity of the array.
 *
 * Capacity is the number of elements the array can hold before needing to
 * reallocate.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @return            The current capacity.
 */
size_t mc_array_capacity(const struct mc_array *self);

/**
 * @brief Check if the array is empty.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @return            true if the array has zero elements, false otherwise.
 */
bool mc_array_is_empty(const struct mc_array *self);

/**
 * @brief Get a pointer to the element at the specified index.
 *
 * Performs bounds checking. Returns NULL if index >= length.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param index       Zero-based index of the element to access.
 * @return            Pointer to the element, or NULL if out of bounds.
 */
void *mc_array_get(struct mc_array *self, size_t index);

/**
 * @brief Get a pointer to the element at the specified index without bounds
 * checking.
 *
 * Undefined behavior if index >= length. Use only when bounds are guaranteed.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param index       Zero-based index of the element to access.
 * @return            Pointer to the element.
 */
void *mc_array_get_unchecked(struct mc_array *self, size_t index);

/**
 * @brief Get a pointer to the first element.
 *
 * Equivalent to mc_array_get(self, 0). Returns NULL if the array is empty.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @return            Pointer to the first element, or NULL if empty.
 */
void *mc_array_get_first(struct mc_array *self);

/**
 * @brief Get a pointer to the last element.
 *
 * Equivalent to mc_array_get(self, len - 1). Returns NULL if the array is
 * empty.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @return            Pointer to the last element, or NULL if empty.
 */
void *mc_array_get_last(struct mc_array *self);

/**
 * @brief Append a new element to the end of the array.
 *
 * The element is move-constructed into the array using @ref mc_type::move_ctor.
 * The source element may be left in a valid but unspecified state.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param elem        Pointer to the element to append. Must not be NULL.
 */
void mc_array_push(struct mc_array *self, void *elem);

/**
 * @brief Remove and return the last element from the array.
 *
 * If out_elem is not NULL, the removed element is move-constructed into it.
 * Otherwise, the element is destroyed.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param out_elem    Pointer to memory where the popped element will be placed,
 *                    or NULL to discard the element.
 * @return            true if an element was popped, false if the array was
 * empty.
 */
bool mc_array_pop(struct mc_array *self, void *out_elem);

/**
 * @brief Insert an element at the specified index.
 *
 * Shifts all elements at or after index one position to the right.
 * The element is move-constructed into the array.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param index       Index at which to insert. May be equal to len (append).
 * @param elem        Pointer to the element to insert. Must not be NULL.
 */
void mc_array_insert(struct mc_array *self, size_t index, void *elem);

/**
 * @brief Remove the element at the specified index.
 *
 * Shifts all elements after index one position to the left.
 * If out_elem is not NULL, the removed element is move-constructed into it.
 * Otherwise, the element is destroyed.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param index       Index of the element to remove. Must be < len.
 * @param out_elem    Pointer to memory where the removed element will be
 *                    placed, or NULL to discard the element.
 */
void mc_array_remove(struct mc_array *self, size_t index, void *out_elem);

/**
 * @brief Append multiple elements to the end of the array.
 *
 * Constructs each element in the array using @ref mc_type::move_ctor.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param elems       Pointer to the first element in the range to append.
 *                    Ignored if elems_len is 0.
 * @param elems_len   Number of elements to append.
 */
void mc_array_append_range(struct mc_array *self, void *elems,
                           size_t elems_len);

/**
 * @brief Insert multiple elements at the specified index.
 *
 * Shifts existing elements to make room. Elements are move-constructed from the
 * range.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param index       Index at which to insert. May be equal to len (append).
 * @param elems       Pointer to the first element in the range to insert.
 *                    Ignored if elems_len is 0.
 * @param elems_len   Number of elements to insert.
 */
void mc_array_insert_range(struct mc_array *self, size_t index, void *elems,
                           size_t elems_len);

/**
 * @brief Remove a range of elements from the array.
 *
 * Shifts subsequent elements to fill the gap.
 * If out_elems is provided, up to out_elems_len elements are move-constructed
 * into it. Remaining removed elements are destroyed.
 *
 * @param self           Pointer to the array. Must not be NULL.
 * @param index          Starting index of the range to remove.
 * @param len            Number of elements to remove.
 * @param out_elems      Pointer to memory where removed elements will be
 *                       placed, or NULL to discard them.
 * @param out_elems_len  Capacity of the out_elems buffer in number of elements.
 */
void mc_array_remove_range(struct mc_array *self, size_t index, size_t len,
                           void *out_elems, size_t out_elems_len);

/**
 * @brief Remove all elements from the array.
 *
 * Destroys every element using @ref mc_type::destroy.
 * Does not change the capacity.
 *
 * @param self        Pointer to the array. Must not be NULL.
 */
void mc_array_clear(struct mc_array *self);

/**
 * @brief Ensure the array can hold at least 'additional' more elements.
 *
 * Increases capacity if necessary. Uses a growth strategy (e.g., doubling)
 * to amortize allocation costs.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param additional  Minimum number of additional elements to accommodate.
 */
void mc_array_reserve(struct mc_array *self, size_t additional);

/**
 * @brief Reserve exact capacity for 'additional' more elements.
 *
 * Sets capacity to len + additional. Avoids over-allocation.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param additional  Exact number of additional elements to reserve space for.
 */
void mc_array_reserve_exact(struct mc_array *self, size_t additional);

/**
 * @brief Shrink the capacity to match the current length.
 *
 * Frees excess memory. No effect if capacity already equals length.
 *
 * @param self        Pointer to the array. Must not be NULL.
 */
void mc_array_shrink_to_fit(struct mc_array *self);

/**
 * @brief Reduce the capacity to at least 'capacity'.
 *
 * If current capacity is less than or equal to the requested capacity, does
 * nothing. Otherwise, reallocates to the requested capacity.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param capacity    Desired capacity. Must be >= len.
 */
void mc_array_shrink_to(struct mc_array *self, size_t capacity);

/**
 * @brief Truncate the array to the specified length.
 *
 * Removes elements from the end. Destroys any removed elements.
 * If new_len >= len, does nothing.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param len         New length of the array.
 */
void mc_array_truncate(struct mc_array *self, size_t len);

/**
 * @brief Resize the array to the specified length.
 *
 * If new_len > len, appends copies of 'elem' to reach the new length.
 * If new_len <= len, truncates the array and destroys excess elements.
 * The 'elem' argument is always destroyed after use (if non-NULL).
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param len         New length of the array.
 * @param elem        Pointer to the element used for padding. Must not be NULL
 *                    if len > current length.
 */
void mc_array_resize(struct mc_array *self, size_t len, void *elem);

/**
 * @brief Check if the array contains the specified element.
 *
 * Requires the element type to have an equality function (@ref mc_type::equal).
 * Uses linear search.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param elem        Pointer to the element to search for. Must not be NULL.
 * @return            true if the element is found, false otherwise.
 */
bool mc_array_contains(struct mc_array *self, const void *elem);

/**
 * @brief Sort the array in ascending order.
 *
 * Requires the element type to have a comparison function (@ref
 * mc_type::compare).
 *
 * @param self        Pointer to the array. Must not be NULL.
 */
void mc_array_sort(struct mc_array *self);

/**
 * @brief Sort the array using a custom comparison function.
 *
 * The comparison function must define a strict weak ordering.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param cmp         Function to compare two elements. Must not be NULL.
 */
void mc_array_sort_with(struct mc_array *self, mc_compare_func cmp);

/**
 * @brief Perform a binary search on the sorted array.
 *
 * Finds the index of an element equal to 'elem'.
 * The array must be sorted according to the same ordering as 'cmp'.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param elem        Pointer to the element to search for. Must not be NULL.
 * @param out_index   Pointer to receive the index of the found element, or NULL
 *                    if not needed.
 * @return            true if the element is found, false otherwise.
 */
bool mc_array_binary_search(struct mc_array *self, const void *elem,
                            size_t *out_index);

/**
 * @brief Apply a function to every element in the array.
 *
 * The function is called with a pointer to each element and the user data.
 *
 * @param self        Pointer to the array. Must not be NULL.
 * @param func        Function to apply to each element. Must not be NULL.
 * @param user_data   Arbitrary data passed to the function.
 */
void mc_array_for_each(struct mc_array *self,
                       void (*func)(void *elem, void *user_data),
                       void *user_data);

#endif
