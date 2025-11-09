#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myclib/array.h"
#include "myclib/utils.h"
#include "myclib/type.h"

struct test_object {
    int id;
    char *name;
};

static void test_object_cleanup(void *obj)
{
    assert(obj);
    struct test_object *o = (struct test_object *)obj;
    if (o->name) {
        free(o->name);
        o->name = NULL;
    }
}

static void test_object_move(void *dst, void *src)
{
    assert(dst);
    assert(src);
    struct test_object *d = (struct test_object *)dst;
    struct test_object *s = (struct test_object *)src;

    d->id = s->id;
    d->name = s->name;

    s->id = 0;
    s->name = NULL;
}

static void test_object_copy(void *dst, const void *src)
{
    assert(dst);
    assert(src);
    struct test_object *d = (struct test_object *)dst;
    const struct test_object *s = (const struct test_object *)src;

    d->id = s->id;
    if (s->name) {
        d->name = strdup(s->name);
    } else {
        d->name = NULL;
    }
}

static int test_object_compare(const void *obj1, const void *obj2)
{
    assert(obj1);
    assert(obj2);
    const struct test_object *a = (const struct test_object *)obj1;
    const struct test_object *b = (const struct test_object *)obj2;

    if (a->id != b->id) {
        return a->id - b->id;
    }

    if (!a->name && !b->name) {
        return 0;
    }
    if (!a->name) {
        return -1;
    }
    if (!b->name) {
        return 1;
    }

    return strcmp(a->name, b->name);
}

static bool test_object_equal(const void *obj1, const void *obj2)
{
    assert(obj1);
    assert(obj2);
    const struct test_object *a = (const struct test_object *)obj1;
    const struct test_object *b = (const struct test_object *)obj2;

    if (a->id != b->id) {
        return false;
    }

    if (!a->name && !b->name) {
        return true;
    }
    if (!a->name || !b->name) {
        return false;
    }

    return strcmp(a->name, b->name) == 0;
}

static size_t test_object_hash(const void *obj)
{
    assert(obj);
    const struct test_object *o = (const struct test_object *)obj;
    size_t hash = (size_t)o->id;

    if (o->name) {
        const char *s = o->name;
        while (*s) {
            hash = (hash * 31) + (unsigned char)*s;
            s++;
        }
    }

    return hash;
}

MC_DEFINE_TYPE(test_object, struct test_object, test_object_cleanup,
               test_object_move, test_object_copy, test_object_compare,
               test_object_equal, test_object_hash)

static struct test_object create_test_object(int id, const char *name)
{
    struct test_object obj;
    obj.id = id;
    obj.name = name ? strdup(name) : NULL;
    return obj;
}

static void test_basic_operations(void)
{
    printf("=== Testing Basic Operations ===\n");

    struct mc_array arr;
    mc_array_init(&arr, int32_get_mc_type());

    printf("Initial length: %zu\n", mc_array_len(&arr));
    printf("Initial capacity: %zu\n", mc_array_capacity(&arr));
    printf("Is empty: %s\n", mc_array_is_empty(&arr) ? "true" : "false");

    int32_t values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        mc_array_push(&arr, &values[i]);
    }

    printf("After pushing 5 elements:\n");
    printf("Length: %zu\n", mc_array_len(&arr));
    printf("Capacity: %zu\n", mc_array_capacity(&arr));

    printf("Elements: ");
    for (size_t i = 0; i < mc_array_len(&arr); i++) {
        int32_t *val = mc_array_get(&arr, i);
        printf("%d ", *val);
    }
    printf("\n");

    int32_t *first = mc_array_get_first(&arr);
    int32_t *last = mc_array_get_last(&arr);
    printf("First element: %d\n", *first);
    printf("Last element: %d\n", *last);

    int32_t popped;
    if (mc_array_pop(&arr, &popped)) {
        printf("Popped element: %d\n", popped);
    }
    printf("After pop, length: %zu\n", mc_array_len(&arr));

    int32_t insert_val = 25;
    mc_array_insert(&arr, 2, &insert_val);
    printf("After inserting 25 at index 2:\nElements: ");
    for (size_t i = 0; i < mc_array_len(&arr); i++) {
        int32_t *val = mc_array_get(&arr, i);
        printf("%d ", *val);
    }
    printf("\n");

    int32_t removed;
    mc_array_remove(&arr, 1, &removed);
    printf("Removed element at index 1: %d\n", removed);
    printf("After remove, elements: ");
    for (size_t i = 0; i < mc_array_len(&arr); i++) {
        int32_t *val = mc_array_get(&arr, i);
        printf("%d ", *val);
    }
    printf("\n");

    int32_t search_val = 30;
    printf("Contains %d: %s\n", search_val,
           mc_array_contains(&arr, &search_val) ? "true" : "false");

    mc_array_sort(&arr);
    printf("After sorting: ");
    for (size_t i = 0; i < mc_array_len(&arr); i++) {
        int32_t *val = mc_array_get(&arr, i);
        printf("%d ", *val);
    }
    printf("\n");

    size_t index;
    if (mc_array_binary_search(&arr, &search_val, &index)) {
        printf("Found %d at index %zu\n", search_val, index);
    }

    mc_array_clear(&arr);
    printf("After clear, length: %zu\n", mc_array_len(&arr));
    printf("After clear, capacity: %zu\n", mc_array_capacity(&arr));

    mc_array_cleanup(&arr);
    printf("Array cleaned\n\n");
}

static void test_advanced_operations_for_each_handler(void *elem,
                                                      void *user_data)
{
    int32_t *val = (int32_t *)elem;
    char *prefix = (char *)user_data;
    printf("%s: %d\n", prefix, *val);
}

static void test_advanced_operations(void)
{
    printf("=== Testing Advanced Operations ===\n");

    struct mc_array arr;
    mc_array_init(&arr, int32_get_mc_type());

    mc_array_reserve(&arr, 10);
    printf("After reserving 10 elements, capacity: %zu\n",
           mc_array_capacity(&arr));

    int32_t values[] = {1, 2, 3, 4, 5};
    mc_array_append_range(&arr, values, sizeof(values) / sizeof(values[0]));
    printf("After appending range, length: %zu\n", mc_array_len(&arr));

    int32_t insert_values[] = {100, 200};
    mc_array_insert_range(&arr, 2, insert_values,
                          sizeof(insert_values) / sizeof(insert_values[0]));
    printf("After inserting range at index 2:\nElements: ");
    for (size_t i = 0; i < mc_array_len(&arr); i++) {
        int32_t *val = mc_array_get(&arr, i);
        printf("%d ", *val);
    }
    printf("\n");

    int32_t removed_values[3];
    mc_array_remove_range(&arr, 1, 3, removed_values, 3);
    printf("Removed values: ");
    for (size_t i = 0; i < 3; i++) {
        printf("%d ", removed_values[i]);
    }
    printf("\n");

    printf("After removing range, elements: ");
    for (size_t i = 0; i < mc_array_len(&arr); i++) {
        int32_t *val = mc_array_get(&arr, i);
        printf("%d ", *val);
    }
    printf("\n");

    mc_array_truncate(&arr, 2);
    printf("After truncating to 2 elements, length: %zu\n", mc_array_len(&arr));

    int32_t fill_value = 999;
    mc_array_resize(&arr, 5, &fill_value);
    printf("After resizing to 5 elements, elements: ");
    for (size_t i = 0; i < mc_array_len(&arr); i++) {
        int32_t *val = mc_array_get(&arr, i);
        printf("%d ", *val);
    }
    printf("\n");

    mc_array_resize(&arr, 3, NULL);
    printf("After resizing to 3 elements, length: %zu\n", mc_array_len(&arr));

    mc_array_shrink_to_fit(&arr);
    printf("After shrink to fit, capacity: %zu\n", mc_array_capacity(&arr));

    printf("Testing for_each:\n");
    mc_array_for_each(&arr, test_advanced_operations_for_each_handler,
                      (void *)"Element");

    mc_array_cleanup(&arr);
    printf("Array cleaned\n\n");
}

static void test_custom_object_type(void)
{
    printf("=== Testing Custom Object Type ===\n");

    struct mc_array arr;
    mc_array_init(&arr, test_object_get_mc_type());

    struct test_object obj1 = create_test_object(1, "Object 1");
    struct test_object obj2 = create_test_object(2, "Object 2");
    struct test_object obj3 = create_test_object(3, "Object 3");

    mc_array_push(&arr, &obj1);
    mc_array_push(&arr, &obj2);
    mc_array_push(&arr, &obj3);

    printf("After pushing 3 objects, length: %zu\n", mc_array_len(&arr));

    printf("Objects in array:\n");
    for (size_t i = 0; i < mc_array_len(&arr); i++) {
        struct test_object *obj = mc_array_get(&arr, i);
        printf("Object %zu: id=%d, name=%s\n", i, obj->id, obj->name);
    }

    struct test_object search_obj = create_test_object(2, "Object 2");
    printf("Contains object with id=2, name='Object 2': %s\n",
           mc_array_contains(&arr, &search_obj) ? "true" : "false");
    test_object_cleanup(&search_obj);

    struct test_object removed_obj;
    mc_array_remove(&arr, 1, &removed_obj);
    printf("Removed object: id=%d, name=%s\n", removed_obj.id,
           removed_obj.name);

    test_object_cleanup(&removed_obj);

    printf("Cleaning array with custom objects...\n");
    mc_array_cleanup(&arr);
    printf("Array with custom objects cleaned\n\n");
}

static void test_edge_cases(void)
{
    printf("=== Testing Edge Cases ===\n");

    struct mc_array arr;
    mc_array_init(&arr, int32_get_mc_type());

    int32_t value;
    printf("Pop from empty array: %s\n",
           mc_array_pop(&arr, &value) ? "succeeded" : "failed (expected)");

    printf("Get from index 0 on empty array: %s\n",
           mc_array_get(&arr, 0) == NULL ? "NULL (expected)" : "not NULL");

    mc_array_reserve_exact(&arr, 5);
    printf("After reserve_exact(5), capacity: %zu\n", mc_array_capacity(&arr));

    mc_array_push(&arr, &(int32_t){42});
    mc_array_shrink_to(&arr, 0);
    printf("After shrink_to(0) with elements, capacity: %zu\n",
           mc_array_capacity(&arr));

    mc_array_shrink_to(&arr, 10);
    printf("After shrink_to(10) with lower current capacity, capacity: %zu\n",
           mc_array_capacity(&arr));

    mc_array_cleanup(&arr);
    printf("Edge case array cleaned\n\n");
}

int main(void)
{
    printf("===== Dynamic Array Unit Tests =====\n\n");

    test_basic_operations();
    test_advanced_operations();
    test_custom_object_type();
    test_edge_cases();

    printf("===== All Tests Completed =====\n");
    return 0;
}
