#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myclib/array.h"
#include "myclib/test.h"
#include "myclib/type.h"
#include "myclib/hash.h"

struct test_struct {
    int id;
    char name[20];
};

static int test_struct_compare(void const *a, void const *b)
{
    struct test_struct const *ta = a;
    struct test_struct const *tb = b;
    if (ta->id < tb->id)
        return -1;
    if (ta->id > tb->id)
        return 1;
    return 0;
}

static bool test_struct_equal(void const *a, void const *b)
{
    struct test_struct const *ta = a;
    struct test_struct const *tb = b;
    return ta->id == tb->id;
}

static size_t test_struct_hash(void const *data)
{
    struct test_struct const *t = data;
    return (size_t)t->id;
}

MC_DEFINE_POD_TYPE(test_struct, struct test_struct, test_struct_compare,
                   test_struct_equal, test_struct_hash)

struct test_object {
    int id;
    char *name;
};

static char *mc_strdup(char const *str)
{
    size_t len = strlen(str) + 1;
    char *new_str = malloc(len);
    assert(new_str);
    memcpy(new_str, str, len);
    return new_str;
}

static void test_object_init(struct test_object *obj, int id, char const *name)
{
    obj->id = id;
    obj->name = mc_strdup(name);
}

static void test_object_cleanup(void *obj)
{
    struct test_object *o = obj;
    free(o->name);
    o->name = NULL;
    o->id = 0;
}

static void test_object_move(void *dst, void *src)
{
    struct test_object *d = dst;
    struct test_object *s = src;
    *d = *s;
    s->name = NULL;
}

static void test_object_copy(void *dst, void const *src)
{
    struct test_object *d = dst;
    struct test_object const *s = src;
    test_object_init(d, s->id, s->name);
}

static int test_object_compare(void const *a, void const *b)
{
    struct test_object const *ta = a;
    struct test_object const *tb = b;
    int cmp = strcmp(ta->name, tb->name);
    if (cmp != 0)
        return cmp;
    return ta->id - tb->id;
}

static bool test_object_equal(void const *a, void const *b)
{
    struct test_object const *ta = a;
    struct test_object const *tb = b;
    return ta->id == tb->id && strcmp(ta->name, tb->name) == 0;
}

static size_t test_object_hash(void const *data)
{
    struct test_object const *t = data;
    return MC_HASH(t->name, strlen(t->name));
}

MC_DEFINE_TYPE(test_object, struct test_object, test_object_cleanup,
               test_object_move, test_object_copy, test_object_compare,
               test_object_equal, test_object_hash)

MC_TEST_SUITE(array);

MC_TEST_IN_SUITE(array, init)
{
    struct mc_array array;
    mc_array_init(&array, int32_get_mc_type());
    MC_ASSERT(mc_array_len(&array) == 0);
    MC_ASSERT(mc_array_capacity(&array) == 0);
    MC_ASSERT(mc_array_is_empty(&array) == true);
    MC_ASSERT(array.elem_type == int32_get_mc_type());

    mc_array_cleanup(&array);
}

static bool is_even(void const *elem, void const *user_data)
{
    (void)user_data;
    int val = *(int *)elem;
    return val % 2 == 0;
}

MC_TEST_IN_SUITE(array, search_functions)
{
    struct mc_array array;
    int values[] = {10, 20, 30, 40, 50};
    mc_array_from(&array, int32_get_mc_type(), values, 5);

    int val = 30;
    int not_val = 35;
    MC_ASSERT(mc_array_contains(&array, &val) == true);
    MC_ASSERT(mc_array_contains(&array, &not_val) == false);

    int *found = mc_array_find(&array, &val);
    MC_ASSERT(found != NULL);
    MC_ASSERT(*found == 30);

    int *not_found = mc_array_find(&array, &not_val);
    MC_ASSERT(not_found == NULL);

    int *even = mc_array_find_if(&array, is_even, NULL);
    MC_ASSERT(even != NULL);
    MC_ASSERT(*even % 2 == 0);

    mc_array_cleanup(&array);
}

static int desc_compare(void const *a, void const *b)
{
    int val_a = *(int *)a;
    int val_b = *(int *)b;
    return val_b - val_a;
}

MC_TEST_IN_SUITE(array, sort_functions)
{
    struct mc_array array;
    mc_array_init(&array, int32_get_mc_type());

    int unsorted[] = {50, 10, 30, 20, 40};
    for (size_t i = 0; i < 5; i++) {
        mc_array_push(&array, &unsorted[i]);
    }

    mc_array_sort(&array);

    int sorted[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == sorted[i]);
    }

    mc_array_sort_with(&array, desc_compare);

    int reversed[] = {50, 40, 30, 20, 10};
    for (size_t i = 0; i < 5; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == reversed[i]);
    }

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, binary_search)
{
    struct mc_array array;
    int values[] = {10, 20, 30, 40, 50};
    mc_array_from(&array, int32_get_mc_type(), values, 5);

    mc_array_sort(&array);

    size_t index;
    int search_val;

    search_val = 30;
    MC_ASSERT(mc_array_binary_search(&array, &search_val, &index) == true);
    MC_ASSERT(index == 2);

    search_val = 35;
    MC_ASSERT(mc_array_binary_search(&array, &search_val, NULL) == false);

    search_val = 10;
    MC_ASSERT(mc_array_binary_search(&array, &search_val, &index) == true);
    MC_ASSERT(index == 0);

    search_val = 50;
    MC_ASSERT(mc_array_binary_search(&array, &search_val, &index) == true);
    MC_ASSERT(index == 4);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, copy_and_move)
{
    struct mc_array src, dst;
    int values[] = {10, 20, 30, 40, 50};
    mc_array_from(&src, int32_get_mc_type(), values, 5);

    mc_array_copy(&dst, &src);

    MC_ASSERT(mc_array_len(&dst) == mc_array_len(&src));
    MC_ASSERT(mc_array_equal(&dst, &src) == true);

    int new_val = 100;
    mc_array_push(&src, &new_val);
    MC_ASSERT(mc_array_len(&src) == 6);
    MC_ASSERT(mc_array_len(&dst) == 5);

    mc_array_cleanup(&dst);

    mc_array_move(&dst, &src);

    MC_ASSERT(mc_array_len(&dst) == 6);
    MC_ASSERT(mc_array_len(&src) == 0);
    MC_ASSERT(mc_array_capacity(&src) == 0);
    MC_ASSERT(src.data == NULL);

    int expected_values[] = {10, 20, 30, 40, 50, 100};
    for (size_t i = 0; i < 6; i++) {
        MC_ASSERT(*(int *)mc_array_get(&dst, i) == expected_values[i]);
    }

    mc_array_cleanup(&src);
    mc_array_cleanup(&dst);
}

MC_TEST_IN_SUITE(array, compare_and_equal)
{
    struct mc_array array1, array2;

    int values1[] = {10, 20, 30};
    int values2[] = {10, 20, 30};
    mc_array_from(&array1, int32_get_mc_type(), values1, 3);
    mc_array_from(&array2, int32_get_mc_type(), values2, 3);

    MC_ASSERT(mc_array_equal(&array1, &array2) == true);
    MC_ASSERT(mc_array_compare(&array1, &array2) == 0);

    int val = 40;
    mc_array_push(&array1, &val);
    MC_ASSERT(mc_array_equal(&array1, &array2) == false);
    MC_ASSERT(mc_array_compare(&array1, &array2) > 0);
    MC_ASSERT(mc_array_compare(&array2, &array1) < 0);

    mc_array_cleanup(&array1);
    mc_array_cleanup(&array2);

    int values3[] = {10, 20, 30};
    int values4[] = {10, 25, 30};
    mc_array_from(&array1, int32_get_mc_type(), values3, 3);
    mc_array_from(&array2, int32_get_mc_type(), values4, 3);

    MC_ASSERT(mc_array_equal(&array1, &array2) == false);
    MC_ASSERT(mc_array_compare(&array1, &array2) < 0);
    MC_ASSERT(mc_array_compare(&array2, &array1) > 0);

    mc_array_cleanup(&array1);
    mc_array_cleanup(&array2);
}

MC_TEST_IN_SUITE(array, hash_function)
{
    struct mc_array array1, array2;
    int values[] = {10, 20, 30, 40, 50};

    mc_array_from(&array1, int32_get_mc_type(), values, 5);
    mc_array_from(&array2, int32_get_mc_type(), values, 5);

    size_t hash1 = mc_array_hash(&array1);
    size_t hash2 = mc_array_hash(&array2);
    MC_ASSERT(hash1 == hash2);

    int val = 100;
    mc_array_push(&array2, &val);
    size_t hash3 = mc_array_hash(&array2);
    MC_ASSERT(hash1 != hash3);

    mc_array_cleanup(&array1);
    mc_array_cleanup(&array2);
}

MC_TEST_IN_SUITE(array, test_object_basic_operations)
{
    struct mc_array array;
    mc_array_init(&array, test_object_get_mc_type());

    struct test_object obj1, obj2, obj3;
    test_object_init(&obj1, 1, "Object 1");
    test_object_init(&obj2, 2, "Object 2");
    test_object_init(&obj3, 3, "Object 3");

    mc_array_push(&array, &obj1);
    mc_array_push(&array, &obj2);
    mc_array_push(&array, &obj3);

    MC_ASSERT(mc_array_len(&array) == 3);

    struct test_object *elem1 = mc_array_get(&array, 0);
    struct test_object *elem2 = mc_array_get(&array, 1);
    struct test_object *elem3 = mc_array_get(&array, 2);

    MC_ASSERT(elem1 != NULL);
    MC_ASSERT(elem1->id == 1);
    MC_ASSERT(strcmp(elem1->name, "Object 1") == 0);

    MC_ASSERT(elem2 != NULL);
    MC_ASSERT(elem2->id == 2);
    MC_ASSERT(strcmp(elem2->name, "Object 2") == 0);

    MC_ASSERT(elem3 != NULL);
    MC_ASSERT(elem3->id == 3);
    MC_ASSERT(strcmp(elem3->name, "Object 3") == 0);

    struct test_object *first = mc_array_get_first(&array);
    struct test_object *last = mc_array_get_last(&array);

    MC_ASSERT(first == elem1);
    MC_ASSERT(last == elem3);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, test_object_copy_and_move)
{
    struct mc_array src, dst;
    mc_array_init(&src, test_object_get_mc_type());

    struct test_object obj1, obj2;
    test_object_init(&obj1, 1, "Source Object 1");
    test_object_init(&obj2, 2, "Source Object 2");

    mc_array_push(&src, &obj1);
    mc_array_push(&src, &obj2);

    mc_array_copy(&dst, &src);

    MC_ASSERT(mc_array_len(&dst) == mc_array_len(&src));
    MC_ASSERT(mc_array_equal(&dst, &src) == true);

    struct test_object *src_elem = mc_array_get(&src, 0);
    struct test_object *dst_elem = mc_array_get(&dst, 0);

    MC_ASSERT(src_elem != dst_elem);
    MC_ASSERT(src_elem->id == dst_elem->id);
    MC_ASSERT(strcmp(src_elem->name, dst_elem->name) == 0);

    free(src_elem->name);
    src_elem->name = mc_strdup("Modified Source");

    MC_ASSERT(strcmp(dst_elem->name, "Source Object 1") == 0);

    mc_array_cleanup(&dst);

    mc_array_move(&dst, &src);

    MC_ASSERT(mc_array_len(&dst) == 2);
    MC_ASSERT(mc_array_len(&src) == 0);
    MC_ASSERT(mc_array_capacity(&src) == 0);
    MC_ASSERT(src.data == NULL);

    struct test_object *moved_elem1 = mc_array_get(&dst, 0);
    struct test_object *moved_elem2 = mc_array_get(&dst, 1);

    MC_ASSERT(moved_elem1 != NULL);
    MC_ASSERT(moved_elem1->id == 1);
    MC_ASSERT(strcmp(moved_elem1->name, "Modified Source") == 0);

    MC_ASSERT(moved_elem2 != NULL);
    MC_ASSERT(moved_elem2->id == 2);
    MC_ASSERT(strcmp(moved_elem2->name, "Source Object 2") == 0);

    mc_array_cleanup(&src);
    mc_array_cleanup(&dst);
}

MC_TEST_IN_SUITE(array, test_object_compare_and_hash)
{
    struct mc_array array1, array2;
    mc_array_init(&array1, test_object_get_mc_type());
    mc_array_init(&array2, test_object_get_mc_type());

    struct test_object obj1a, obj1b, obj2a, obj2b;
    test_object_init(&obj1a, 1, "Test Object");
    test_object_init(&obj1b, 1, "Test Object");
    test_object_init(&obj2a, 2, "Another Object");
    test_object_init(&obj2b, 2, "Another Object");

    mc_array_push(&array1, &obj1a);
    mc_array_push(&array1, &obj2a);

    mc_array_push(&array2, &obj1b);
    mc_array_push(&array2, &obj2b);

    MC_ASSERT(mc_array_equal(&array1, &array2) == true);
    MC_ASSERT(mc_array_compare(&array1, &array2) == 0);

    size_t hash1 = mc_array_hash(&array1);
    size_t hash2 = mc_array_hash(&array2);
    MC_ASSERT(hash1 == hash2);

    struct test_object diff_obj;
    test_object_init(&diff_obj, 3, "Different Object");
    mc_array_push(&array2, &diff_obj);

    MC_ASSERT(mc_array_equal(&array1, &array2) == false);
    MC_ASSERT(mc_array_compare(&array1, &array2) < 0);

    size_t hash3 = mc_array_hash(&array2);
    MC_ASSERT(hash1 != hash3);

    mc_array_cleanup(&array1);
    mc_array_cleanup(&array2);

    struct mc_array sorted_array;
    mc_array_init(&sorted_array, test_object_get_mc_type());

    struct test_object obj_a, obj_b, obj_c;
    test_object_init(&obj_a, 100, "B Object");
    test_object_init(&obj_b, 200, "A Object");
    test_object_init(&obj_c, 300, "C Object");

    mc_array_push(&sorted_array, &obj_a);
    mc_array_push(&sorted_array, &obj_b);
    mc_array_push(&sorted_array, &obj_c);

    mc_array_sort(&sorted_array);

    struct test_object *sorted_elem1 = mc_array_get(&sorted_array, 0);
    struct test_object *sorted_elem2 = mc_array_get(&sorted_array, 1);
    struct test_object *sorted_elem3 = mc_array_get(&sorted_array, 2);

    MC_ASSERT(strcmp(sorted_elem1->name, "A Object") == 0);
    MC_ASSERT(strcmp(sorted_elem2->name, "B Object") == 0);
    MC_ASSERT(strcmp(sorted_elem3->name, "C Object") == 0);

    mc_array_cleanup(&sorted_array);
}

MC_TEST_IN_SUITE(array, capacity_management)
{
    struct mc_array array;
    mc_array_init(&array, int32_get_mc_type());

    mc_array_reserve(&array, 10);
    MC_ASSERT(mc_array_capacity(&array) >= 10);
    MC_ASSERT(mc_array_len(&array) == 0);

    size_t current_cap = mc_array_capacity(&array);
    mc_array_reserve_exact(&array, 5);
    MC_ASSERT(mc_array_capacity(&array) >= mc_array_len(&array) + 5);

    int values[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < 5; i++) {
        mc_array_push(&array, &values[i]);
    }

    size_t before_shrink = mc_array_capacity(&array);
    mc_array_shrink_to_fit(&array);
    size_t after_shrink = mc_array_capacity(&array);

    MC_ASSERT(after_shrink <= before_shrink);
    MC_ASSERT(after_shrink >= 5);

    mc_array_shrink_to(&array, 10);
    if (mc_array_capacity(&array) > 10) {
        MC_ASSERT(mc_array_capacity(&array) >= 10);
    }

    size_t capacity_before = mc_array_capacity(&array);
    mc_array_shrink_to(&array, 3);
    MC_ASSERT(mc_array_capacity(&array) == capacity_before);

    mc_array_clear(&array);
    mc_array_reserve(&array, 100);
    size_t cap = mc_array_capacity(&array);

    for (int i = 0; i < 50; i++) {
        mc_array_push(&array, &i);
    }

    MC_ASSERT(mc_array_capacity(&array) == cap);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, boundary_conditions)
{
    struct mc_array array;
    mc_array_init(&array, int32_get_mc_type());

    MC_ASSERT(mc_array_is_empty(&array) == true);
    MC_ASSERT(mc_array_get(&array, 0) == NULL);
    MC_ASSERT(mc_array_get_first(&array) == NULL);
    MC_ASSERT(mc_array_get_last(&array) == NULL);

    int val;
    MC_ASSERT(mc_array_pop(&array, &val) == false);

    mc_array_remove_range(&array, 0, 0, NULL, 0);

    for (int i = 0; i < 1000; i++) {
        mc_array_push(&array, &i);
    }
    MC_ASSERT(mc_array_len(&array) == 1000);

    for (int i = 995; i < 1000; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == i);
    }

    mc_array_clear(&array);
    MC_ASSERT(mc_array_len(&array) == 0);

    int default_val = -1;
    mc_array_resize(&array, 5, &default_val);
    MC_ASSERT(mc_array_len(&array) == 5);
    for (size_t i = 0; i < 5; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == -1);
    }

    mc_array_resize(&array, 2, &default_val);
    MC_ASSERT(mc_array_len(&array) == 2);

    mc_array_truncate(&array, 0);
    MC_ASSERT(mc_array_len(&array) == 0);

    mc_array_truncate(&array, 10);
    MC_ASSERT(mc_array_len(&array) == 0);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, push_pop)
{
    struct mc_array array;
    mc_array_init(&array, int32_get_mc_type());

    int values[] = {1, 2, 3, 4, 5};
    for (size_t i = 0; i < 5; i++) {
        mc_array_push(&array, &values[i]);
        MC_ASSERT(mc_array_len(&array) == i + 1);
        MC_ASSERT(*(int *)mc_array_get_last(&array) == values[i]);
    }

    int popped;
    for (int i = 4; i >= 0; i--) {
        MC_ASSERT(mc_array_pop(&array, &popped) == true);
        MC_ASSERT(popped == values[i]);
        MC_ASSERT(mc_array_len(&array) == i);
    }

    MC_ASSERT(mc_array_pop(&array, &popped) == false);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, insert_remove)
{
    struct mc_array array;
    mc_array_init(&array, int32_get_mc_type());

    int base[] = {10, 20, 40, 50};
    for (size_t i = 0; i < 4; i++) {
        mc_array_push(&array, &base[i]);
    }

    int insert_val = 30;
    mc_array_insert(&array, 2, &insert_val);
    MC_ASSERT(mc_array_len(&array) == 5);
    MC_ASSERT(*(int *)mc_array_get(&array, 2) == 30);

    int expected[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == expected[i]);
    }

    int start_val = 5;
    mc_array_insert(&array, 0, &start_val);
    MC_ASSERT(*(int *)mc_array_get(&array, 0) == 5);

    int end_val = 60;
    mc_array_insert(&array, 6, &end_val);
    MC_ASSERT(*(int *)mc_array_get(&array, 6) == 60);

    int removed;
    mc_array_remove(&array, 2, &removed);
    MC_ASSERT(removed == 20);
    MC_ASSERT(mc_array_len(&array) == 6);

    int after_remove[] = {5, 10, 30, 40, 50, 60};
    for (size_t i = 0; i < 6; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == after_remove[i]);
    }

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, range_operations)
{
    struct mc_array array;
    mc_array_init(&array, int32_get_mc_type());

    int batch1[] = {1, 2, 3};
    mc_array_append_range(&array, batch1, 3);
    MC_ASSERT(mc_array_len(&array) == 3);

    int batch2[] = {4, 5, 6};
    mc_array_append_range(&array, batch2, 3);
    MC_ASSERT(mc_array_len(&array) == 6);

    int expected[] = {1, 2, 3, 4, 5, 6};
    for (size_t i = 0; i < 6; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == expected[i]);
    }

    int insert_batch[] = {10, 20};
    mc_array_insert_range(&array, 2, insert_batch, 2);
    MC_ASSERT(mc_array_len(&array) == 8);

    int after_insert[] = {1, 2, 10, 20, 3, 4, 5, 6};
    for (size_t i = 0; i < 8; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == after_insert[i]);
    }

    int out_batch[3];
    mc_array_remove_range(&array, 2, 3, out_batch, 3);
    MC_ASSERT(mc_array_len(&array) == 5);

    int after_remove[] = {1, 2, 4, 5, 6};
    for (size_t i = 0; i < 5; i++) {
        MC_ASSERT(*(int *)mc_array_get(&array, i) == after_remove[i]);
    }

    MC_ASSERT(out_batch[0] == 10);
    MC_ASSERT(out_batch[1] == 20);
    MC_ASSERT(out_batch[2] == 3);

    mc_array_clear(&array);
    MC_ASSERT(mc_array_len(&array) == 0);
    MC_ASSERT(mc_array_is_empty(&array) == true);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, custom_type)
{
    struct mc_array array;
    mc_array_init(&array, test_struct_get_mc_type());

    struct test_struct data[] = {{1, "Item 1"}, {2, "Item 2"}, {3, "Item 3"}};

    for (size_t i = 0; i < 3; i++) {
        mc_array_push(&array, &data[i]);
    }

    MC_ASSERT(mc_array_len(&array) == 3);
    for (size_t i = 0; i < 3; i++) {
        struct test_struct *item = mc_array_get(&array, i);
        MC_ASSERT(item->id == data[i].id);
        MC_ASSERT(strcmp(item->name, data[i].name) == 0);
    }

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, init_with_capacity)
{
    struct mc_array array;
    mc_array_with_capacity(&array, int32_get_mc_type(), 10);
    MC_ASSERT(mc_array_len(&array) == 0);
    MC_ASSERT(mc_array_capacity(&array) >= 10);
    MC_ASSERT(mc_array_is_empty(&array) == true);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, init_from_array)
{
    int elems[] = {1, 2, 3, 4, 5};
    struct mc_array array;
    mc_array_from(&array, int32_get_mc_type(), elems, 5);

    MC_ASSERT(mc_array_len(&array) == 5);
    MC_ASSERT(mc_array_capacity(&array) >= 5);
    MC_ASSERT(mc_array_is_empty(&array) == false);

    for (size_t i = 0; i < 5; i++) {
        int *val = mc_array_get(&array, i);
        MC_ASSERT(*val == elems[i]);
    }

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, basic_properties)
{
    struct mc_array array;
    mc_array_init(&array, int32_get_mc_type());

    MC_ASSERT(mc_array_len(&array) == 0);
    MC_ASSERT(mc_array_capacity(&array) == 0);
    MC_ASSERT(mc_array_is_empty(&array) == true);

    int val = 42;
    mc_array_push(&array, &val);

    MC_ASSERT(mc_array_len(&array) == 1);
    MC_ASSERT(mc_array_capacity(&array) >= 1);
    MC_ASSERT(mc_array_is_empty(&array) == false);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, get_functions)
{
    int elems[] = {10, 20, 30, 40, 50};
    struct mc_array array;
    mc_array_from(&array, int32_get_mc_type(), elems, 5);

    for (size_t i = 0; i < 5; i++) {
        int *val = mc_array_get(&array, i);
        MC_ASSERT(*val == elems[i]);
    }

    MC_ASSERT(mc_array_get(&array, 10) == NULL);

    int *first = mc_array_get_unchecked(&array, 0);
    int *last = mc_array_get_unchecked(&array, 4);
    MC_ASSERT(*first == 10);
    MC_ASSERT(*last == 50);

    MC_ASSERT(*(int *)mc_array_get_first(&array) == 10);
    MC_ASSERT(*(int *)mc_array_get_last(&array) == 50);

    mc_array_clear(&array);
    MC_ASSERT(mc_array_get_first(&array) == NULL);
    MC_ASSERT(mc_array_get_last(&array) == NULL);

    mc_array_cleanup(&array);
}

MC_TEST_IN_SUITE(array, iter_functions)
{
    int elems[] = {1, 2, 3, 4, 5};
    struct mc_array array;
    mc_array_from(&array, int32_get_mc_type(), elems, 5);

    struct mc_iter iter;
    mc_array_iter_init(&iter, &array);

    size_t i = 0;
    while (iter.next(&iter)) {
        int *val = iter.value;
        MC_ASSERT(*val == elems[i]);
        i++;
    }

    mc_array_cleanup(&array);
}

int main(void)
{
#if !MC_COMPILER_SUPPORTS_ATTRIBUTE
    register_test_suite_array();
    register_test_array_init();
    register_test_array_search_functions();
    register_test_array_sort_functions();
    register_test_array_binary_search();
    register_test_array_copy_and_move();
    register_test_array_compare_and_equal();
    register_test_array_hash_function();
    register_test_array_capacity_management();
    register_test_array_boundary_conditions();
    register_test_array_push_pop();
    register_test_array_insert_remove();
    register_test_array_range_operations();
    register_test_array_custom_type();
    register_test_array_init_with_capacity();
    register_test_array_init_from_array();
    register_test_array_basic_properties();
    register_test_array_get_functions();
    register_test_array_test_object_basic_operations();
    register_test_array_test_object_copy_and_move();
    register_test_array_test_object_compare_and_hash();
    register_test_array_iter_functions();
#endif
    return mc_run_all_tests();
}
