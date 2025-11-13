#ifndef MYCLIB_TEST_H
#define MYCLIB_TEST_H

#include "myclib/map.h"
#include "myclib/type.h"
#include "myclib/utils.h"
#include "myclib/attribute.h"

struct mc_test_entry {
    char const *name;
    void (*func)(void);
};

struct mc_test_suite {
    char const *name;
    void (*setup)(void);
    void (*teardown)(void);
    struct mc_map tests;
    double start_time;
    double end_time;
};

MC_DECLARE_EXTERNAL_TYPE(mc_test_entry);
MC_DECLARE_EXTERNAL_TYPE(mc_test_suite);

int mc_run_all_tests(void);

void mc_register_test(char const *suite_name, struct mc_test_entry *entry);
void mc_register_suite(struct mc_test_suite *suite);

void mc_set_test_suite_setup(char const *suite_name, void (*setup)(void));
void mc_set_test_suite_teardown(char const *suite_name, void (*teardown)(void));

void mc_assert_fail(char const *file, int line, char const *expr,
                    char const *fmt, ...);

void mc_set_test_output_color(bool enable);

void mc_test_suite_init(struct mc_test_suite *suite, char const *name);
void mc_test_suite_cleanup(void *suite);
void mc_test_suite_move(void *dst, void *src);

#define MC_TEST(test_name)                                                     \
    static void MC_JOIN_UNDERSCORE(test, test_name)(void);                     \
    MC_ATTR_CONSTRUCTOR static void MC_JOIN_UNDERSCORE(register_test,          \
                                                       test_name)(void)        \
    {                                                                          \
        struct mc_test_entry MC_JOIN_UNDERSCORE(test_entry, test_name) = {     \
            .name = "global::" #test_name,                                     \
            .func = MC_JOIN_UNDERSCORE(test, test_name),                       \
        };                                                                     \
        mc_register_test("global",                                             \
                         &MC_JOIN_UNDERSCORE(test_entry, test_name));          \
    }                                                                          \
    static void MC_JOIN_UNDERSCORE(test, test_name)(void)

#define MC_TEST_SUITE(suite_name)                                              \
    MC_ATTR_CONSTRUCTOR static void MC_JOIN_UNDERSCORE(register_test_suite,    \
                                                       suite_name)(void)       \
    {                                                                          \
        struct mc_test_suite MC_JOIN_UNDERSCORE(test_suite, suite_name);       \
        mc_test_suite_init(&MC_JOIN_UNDERSCORE(test_suite, suite_name),        \
                           #suite_name);                                       \
        mc_register_suite(&MC_JOIN_UNDERSCORE(test_suite, suite_name));        \
    }

#define MC_TEST_IN_SUITE(suite_name, test_name)                                \
    static void MC_JOIN_UNDERSCORE(test, suite_name, test_name)(void);         \
    MC_ATTR_CONSTRUCTOR static void MC_JOIN_UNDERSCORE(                        \
        register_test, suite_name, test_name)(void)                            \
    {                                                                          \
        struct mc_test_entry MC_JOIN_UNDERSCORE(test_entry, suite_name,        \
                                                test_name) = {                 \
            .name = #suite_name "::" #test_name,                               \
            .func = MC_JOIN_UNDERSCORE(test, suite_name, test_name),           \
        };                                                                     \
        mc_register_test(#suite_name, &MC_JOIN_UNDERSCORE(                     \
                                          test_entry, suite_name, test_name)); \
    }                                                                          \
    static void MC_JOIN_UNDERSCORE(test, suite_name, test_name)(void)

#define MC_ASSERT(expr)                                                        \
    do {                                                                       \
        if (!(expr)) {                                                         \
            mc_assert_fail(__FILE__, __LINE__, #expr, "");                     \
            return;                                                            \
        }                                                                      \
    } while (0)

#define MC_ASSERT_TRUE(expr)                                                   \
    do {                                                                       \
        if (!(expr)) {                                                         \
            mc_assert_fail(__FILE__, __LINE__, #expr, "condition is false");   \
            return;                                                            \
        }                                                                      \
    } while (0)

#define MC_ASSERT_FALSE(expr)                                                  \
    do {                                                                       \
        if ((expr)) {                                                          \
            mc_assert_fail(__FILE__, __LINE__, #expr, "condition is true");    \
            return;                                                            \
        }                                                                      \
    } while (0)

#define MC_ASSERT_NULL(ptr)                                                    \
    do {                                                                       \
        if ((ptr) != NULL) {                                                   \
            mc_assert_fail(__FILE__, __LINE__, #ptr, "pointer is not null");   \
            return;                                                            \
        }                                                                      \
    } while (0)

#define MC_ASSERT_NOT_NULL(ptr)                                                \
    do {                                                                       \
        if ((ptr) == NULL) {                                                   \
            mc_assert_fail(__FILE__, __LINE__, #ptr, "pointer is null");       \
            return;                                                            \
        }                                                                      \
    } while (0)

#define MC_ASSERT_CMP_INT(a, cmp, b)                                           \
    do {                                                                       \
        int a_ = (a);                                                          \
        int b_ = (b);                                                          \
        if (!(a_ cmp b_)) {                                                    \
            mc_assert_fail(__FILE__, __LINE__, #a " " #cmp " " #b,             \
                           "\nleft: %d\nright: %d", a_, b_);                   \
            return;                                                            \
        }                                                                      \
    } while (0)
#define MC_ASSERT_EQ_INT(a, b) MC_ASSERT_CMP_INT(a, ==, b)
#define MC_ASSERT_NE_INT(a, b) MC_ASSERT_CMP_INT(a, !=, b)
#define MC_ASSERT_GT_INT(a, b) MC_ASSERT_CMP_INT(a, >, b)
#define MC_ASSERT_GE_INT(a, b) MC_ASSERT_CMP_INT(a, >=, b)
#define MC_ASSERT_LT_INT(a, b) MC_ASSERT_CMP_INT(a, <, b)
#define MC_ASSERT_LE_INT(a, b) MC_ASSERT_CMP_INT(a, <=, b)

#define MC_ASSERT_CMP_SIZE(a, cmp, b)                                          \
    do {                                                                       \
        size_t a_ = (a);                                                       \
        size_t b_ = (b);                                                       \
        if (!(a_ cmp b_)) {                                                    \
            mc_assert_fail(__FILE__, __LINE__, #a " " #cmp " " #b,             \
                           "\nleft: %zu\nright: %zu", a_, b_);                 \
            return;                                                            \
        }                                                                      \
    } while (0)
#define MC_ASSERT_EQ_SIZE(a, b) MC_ASSERT_CMP_SIZE(a, ==, b)
#define MC_ASSERT_NE_SIZE(a, b) MC_ASSERT_CMP_SIZE(a, !=, b)
#define MC_ASSERT_GT_SIZE(a, b) MC_ASSERT_CMP_SIZE(a, >, b)
#define MC_ASSERT_GE_SIZE(a, b) MC_ASSERT_CMP_SIZE(a, >=, b)
#define MC_ASSERT_LT_SIZE(a, b) MC_ASSERT_CMP_SIZE(a, <, b)
#define MC_ASSERT_LE_SIZE(a, b) MC_ASSERT_CMP_SIZE(a, <=, b)

#define MC_ASSERT_CMP_PTR(a, cmp, b)                                           \
    do {                                                                       \
        void const *a_ = (a);                                                  \
        void const *b_ = (b);                                                  \
        if (!(a_ cmp b_)) {                                                    \
            mc_assert_fail(__FILE__, __LINE__, #a " " #cmp " " #b,             \
                           "\nleft: %p\nright: %p", a_, b_);                   \
            return;                                                            \
        }                                                                      \
    } while (0)
#define MC_ASSERT_EQ_PTR(a, b) MC_ASSERT_CMP_PTR(a, ==, b)
#define MC_ASSERT_NE_PTR(a, b) MC_ASSERT_CMP_PTR(a, !=, b)
#define MC_ASSERT_GT_PTR(a, b) MC_ASSERT_CMP_PTR(a, >, b)
#define MC_ASSERT_GE_PTR(a, b) MC_ASSERT_CMP_PTR(a, >=, b)
#define MC_ASSERT_LT_PTR(a, b) MC_ASSERT_CMP_PTR(a, <, b)
#define MC_ASSERT_LE_PTR(a, b) MC_ASSERT_CMP_PTR(a, <=, b)

#define MC_ASSERT_CMP_STR(a, cmp, b)                                           \
    do {                                                                       \
        char const *a_ = (a);                                                  \
        char const *b_ = (b);                                                  \
        if (!(strcmp(a_, b_) cmp 0)) {                                         \
            mc_assert_fail(__FILE__, __LINE__, #a " " #cmp " " #b,             \
                           "\nleft: %s\nright: %s", a_, b_);                   \
            return;                                                            \
        }                                                                      \
    } while (0)
#define MC_ASSERT_EQ_STR(a, b) MC_ASSERT_CMP_STR(a, ==, b)
#define MC_ASSERT_NE_STR(a, b) MC_ASSERT_CMP_STR(a, !=, b)
#define MC_ASSERT_GT_STR(a, b) MC_ASSERT_CMP_STR(a, >, b)
#define MC_ASSERT_GE_STR(a, b) MC_ASSERT_CMP_STR(a, >=, b)
#define MC_ASSERT_LT_STR(a, b) MC_ASSERT_CMP_STR(a, <, b)
#define MC_ASSERT_LE_STR(a, b) MC_ASSERT_CMP_STR(a, <=, b)

#endif
