#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myclib/list.h"
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

MC_TEST_SUITE(list);

MC_TEST_IN_SUITE(list, init)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());
    MC_ASSERT_EQ_SIZE(mc_list_len(&list), 0);
    MC_ASSERT_TRUE(mc_list_is_empty(&list));
    MC_ASSERT_EQ_PTR(list.elem_type, int32_get_mc_type());
    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, push_back)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&list, &values[i]);
        MC_ASSERT_EQ_SIZE(mc_list_len(&list), i + 1);
    }

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, push_front)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_front(&list, &values[i]);
        MC_ASSERT(mc_list_len(&list) == i + 1);
    }

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, pop_back)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&list, &values[i]);
    }

    int popped;
    for (int i = 4; i >= 0; i--) {
        MC_ASSERT(mc_list_pop_back(&list, &popped) == true);
        MC_ASSERT(popped == values[i]);
        MC_ASSERT(mc_list_len(&list) == i);
    }

    MC_ASSERT(mc_list_pop_back(&list, &popped) == false);

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, pop_front)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&list, &values[i]);
    }

    int popped;
    for (int i = 0; i < 5; i++) {
        MC_ASSERT(mc_list_pop_front(&list, &popped) == true);
        MC_ASSERT(popped == values[i]);
        MC_ASSERT(mc_list_len(&list) == 4 - i);
    }

    MC_ASSERT(mc_list_pop_front(&list, &popped) == false);

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, insert)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    int base[] = {10, 20, 40, 50};
    for (size_t i = 0; i < 4; i++) {
        mc_list_push_back(&list, &base[i]);
    }

    int insert_val = 30;
    mc_list_insert(&list, 2, &insert_val);
    MC_ASSERT(mc_list_len(&list) == 5);

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, remove)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&list, &values[i]);
    }

    int removed;
    mc_list_remove(&list, 2, &removed);
    MC_ASSERT(removed == 30);
    MC_ASSERT(mc_list_len(&list) == 4);

    mc_list_cleanup(&list);
}

static int for_each_sum = 0;

static void sum_func(void *elem, void *user_data)
{
    (void)user_data;
    for_each_sum += *(int *)elem;
}

MC_TEST_IN_SUITE(list, for_each)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&list, &values[i]);
    }

    for_each_sum = 0;
    mc_list_for_each(&list, sum_func, NULL);
    MC_ASSERT(for_each_sum == 150);

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, clear)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&list, &values[i]);
    }

    MC_ASSERT(mc_list_len(&list) == 5);
    mc_list_clear(&list);
    MC_ASSERT(mc_list_len(&list) == 0);
    MC_ASSERT(mc_list_is_empty(&list) == true);

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, move)
{
    struct mc_list src, dst;
    mc_list_init(&src, int32_get_mc_type());
    mc_list_init(&dst, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&src, &values[i]);
    }

    mc_list_move(&dst, &src);
    MC_ASSERT(mc_list_len(&dst) == 5);
    MC_ASSERT(mc_list_len(&src) == 0);

    mc_list_cleanup(&src);
    mc_list_cleanup(&dst);
}

MC_TEST_IN_SUITE(list, copy)
{
    struct mc_list src, dst;
    mc_list_init(&src, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&src, &values[i]);
    }

    mc_list_copy(&dst, &src);
    MC_ASSERT_EQ_SIZE(mc_list_len(&dst), mc_list_len(&src));
    MC_ASSERT_TRUE(mc_list_equal(&dst, &src));

    mc_list_cleanup(&src);
    mc_list_cleanup(&dst);
}

MC_TEST_IN_SUITE(list, compare)
{
    struct mc_list list1, list2;
    mc_list_init(&list1, int32_get_mc_type());
    mc_list_init(&list2, int32_get_mc_type());

    int values[] = {10, 20, 30};
    for (size_t i = 0; i < 3; i++) {
        mc_list_push_back(&list1, &values[i]);
        mc_list_push_back(&list2, &values[i]);
    }

    MC_ASSERT(mc_list_compare(&list1, &list2) == 0);

    int extra = 40;
    mc_list_push_back(&list1, &extra);
    MC_ASSERT(mc_list_compare(&list1, &list2) > 0);
    MC_ASSERT(mc_list_compare(&list2, &list1) < 0);

    mc_list_cleanup(&list1);
    mc_list_cleanup(&list2);
}

MC_TEST_IN_SUITE(list, equal)
{
    struct mc_list list1, list2;
    mc_list_init(&list1, int32_get_mc_type());
    mc_list_init(&list2, int32_get_mc_type());

    int values[] = {10, 20, 30};
    for (size_t i = 0; i < 3; i++) {
        mc_list_push_back(&list1, &values[i]);
        mc_list_push_back(&list2, &values[i]);
    }

    MC_ASSERT(mc_list_equal(&list1, &list2) == true);

    int different = 35;
    mc_list_remove(&list2, 2, NULL);
    mc_list_push_back(&list2, &different);
    MC_ASSERT(mc_list_equal(&list1, &list2) == false);

    mc_list_cleanup(&list1);
    mc_list_cleanup(&list2);
}

MC_TEST_IN_SUITE(list, hash)
{
    struct mc_list list1, list2;
    mc_list_init(&list1, int32_get_mc_type());
    mc_list_init(&list2, int32_get_mc_type());

    int values[] = {10, 20, 30, 40, 50};
    for (size_t i = 0; i < 5; i++) {
        mc_list_push_back(&list1, &values[i]);
        mc_list_push_back(&list2, &values[i]);
    }

    size_t hash1 = mc_list_hash(&list1);
    size_t hash2 = mc_list_hash(&list2);
    MC_ASSERT(hash1 == hash2);

    int extra = 60;
    mc_list_push_back(&list2, &extra);
    size_t hash3 = mc_list_hash(&list2);
    MC_ASSERT(hash1 != hash3);

    mc_list_cleanup(&list1);
    mc_list_cleanup(&list2);
}

MC_TEST_IN_SUITE(list, test_struct_basic_operations)
{
    struct mc_list list;
    mc_list_init(&list, test_struct_get_mc_type());

    struct test_struct obj1 = {1, "Item 1"};
    struct test_struct obj2 = {2, "Item 2"};
    struct test_struct obj3 = {3, "Item 3"};

    mc_list_push_back(&list, &obj1);
    mc_list_push_back(&list, &obj2);
    mc_list_push_back(&list, &obj3);

    MC_ASSERT(mc_list_len(&list) == 3);

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, test_object_basic_operations)
{
    struct mc_list list;
    mc_list_init(&list, test_object_get_mc_type());

    struct test_object obj1, obj2, obj3;
    test_object_init(&obj1, 1, "Object 1");
    test_object_init(&obj2, 2, "Object 2");
    test_object_init(&obj3, 3, "Object 3");

    mc_list_push_back(&list, &obj1);
    mc_list_push_back(&list, &obj2);
    mc_list_push_back(&list, &obj3);

    MC_ASSERT(mc_list_len(&list) == 3);

    mc_list_cleanup(&list);
}

MC_TEST_IN_SUITE(list, boundary_conditions)
{
    struct mc_list list;
    mc_list_init(&list, int32_get_mc_type());

    MC_ASSERT(mc_list_is_empty(&list) == true);

    int val;
    MC_ASSERT(mc_list_pop_back(&list, &val) == false);
    MC_ASSERT(mc_list_pop_front(&list, &val) == false);

    mc_list_clear(&list);
    MC_ASSERT(mc_list_len(&list) == 0);

    for (int i = 0; i < 1000; i++) {
        mc_list_push_back(&list, &i);
    }
    MC_ASSERT(mc_list_len(&list) == 1000);

    mc_list_cleanup(&list);
}

int main(void)
{
#if !MC_COMPILER_SUPPORTS_ATTRIBUTE
    register_test_suite_list();
    register_test_list_init();
    register_test_list_push_back();
    register_test_list_push_front();
    register_test_list_pop_back();
    register_test_list_pop_front();
    register_test_list_insert();
    register_test_list_remove();
    register_test_list_for_each();
    register_test_list_clear();
    register_test_list_move();
    register_test_list_copy();
    register_test_list_compare();
    register_test_list_equal();
    register_test_list_hash();
    register_test_list_test_struct_basic_operations();
    register_test_list_test_object_basic_operations();
    register_test_list_boundary_conditions();
#endif
    return mc_run_all_tests();
}
