#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myclib/string.h"
#include "myclib/test.h"
#include "myclib/array.h"

MC_TEST_SUITE(string);

MC_TEST_IN_SUITE(string, init_cleanup)
{
    /* Test basic initialization */
    struct mc_string str;
    mc_string_init(&str);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), 0);
    MC_ASSERT_EQ_SIZE(mc_string_capacity(&str), 0);
    MC_ASSERT_TRUE(mc_string_is_empty(&str));
    mc_string_cleanup(&str);

    /* Test initialization with from function */
    mc_string_from(&str, "hello world");
    char const *const expected1 = "hello world";
    size_t const expected_len1 = strlen(expected1);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len1);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected1);
    mc_string_cleanup(&str);

    /* Test initialization with NULL string */
    mc_string_from(&str, NULL);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), 0);
    MC_ASSERT_TRUE(mc_string_is_empty(&str));
    mc_string_cleanup(&str);

    /* Test initialization with bytes */
    char bytes[] = {104, 101, 108, 108, 111}; /* "hello" in ASCII */
    mc_string_from_bytes(&str, bytes, 5);
    char const *const expected2 = "hello";
    size_t const expected_len2 = strlen(expected2);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len2);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected2);
    mc_string_cleanup(&str);

    /* Test initialization with empty bytes */
    mc_string_from_bytes(&str, bytes, 0);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), 0);
    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, format)
{
    struct mc_string str;

    /* Test basic formatting */
    mc_string_format(&str, "Hello %s", "world");
    char const *const expected1 = "Hello world";
    size_t const expected_len1 = strlen(expected1);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len1);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected1);
    mc_string_cleanup(&str);

    /* Test numeric formatting */
    mc_string_format(&str, "Number: %d, Float: %.2f", 42, 3.14159);
    char const *const expected2 = "Number: 42, Float: 3.14";
    size_t const expected_len2 = strlen(expected2);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len2);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected2);
    mc_string_cleanup(&str);

    /* Test empty format string */
    mc_string_format(&str, "");
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), 0);
    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, append)
{
    struct mc_string str;
    mc_string_from(&str, "Hello");

    /* Test append string */
    mc_string_append(&str, " World");
    char const *const expected1 = "Hello World";
    size_t const expected_len1 = strlen(expected1);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len1);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected1);

    /* Test append NULL */
    mc_string_append(&str, NULL);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len1); /* No change */

    /* Test append bytes */
    char bytes[] = {33, 33}; /* "!!" in ASCII */
    mc_string_append_bytes(&str, bytes, 2);
    char const *const expected2 = "Hello World!!";
    size_t const expected_len2 = strlen(expected2);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len2);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected2);

    /* Test append empty bytes */
    mc_string_append_bytes(&str, bytes, 0);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len2); /* No change */

    /* Test append format */
    mc_string_append_format(&str, " %d %s", 42, "test");
    char const *const expected3 = "Hello World!! 42 test";
    size_t const expected_len3 = strlen(expected3);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len3);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected3);

    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, capacity)
{
    struct mc_string str;
    mc_string_init(&str);

    /* Test reserve */
    mc_string_reserve(&str, 20);
    MC_ASSERT_GE_SIZE(mc_string_capacity(&str), 20);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), 0);

    /* Test reserve exact */
    mc_string_cleanup(&str);
    mc_string_init(&str);
    mc_string_reserve_exact(&str, 15);
    MC_ASSERT_GE_SIZE(mc_string_capacity(&str), 15);

    /* Test shrink to fit */
    mc_string_append(&str, "hello");
    size_t orig_cap = mc_string_capacity(&str);
    mc_string_shrink_to_fit(&str);
    MC_ASSERT_LE_SIZE(mc_string_capacity(&str), orig_cap);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), 5);

    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, insert_remove)
{
    struct mc_string str;
    mc_string_from(&str, "Hello World");

    /* Test insert */
    mc_string_insert(&str, 5, " Beautiful");
    char const *const expected = "Hello Beautiful World";
    size_t const expected_len = strlen(expected);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected);

    /* Test insert at beginning */
    mc_string_insert(&str, 0, "Hi! ");
    char const *const expected2 = "Hi! Hello Beautiful World";
    size_t const expected_len2 = strlen(expected2);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len2);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected2);

    /* Test insert NULL */
    mc_string_insert(&str, 3, NULL);
    char const *const expected3 = "Hi! Hello Beautiful World";
    size_t const expected_len3 = strlen(expected3);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len3);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected3);

    /* Test remove */
    mc_string_remove(&str, "Beautiful ");
    char const *const expected4 = "Hi! Hello World";
    size_t const expected_len4 = strlen(expected4);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len4);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected4);

    /* Test remove non-existent substring */
    mc_string_remove(&str, "xyz");
    char const *const expected5 = "Hi! Hello World";
    size_t const expected_len5 = strlen(expected5);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len5);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected5);

    /* Test clear */
    mc_string_clear(&str);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), 0);
    MC_ASSERT_TRUE(mc_string_is_empty(&str));

    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, search)
{
    struct mc_string str;
    mc_string_from(&str, "Hello World, this is a test string");

    /* Test find */
    size_t index;
    MC_ASSERT_TRUE(mc_string_find(&str, "World", &index));
    MC_ASSERT_EQ_SIZE(index, 6);

    /* Test find non-existent */
    MC_ASSERT_FALSE(mc_string_find(&str, "xyz", &index));

    /* Test rfind */
    MC_ASSERT_TRUE(mc_string_rfind(&str, "is", &index));
    MC_ASSERT_EQ_SIZE(index, 18);

    /* Test find character */
    MC_ASSERT_TRUE(mc_string_find_ch(&str, 'W', &index));
    MC_ASSERT_EQ_SIZE(index, 6);

    /* Test rfind character */
    MC_ASSERT_TRUE(mc_string_rfind_ch(&str, 't', &index));
    MC_ASSERT_EQ_SIZE(index, 29);

    /* Test contains */
    MC_ASSERT_TRUE(mc_string_contains(&str, "test"));
    MC_ASSERT_FALSE(mc_string_contains(&str, "xyz"));

    /* Test contains character */
    MC_ASSERT_TRUE(mc_string_contains_ch(&str, 's'));
    MC_ASSERT_FALSE(mc_string_contains_ch(&str, 'z'));

    /* Test starts with */
    MC_ASSERT_TRUE(mc_string_starts_with(&str, "Hello"));
    MC_ASSERT_FALSE(mc_string_starts_with(&str, "Hi"));

    /* Test ends with */
    MC_ASSERT_TRUE(mc_string_ends_with(&str, "string"));
    MC_ASSERT_FALSE(mc_string_ends_with(&str, "test"));

    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, transform)
{
    struct mc_string str;

    /* Test replace */
    mc_string_from(&str, "Hello World");
    mc_string_replace(&str, "World", "Universe");
    char const *const expected1 = "Hello Universe";
    size_t const expected_len1 = strlen(expected1);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len1);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected1);

    /* Test replace with empty string */
    mc_string_replace(&str, "Hello ", "");
    char const *const expected2 = "Universe";
    size_t const expected_len2 = strlen(expected2);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len2);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected2);

    mc_string_cleanup(&str);

    /* Test to upper */
    mc_string_from(&str, "Hello World");
    mc_string_to_upper(&str);
    char const *const expected3 = "HELLO WORLD";
    size_t const expected_len3 = strlen(expected3);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len3);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected3);

    /* Test to lower */
    mc_string_to_lower(&str);
    char const *const expected4 = "hello world";
    size_t const expected_len4 = strlen(expected4);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len4);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected4);

    mc_string_cleanup(&str);

    /* Test trim */
    mc_string_from(&str, "   Hello World   ");
    mc_string_trim(&str);
    char const *const expected5 = "Hello World";
    size_t const expected_len5 = strlen(expected5);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len5);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected5);

    mc_string_clear(&str);
    mc_string_append(&str, "     ");
    mc_string_trim_left(&str);
    char const *const expected6 = "";
    size_t const expected_len6 = strlen(expected6);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len6);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected6);

    mc_string_clear(&str);
    mc_string_append(&str, "     ");
    mc_string_trim_right(&str);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len6);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected6);

    /* Test trim left */
    mc_string_cleanup(&str);
    mc_string_from(&str, "   Hello World");
    mc_string_trim_left(&str);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len5);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected5);

    /* Test trim right */
    mc_string_cleanup(&str);
    mc_string_from(&str, "Hello World   ");
    mc_string_trim_right(&str);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len5);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected5);

    /* Test strip prefix */
    mc_string_cleanup(&str);
    mc_string_from(&str, "Hello World");
    mc_string_strip_prefix(&str, "Hello ");
    char const *const expected7 = "World";
    size_t const expected_len7 = strlen(expected7);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len7);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected7);

    /* Test strip suffix */
    mc_string_cleanup(&str);
    mc_string_from(&str, "Hello World");
    mc_string_strip_suffix(&str, " World");
    char const *const expected8 = "Hello";
    size_t const expected_len8 = strlen(expected8);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len8);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected8);

    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, repeat)
{
    struct mc_string str;
    mc_string_from(&str, "ab");

    /* Test repeat 3 times */
    mc_string_repeat(&str, 3);
    char const *const expected1 = "ababab";
    size_t const expected_len1 = strlen(expected1);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len1);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected1);

    /* Test repeat 1 time (no change) */
    mc_string_repeat(&str, 1);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len1);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected1); /* No change */

    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, split)
{
    struct mc_string str;
    struct mc_array parts;

    /* Test split by space */
    mc_string_from(&str, "Hello World Test");
    mc_string_split(&str, " ", &parts);
    MC_ASSERT(mc_array_len(&parts) == 3);

    struct mc_string *part1 = mc_array_get(&parts, 0);
    struct mc_string *part2 = mc_array_get(&parts, 1);
    struct mc_string *part3 = mc_array_get(&parts, 2);

    MC_ASSERT(strcmp(mc_string_c_str(part1), "Hello") == 0);
    MC_ASSERT(strcmp(mc_string_c_str(part2), "World") == 0);
    MC_ASSERT(strcmp(mc_string_c_str(part3), "Test") == 0);

    mc_array_cleanup(&parts);

    /* Test split at specific index */
    struct mc_string left, right;
    mc_string_split_at(&str, 5, &left, &right);
    MC_ASSERT(strcmp(mc_string_c_str(&left), "Hello") == 0);
    MC_ASSERT(strcmp(mc_string_c_str(&right), " World Test") == 0);

    mc_string_cleanup(&left);
    mc_string_cleanup(&right);

    /* Test lines */
    mc_string_cleanup(&str);
    mc_string_from(&str, "Line 1\nLine 2\nLine 3");
    mc_string_lines(&str, &parts);
    MC_ASSERT(mc_array_len(&parts) == 3);

    part1 = mc_array_get(&parts, 0);
    part2 = mc_array_get(&parts, 1);
    part3 = mc_array_get(&parts, 2);

    MC_ASSERT(strcmp(mc_string_c_str(part1), "Line 1") == 0);
    MC_ASSERT(strcmp(mc_string_c_str(part2), "Line 2") == 0);
    MC_ASSERT(strcmp(mc_string_c_str(part3), "Line 3") == 0);

    mc_array_cleanup(&parts);
    mc_string_cleanup(&str);
}

MC_TEST_IN_SUITE(string, join)
{
    struct mc_array parts;
    mc_array_init(&parts, mc_string_get_mc_type());

    /* Create parts to join */
    struct mc_string part1, part2, part3;
    mc_string_from(&part1, "Hello");
    mc_string_from(&part2, "World");
    mc_string_from(&part3, "Test");

    mc_array_push(&parts, &part1);
    mc_array_push(&parts, &part2);
    mc_array_push(&parts, &part3);

    /* Test join with separator */
    struct mc_string joined;
    mc_string_join(&parts, ", ", &joined);
    MC_ASSERT(strcmp(mc_string_c_str(&joined), "Hello, World, Test") == 0);

    /* Test join with empty array */
    struct mc_array empty_parts;
    mc_array_init(&empty_parts, mc_string_get_mc_type());
    struct mc_string empty_joined;
    mc_string_join(&empty_parts, ", ", &empty_joined);
    MC_ASSERT(mc_string_len(&empty_joined) == 0);

    mc_string_cleanup(&joined);
    mc_string_cleanup(&empty_joined);
    mc_array_cleanup(&empty_parts);
    mc_array_cleanup(&parts);
    mc_string_cleanup(&part1);
    mc_string_cleanup(&part2);
    mc_string_cleanup(&part3);
}

MC_TEST_IN_SUITE(string, compare)
{
    struct mc_string str1, str2, str3;
    mc_string_from(&str1, "abc");
    mc_string_from(&str2, "abc");
    mc_string_from(&str3, "abd");

    /* Test equality */
    MC_ASSERT_TRUE(mc_string_equal(&str1, &str2));
    MC_ASSERT_FALSE(mc_string_equal(&str1, &str3));

    /* Test comparison */
    MC_ASSERT_EQ_INT(mc_string_compare(&str1, &str2), 0);
    MC_ASSERT_LT_INT(mc_string_compare(&str1, &str3), 0);
    MC_ASSERT_GT_INT(mc_string_compare(&str3, &str1), 0);

    /* Test with different lengths */
    mc_string_cleanup(&str3);
    mc_string_from(&str3, "abcd");
    MC_ASSERT_LT_INT(mc_string_compare(&str1, &str3), 0);

    mc_string_cleanup(&str1);
    mc_string_cleanup(&str2);
    mc_string_cleanup(&str3);
}

MC_TEST_IN_SUITE(string, move_copy)
{
    struct mc_string src, dst;
    mc_string_from(&src, "Hello World");

    /* Test move */
    mc_string_move(&dst, &src);
    char const *const expected = "Hello World";
    size_t const expected_len = strlen(expected);
    MC_ASSERT_EQ_SIZE(mc_string_len(&dst), expected_len);
    MC_ASSERT_EQ_STR(mc_string_c_str(&dst), expected);
    MC_ASSERT_NULL(src.data);
    MC_ASSERT_EQ_SIZE(src.len, 0);

    /* Test copy */
    struct mc_string copy;
    mc_string_copy(&copy, &dst);
    MC_ASSERT_EQ_SIZE(mc_string_len(&copy), expected_len);
    MC_ASSERT_EQ_STR(mc_string_c_str(&copy), expected);

    /* Verify original was not modified */
    MC_ASSERT_EQ_SIZE(mc_string_len(&dst), expected_len);
    MC_ASSERT_EQ_STR(mc_string_c_str(&dst), expected);

    mc_string_cleanup(&dst);
    mc_string_cleanup(&copy);
}

MC_TEST_IN_SUITE(string, hash)
{
    struct mc_string str1, str2;
    mc_string_from(&str1, "test");
    mc_string_from(&str2, "test");

    /* Identical strings should have same hash */
    MC_ASSERT_EQ_SIZE(mc_string_hash(&str1), mc_string_hash(&str2));

    /* Different strings should (probably) have different hashes */
    mc_string_cleanup(&str2);
    mc_string_from(&str2, "test1");
    MC_ASSERT_NE_SIZE(mc_string_hash(&str1), mc_string_hash(&str2));

    mc_string_cleanup(&str1);
    mc_string_cleanup(&str2);
}

MC_TEST_IN_SUITE(string, edge_cases)
{
    struct mc_string str;

    /* Test empty string operations */
    mc_string_init(&str);
    MC_ASSERT_NOT_NULL(mc_string_c_str(&str)); /* Should return valid pointer */
    char const *const expected = "";
    size_t const expected_len = strlen(expected);
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len);
    MC_ASSERT_EQ_STR(mc_string_c_str(&str), expected);

    /* Test find in empty string */
    size_t index;
    MC_ASSERT_FALSE(mc_string_find(&str, "x", &index));
    MC_ASSERT_TRUE(mc_string_find(&str, "", &index));
    MC_ASSERT_EQ_SIZE(index, 0);

    /* Test replace in empty string */
    mc_string_replace(&str, "x", "y");
    MC_ASSERT_EQ_SIZE(mc_string_len(&str), expected_len);

    mc_string_cleanup(&str);
}

int main(void)
{
#if !MC_COMPILER_SUPPORTS_ATTRIBUTE
    register_test_suite_string();
    register_test_string_init_cleanup();
    register_test_string_format();
    register_test_string_append();
    register_test_string_capacity();
    register_test_string_insert_remove();
    register_test_string_search();
    register_test_string_transform();
    register_test_string_repeat();
    register_test_string_split();
    register_test_string_join();
    register_test_string_compare();
    register_test_string_move_copy();
    register_test_string_hash();
    register_test_string_edge_cases();
#endif
    return mc_run_all_tests();
}
