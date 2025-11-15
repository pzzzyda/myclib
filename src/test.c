#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "myclib/test.h"
#include "myclib/map.h"
#include "myclib/time.h"
#include "myclib/type.h"
#include "myclib/array.h"

#define MC_COLOR_RESET "\033[0m"
#define MC_COLOR_RED "\033[31m"
#define MC_COLOR_GREEN "\033[32m"

struct mc_test_state {
    struct mc_map suites;
    struct mc_array failed_tests;
    size_t num_of_suites_run;
    size_t num_of_tests_run;
    bool current_test_failed;
    bool color_output_enabled;
};

static struct mc_test_state test_state;

static void mc_test_print_info(char const *type, char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (test_state.color_output_enabled)
        printf(MC_COLOR_GREEN "[ %-8s ] " MC_COLOR_RESET, type);
    else
        printf("[ %-8s ] ", type);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

static void mc_test_print_error(char const *type, char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (test_state.color_output_enabled)
        printf(MC_COLOR_RED);
    printf("[ %-8s ] ", type);
    vprintf(fmt, args);
    if (test_state.color_output_enabled)
        printf(MC_COLOR_RESET);
    printf("\n");
    va_end(args);
}

static void mc_test_print_separator(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    if (test_state.color_output_enabled)
        printf(MC_COLOR_GREEN "[----------] " MC_COLOR_RESET);
    else
        printf("[----------] ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

static void mc_test_state_init(struct mc_test_state *state)
{
    mc_map_init(&state->suites, str_get_mc_type(), &mc_test_suite_mc_type);
    mc_array_init(&state->failed_tests, &mc_test_entry_mc_type);
    state->num_of_suites_run = 0;
    state->num_of_tests_run = 0;
    state->current_test_failed = false;
    state->color_output_enabled = true;
}

static void mc_test_state_cleanup(struct mc_test_state *state)
{
    mc_map_cleanup(&state->suites);
    mc_array_cleanup(&state->failed_tests);
    memset(state, 0, sizeof(struct mc_test_state));
}

static struct mc_test_suite *
mc_test_state_find_suite(struct mc_test_state *state, char const *suite_name)
{
    return mc_map_get(&state->suites, &suite_name);
}

static void mc_test_state_register_suite(struct mc_test_state *state,
                                         struct mc_test_suite *suite)
{
    if (strcmp(suite->name, "global") == 0) {
        mc_test_print_error("ERROR",
                            "Cannot register suite with name 'global'");
        return;
    }

    if (mc_map_contains_key(&state->suites, &suite->name)) {
        mc_test_print_error("ERROR", "Suite '%s' already registered",
                            suite->name);
        return;
    }

    mc_map_insert(&state->suites, &suite->name, suite);
}

static void mc_test_state_register_test(struct mc_test_state *state,
                                        char const *suite_name,
                                        struct mc_test_entry *entry)
{
    struct mc_test_suite *suite = mc_test_state_find_suite(state, suite_name);
    if (!suite) {
        mc_test_print_error("ERROR", "No suite found with name '%s'",
                            suite_name);
        return;
    }

    if (mc_map_contains_key(&suite->tests, &entry->name)) {
        mc_test_print_error("ERROR",
                            "Test '%s' already registered in suite '%s'",
                            entry->name, suite_name);
        return;
    }

    mc_map_insert(&suite->tests, &entry->name, entry);
}

MC_DEFINE_EXTERNAL_POD_TYPE(mc_test_entry, struct mc_test_entry, NULL, NULL,
                            NULL)

MC_DEFINE_EXTERNAL_TYPE(mc_test_suite, struct mc_test_suite,
                        mc_test_suite_cleanup, mc_test_suite_move, NULL, NULL,
                        NULL, NULL)

static void mc_run_test(struct mc_test_state *state,
                        struct mc_test_entry *entry)
{
    state->current_test_failed = false;

    mc_test_print_info("RUNNING", "%s", entry->name);

    double start_time = mc_get_current_time_ms();
    entry->func();
    double end_time = mc_get_current_time_ms();
    double elapsed_time = end_time - start_time;

    char const *slow_marker = (elapsed_time > 100) ? " [SLOW]" : "";

    if (!state->current_test_failed) {
        mc_test_print_info("PASSED", "%s (%.3f ms)%s", entry->name,
                           elapsed_time, slow_marker);
    } else {
        mc_test_print_error("FAILED", "%s (%.3f ms)", entry->name,
                            elapsed_time);
        mc_array_push(&state->failed_tests, entry);
    }

    state->num_of_tests_run++;
}

static void mc_run_test_cb(void const *key, void *value, void *user_data)
{
    (void)key;
    struct mc_test_state *state = user_data;
    struct mc_test_entry *entry = value;
    mc_run_test(state, entry);
}

static void mc_print_test_suite_header(struct mc_test_suite const *suite)
{
    struct mc_map const *tests = &suite->tests;
    size_t total_tests = mc_map_len(tests);
    mc_test_print_separator("running %zu tests from suite '%s'", total_tests,
                            suite->name);
}

static void mc_print_test_suite_footer(struct mc_test_suite const *suite)
{
    struct mc_map const *tests = &suite->tests;
    size_t total_tests = mc_map_len(tests);
    double elapsed_time = suite->end_time - suite->start_time;
    mc_test_print_separator(
        "completed %zu tests from suite '%s' (%.3f ms total)", total_tests,
        suite->name, elapsed_time);
}

static void mc_run_test_suite_setup(struct mc_test_suite const *suite)
{
    if (suite->setup) {
        mc_test_print_info("SETUP", "running setup for suite '%s'",
                           suite->name);
        suite->setup();
    }
}

static void mc_run_test_suite_teardown(struct mc_test_suite const *suite)
{
    if (suite->teardown) {
        mc_test_print_info("TEARDOWN", "running teardown for suite '%s'",
                           suite->name);
        suite->teardown();
    }
}

static void mc_run_suite(struct mc_test_state *state,
                         struct mc_test_suite *suite)
{
    mc_run_test_suite_setup(suite);

    struct mc_map *tests = &suite->tests;

    mc_print_test_suite_header(suite);

    suite->start_time = mc_get_current_time_ms();
    mc_map_for_each(tests, mc_run_test_cb, state);
    suite->end_time = mc_get_current_time_ms();

    mc_print_test_suite_footer(suite);

    mc_run_test_suite_teardown(suite);

    state->num_of_suites_run++;
}

static void mc_run_suite_cb(void const *key, void *value, void *user_data)
{
    (void)key;
    struct mc_test_state *state = user_data;
    struct mc_test_suite *suite = value;
    if (mc_map_is_empty(&suite->tests))
        return;
    mc_run_suite(state, suite);
}

static void mc_test_cleanup(void)
{
    mc_test_state_cleanup(&test_state);
}

static void mc_test_init(void)
{
    static bool initialized = false;
    if (initialized)
        return;

    mc_test_state_init(&test_state);
    atexit(mc_test_cleanup);

    struct mc_test_suite suite;
    mc_test_suite_init(&suite, "global");
    mc_map_insert(&test_state.suites, &suite.name, &suite);

    initialized = true;
}

int mc_run_all_tests(void)
{
    mc_test_init();

    mc_map_for_each(&test_state.suites, mc_run_suite_cb, &test_state);

    size_t failed_tests_count = mc_array_len(&test_state.failed_tests);
    size_t passed_tests_count =
        test_state.num_of_tests_run - failed_tests_count;
    mc_test_print_info("PASSED", "%zu tests passed", passed_tests_count);

    if (failed_tests_count == 0) {
        mc_test_print_info("PASSED", "all tests passed");
        return 0;
    }

    mc_test_print_error("FAILED", "%zu tests failed", failed_tests_count);

    for (size_t i = 0; i < failed_tests_count; i++) {
        struct mc_test_entry *entry = mc_array_get(&test_state.failed_tests, i);
        mc_test_print_error("FAILED", "%s", entry->name);
    }

    return 1;
}

void mc_register_test(char const *suite_name, struct mc_test_entry *entry)
{
    assert(suite_name);
    assert(entry);

    mc_test_init();

    mc_test_state_register_test(&test_state, suite_name, entry);
}

void mc_register_suite(struct mc_test_suite *suite)
{
    assert(suite);

    mc_test_init();

    mc_test_state_register_suite(&test_state, suite);
}

void mc_set_test_suite_setup(char const *suite_name, void (*setup)(void))
{
    assert(suite_name);

    mc_test_init();

    struct mc_test_suite *suite =
        mc_test_state_find_suite(&test_state, suite_name);
    if (!suite) {
        mc_test_print_error("ERROR", "No suite found with name '%s'",
                            suite_name);
        return;
    }

    suite->setup = setup;
}

void mc_set_test_suite_teardown(char const *suite_name, void (*teardown)(void))
{
    assert(suite_name);

    mc_test_init();

    struct mc_test_suite *suite =
        mc_test_state_find_suite(&test_state, suite_name);
    if (!suite) {
        mc_test_print_error("ERROR", "No suite found with name '%s'",
                            suite_name);
        return;
    }

    suite->teardown = teardown;
}

void mc_assert_fail(char const *file, int line, char const *expr,
                    char const *fmt, ...)
{
    assert(file);
    assert(expr);
    mc_test_init();
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s:%d: assertion `%s` failed: ", file, line, expr);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    test_state.current_test_failed = true;
}

void mc_set_test_output_color(bool enable)
{
    mc_test_init();
    test_state.color_output_enabled = enable;
}

void mc_test_suite_init(struct mc_test_suite *suite, char const *name)
{
    suite->name = name;
    suite->setup = NULL;
    suite->teardown = NULL;
    mc_map_init(&suite->tests, str_get_mc_type(), &mc_test_entry_mc_type);
    suite->start_time = 0.0;
    suite->end_time = 0.0;
}

void mc_test_suite_cleanup(void *suite)
{
    struct mc_test_suite *s = suite;
    mc_map_cleanup(&s->tests);
}

void mc_test_suite_move(void *dst, void *src)
{
    struct mc_test_suite *d = dst;
    struct mc_test_suite *s = src;
    d->name = s->name;
    d->setup = s->setup;
    d->teardown = s->teardown;
    mc_map_move(&d->tests, &s->tests);
}
