#include "myclib/map.h"
#include "myclib/test.h"

MC_TEST_SUITE(map)

MC_TEST_IN_SUITE(map, init)
{
    struct mc_map map;
    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());
    MC_ASSERT(mc_map_len(&map) == 0);
    MC_ASSERT(mc_map_capacity(&map) == 0);
    MC_ASSERT(map.table.key_type == str_get_mc_type());
    MC_ASSERT(map.table.value_type == int_get_mc_type());
    mc_map_cleanup(&map);
}

MC_TEST_IN_SUITE(map, basic_properties)
{
    struct mc_map map;
    struct {
        char const *key;
        int value;
    } pairs[] = {{"one", 1}, {"two", 2}, {"three", 3}, {"four", 4}};
    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        mc_map_insert(&map, &pairs[i].key, &pairs[i].value);
    }

    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        int *value = mc_map_get(&map, &pairs[i].key);
        MC_ASSERT(value != NULL);
        MC_ASSERT(*value == pairs[i].value);
    }

    mc_map_cleanup(&map);
}

MC_TEST_IN_SUITE(map, insert_overwrite)
{
    struct mc_map map;
    char const *key = "test";
    int value1 = 10;
    int value2 = 20;

    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    /* Insert first value */
    mc_map_insert(&map, &key, &value1);
    MC_ASSERT(mc_map_len(&map) == 1);

    int *retrieved = mc_map_get(&map, &key);
    MC_ASSERT(retrieved != NULL);
    MC_ASSERT(*retrieved == value1);

    /* Overwrite with second value */
    mc_map_insert(&map, &key, &value2);
    MC_ASSERT(mc_map_len(&map) == 1);

    retrieved = mc_map_get(&map, &key);
    MC_ASSERT(retrieved != NULL);
    MC_ASSERT(*retrieved == value2);

    mc_map_cleanup(&map);
}

MC_TEST_IN_SUITE(map, remove)
{
    struct mc_map map;
    struct {
        char const *key;
        int value;
    } pairs[] = {{"one", 1}, {"two", 2}, {"three", 3}};
    char const *non_existent_key = "four";

    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    /* Insert test data */
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        mc_map_insert(&map, &pairs[i].key, &pairs[i].value);
    }

    MC_ASSERT(mc_map_len(&map) == 3);

    /* Test removing non-existent key */
    char const *out_key;
    int out_value;
    MC_ASSERT(!mc_map_remove(&map, &non_existent_key, &out_key, &out_value));
    MC_ASSERT(mc_map_len(&map) == 3);

    /* Test removing existing key */
    MC_ASSERT(mc_map_remove(&map, &pairs[1].key, &out_key, &out_value));
    MC_ASSERT(mc_map_len(&map) == 2);
    MC_ASSERT(strcmp(out_key, pairs[1].key) == 0);
    MC_ASSERT(out_value == pairs[1].value);

    /* Verify the key is no longer present */
    MC_ASSERT(mc_map_get(&map, &pairs[1].key) == NULL);

    /* Verify other keys are still present */
    MC_ASSERT(mc_map_get(&map, &pairs[0].key) != NULL);
    MC_ASSERT(mc_map_get(&map, &pairs[2].key) != NULL);

    mc_map_cleanup(&map);
}

MC_TEST_IN_SUITE(map, contains_key)
{
    struct mc_map map;
    struct {
        char const *key;
        int value;
    } pairs[] = {{"one", 1}, {"two", 2}};
    char const *non_existent_key = "three";

    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    /* Test on empty map */
    MC_ASSERT(!mc_map_contains_key(&map, &non_existent_key));

    /* Insert test data */
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        mc_map_insert(&map, &pairs[i].key, &pairs[i].value);
    }

    /* Test existing keys */
    MC_ASSERT(mc_map_contains_key(&map, &pairs[0].key));
    MC_ASSERT(mc_map_contains_key(&map, &pairs[1].key));

    /* Test non-existent key */
    MC_ASSERT(!mc_map_contains_key(&map, &non_existent_key));

    mc_map_cleanup(&map);
}

MC_TEST_IN_SUITE(map, clear)
{
    struct mc_map map;
    struct {
        char const *key;
        int value;
    } pairs[] = {{"one", 1}, {"two", 2}, {"three", 3}};

    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    /* Insert test data */
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        mc_map_insert(&map, &pairs[i].key, &pairs[i].value);
    }

    MC_ASSERT(mc_map_len(&map) == 3);

    /* Clear the map */
    mc_map_clear(&map);

    MC_ASSERT(mc_map_len(&map) == 0);
    MC_ASSERT(mc_map_is_empty(&map));

    /* Verify all keys are gone */
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        MC_ASSERT(mc_map_get(&map, &pairs[i].key) == NULL);
    }

    mc_map_cleanup(&map);
}

MC_TEST_IN_SUITE(map, reserve)
{
    struct mc_map map;

    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    /* Test initial state */
    MC_ASSERT(mc_map_capacity(&map) == 0);

    /* Reserve capacity */
    mc_map_reserve(&map, 10);

    /* Verify capacity is at least what we requested */
    MC_ASSERT(mc_map_capacity(&map) >= 10);

    /* Verify we can insert up to the reserved capacity */
    char const *keys[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        mc_map_insert(&map, &keys[i], &values[i]);
    }

    MC_ASSERT(mc_map_len(&map) == 10);

    /* Verify all values are correctly stored */
    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        int *value = mc_map_get(&map, &keys[i]);
        MC_ASSERT(value != NULL);
        MC_ASSERT(*value == values[i]);
    }

    mc_map_cleanup(&map);
}

MC_TEST_IN_SUITE(map, shrink_to_fit)
{
    struct mc_map map;

    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    /* Insert many elements to ensure capacity is large */
    char const *keys[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10"};
    int values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        mc_map_insert(&map, &keys[i], &values[i]);
    }

    size_t initial_capacity = mc_map_capacity(&map);
    MC_ASSERT(initial_capacity >= 10);

    /* Remove most elements */
    for (size_t i = 1; i < sizeof(keys) / sizeof(keys[0]); ++i) {
        mc_map_remove(&map, &keys[i], NULL, NULL);
    }

    MC_ASSERT(mc_map_len(&map) == 1);

    /* Shrink to fit */
    mc_map_shrink_to_fit(&map);

    /* Verify capacity is now smaller */
    size_t new_capacity = mc_map_capacity(&map);
    MC_ASSERT(new_capacity < initial_capacity);

    /* Verify the remaining element is still present */
    int *value = mc_map_get(&map, &keys[0]);
    MC_ASSERT(value != NULL);
    MC_ASSERT(*value == values[0]);

    mc_map_cleanup(&map);
}

/* Helper struct for for_each test */
struct for_each_test_data {
    size_t count;
    int sum;
};

static void for_each_callback(void const *key, void *value, void *user_data)
{
    (void)key; /* Unused */
    struct for_each_test_data *data = user_data;
    int *int_value = value;

    data->count++;
    data->sum += *int_value;
}

MC_TEST_IN_SUITE(map, for_each)
{
    struct mc_map map;
    struct {
        char const *key;
        int value;
    } pairs[] = {{"one", 1}, {"two", 2}, {"three", 3}};

    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    /* Insert test data */
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        mc_map_insert(&map, &pairs[i].key, &pairs[i].value);
    }

    /* Test for_each */
    struct for_each_test_data data = {0, 0};
    mc_map_for_each(&map, for_each_callback, &data);

    MC_ASSERT(data.count == 3);
    MC_ASSERT(data.sum == 6); /* 1 + 2 + 3 = 6 */

    mc_map_cleanup(&map);
}

MC_TEST_IN_SUITE(map, move)
{
    struct mc_map src_map;
    struct mc_map dst_map;
    struct {
        char const *key;
        int value;
    } pairs[] = {{"one", 1}, {"two", 2}};

    /* Initialize source map */
    mc_map_init(&src_map, str_get_mc_type(), int_get_mc_type());
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        mc_map_insert(&src_map, &pairs[i].key, &pairs[i].value);
    }

    /* Initialize destination map */
    mc_map_init(&dst_map, str_get_mc_type(), int_get_mc_type());

    /* Move from source to destination */
    mc_map_move(&dst_map, &src_map);

    /* Verify destination has the elements */
    MC_ASSERT(mc_map_len(&dst_map) == 2);
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        int *value = mc_map_get(&dst_map, &pairs[i].key);
        MC_ASSERT(value != NULL);
        MC_ASSERT(*value == pairs[i].value);
    }

    /* Verify source is empty */
    MC_ASSERT(mc_map_len(&src_map) == 0);
    MC_ASSERT(mc_map_capacity(&src_map) == 0);

    /* Cleanup both maps */
    mc_map_cleanup(&src_map);
    mc_map_cleanup(&dst_map);
}

MC_TEST_IN_SUITE(map, copy)
{
    struct mc_map src_map;
    struct mc_map dst_map;
    struct {
        char const *key;
        int value;
    } pairs[] = {{"one", 1}, {"two", 2}};

    /* Initialize source map */
    mc_map_init(&src_map, str_get_mc_type(), int_get_mc_type());
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        mc_map_insert(&src_map, &pairs[i].key, &pairs[i].value);
    }

    /* Initialize destination map */
    mc_map_init(&dst_map, str_get_mc_type(), int_get_mc_type());

    /* Copy from source to destination */
    mc_map_copy(&dst_map, &src_map);

    /* Verify destination has the same elements */
    MC_ASSERT(mc_map_len(&dst_map) == mc_map_len(&src_map));
    for (size_t i = 0; i < sizeof(pairs) / sizeof(pairs[0]); ++i) {
        int *value = mc_map_get(&dst_map, &pairs[i].key);
        MC_ASSERT(value != NULL);
        MC_ASSERT(*value == pairs[i].value);
    }

    /* Verify source is unchanged */
    MC_ASSERT(mc_map_len(&src_map) == 2);

    /* Cleanup both maps */
    mc_map_cleanup(&src_map);
    mc_map_cleanup(&dst_map);
}

MC_TEST_IN_SUITE(map, is_empty)
{
    struct mc_map map;
    char const *key = "test";
    int value = 10;

    mc_map_init(&map, str_get_mc_type(), int_get_mc_type());

    /* Test on empty map */
    MC_ASSERT(mc_map_is_empty(&map));

    /* Insert an element */
    mc_map_insert(&map, &key, &value);

    /* Test on non-empty map */
    MC_ASSERT(!mc_map_is_empty(&map));

    /* Remove the element */
    mc_map_remove(&map, &key, NULL, NULL);

    /* Test on empty map again */
    MC_ASSERT(mc_map_is_empty(&map));

    mc_map_cleanup(&map);
}

int main(void)
{
#if !MC_COMPILER_SUPPORTS_ATTRIBUTE
    register_test_suite_map();
    register_test_map_init();
    register_test_map_basic_properties();
    register_test_map_insert_overwrite();
    register_test_map_remove();
    register_test_map_contains_key();
    register_test_map_clear();
    register_test_map_reserve();
    register_test_map_shrink_to_fit();
    register_test_map_for_each();
    register_test_map_move();
    register_test_map_copy();
    register_test_map_is_empty();
#endif
    return mc_run_all_tests();
}
