#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include "myclib/string.h"
#include "myclib/hash.h"
#include "myclib/utils.h"

void mc_string_init(struct mc_string *str)
{
    assert(str);
    str->data = NULL;
    str->len = 0;
    str->capacity = 0;
}

void mc_string_from(struct mc_string *str, char const *s)
{
    assert(str);
    mc_string_init(str);
    if (!s)
        return;
    mc_string_append_bytes(str, s, strlen(s));
}

void mc_string_from_bytes(struct mc_string *str, void const *bytes, size_t len)
{
    assert(str);
    mc_string_init(str);
    mc_string_append_bytes(str, bytes, len);
}

void mc_string_format(struct mc_string *str, char const *fmt, ...)
{
    va_list args1, args2;

    assert(str);
    assert(fmt);

    mc_string_init(str);

    va_start(args1, fmt);
    va_copy(args2, args1);

    int const n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);

    mc_string_reserve(str, n + 1);

    vsnprintf(str->data, n + 1, fmt, args1);
    va_end(args1);

    str->len += n;
}

void mc_string_join(struct mc_array const *parts, char const *separator,
                    struct mc_string *str)
{
    assert(parts);
    assert(separator);
    assert(str);

    mc_string_init(str);

    size_t const arr_len = mc_array_len(parts);
    if (arr_len == 0)
        return;

    size_t const sep_len = strlen(separator);
    for (size_t i = 0; i < arr_len; ++i) {
        struct mc_string const *const part = mc_array_get_unchecked(parts, i);
        mc_string_append_bytes(str, part->data, part->len);
        if (i < arr_len - 1)
            mc_string_append_bytes(str, separator, sep_len);
    }
}

static void mc_string_deallocate(struct mc_string *str)
{
    if (str->capacity > 0) {
        free(str->data);
        str->data = NULL;
        str->capacity = 0;
    }
}

void mc_string_cleanup(struct mc_string *str)
{
    assert(str);
    mc_string_clear(str);
    mc_string_deallocate(str);
}

size_t mc_string_len(struct mc_string const *str)
{
    assert(str);
    return str->len;
}

size_t mc_string_capacity(struct mc_string const *str)
{
    assert(str);
    return str->capacity;
}

bool mc_string_is_empty(struct mc_string const *str)
{
    assert(str);
    return str->len == 0;
}

char const *mc_string_c_str(struct mc_string *str)
{
    assert(str);
    mc_string_reserve(str, 1);
    str->data[str->len] = '\0';
    return str->data;
}

void mc_string_append(struct mc_string *str, char const *s)
{
    assert(str);
    if (!s)
        return;
    mc_string_append_bytes(str, s, strlen(s));
}

void mc_string_append_bytes(struct mc_string *str, void const *bytes,
                            size_t len)
{
    assert(str);

    if (!bytes || !len)
        return;

    mc_string_reserve(str, len);
    memcpy(str->data + str->len, bytes, len);
    str->len += len;
    str->data[str->len] = '\0';
}

void mc_string_append_format(struct mc_string *str, char const *fmt, ...)
{
    va_list args1, args2;

    va_start(args1, fmt);
    va_copy(args2, args1);

    int const n = vsnprintf(NULL, 0, fmt, args2);
    va_end(args2);

    mc_string_reserve(str, n + 1);

    vsnprintf(str->data + str->len, n + 1, fmt, args1);
    va_end(args1);

    str->len += n;
}

static void mc_string_bounds_check(char const *func_name, size_t index,
                                   size_t bounds, bool allow_equal)
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

void mc_string_insert(struct mc_string *str, size_t index, char const *s)
{
    assert(str);

    size_t len = str->len;
    mc_string_bounds_check(__func__, index, len, true);

    if (!s)
        return;

    size_t const s_len = strlen(s);
    if (s_len == 0)
        return;

    mc_string_reserve(str, s_len);

    char *const data = str->data;
    memmove(data + index + s_len, data + index, len - index + 1);
    memcpy(data + index, s, s_len);
    len += s_len;
    data[len] = '\0';
    str->len = len;
}

void mc_string_remove(struct mc_string *str, char const *s)
{
    assert(str);

    if (!s)
        return;

    size_t len = str->len;
    size_t const s_len = strlen(s);
    if (len == 0 || s_len == 0)
        return;

    char *const data = str->data;
    char *const p = strstr(data, s);
    if (!p)
        return;

    size_t const s_start = p - data;
    size_t const s_end = s_start + s_len;
    memmove(data + s_start, data + s_end, len - s_end + 1);
    len -= s_len;
    data[len] = '\0';
    str->len = len;
}

void mc_string_clear(struct mc_string *str)
{
    assert(str);

    if (str->len > 0) {
        str->len = 0;
        str->data[str->len] = '\0';
    }
}

static void mc_string_reallocate(struct mc_string *str, size_t new_capacity)
{
    if (new_capacity == 0) {
        mc_string_deallocate(str);
        return;
    }

    new_capacity += 1;

    char *const new_data = realloc(str->data, new_capacity);
    if (new_data == NULL) {
        fprintf(stderr, "memory allocation of %zu bytes failed\n",
                new_capacity);
        abort();
    }

    str->data = new_data;
    str->capacity = new_capacity - 1;
}

void mc_string_reserve(struct mc_string *str, size_t additional)
{
    assert(str);

    size_t const len = str->len;
    if (additional > SIZE_MAX - len - 1) {
        fprintf(stderr, "capacity overflow\n");
        abort();
    }

    size_t const request_size = len + additional;
    size_t const capacity = str->capacity;
    if (capacity >= request_size)
        return;

    mc_string_reallocate(str, mc_max2(request_size, capacity * 2));
}

void mc_string_reserve_exact(struct mc_string *str, size_t additional)
{
    assert(str);

    size_t const len = str->len;
    if (additional > SIZE_MAX - len - 1) {
        fprintf(stderr, "capacity overflow\n");
        abort();
    }

    size_t const request_size = len + additional;
    size_t const capacity = str->capacity;
    if (capacity >= request_size)
        return;

    mc_string_reallocate(str, request_size);
}

void mc_string_shrink_to_fit(struct mc_string *str)
{
    assert(str);
    if (str->capacity > str->len)
        mc_string_reallocate(str, str->len);
}

void mc_string_trim(struct mc_string *str)
{
    assert(str);
    mc_string_trim_left(str);
    mc_string_trim_right(str);
}

void mc_string_trim_left(struct mc_string *str)
{
    assert(str);

    size_t len = str->len;
    if (len == 0)
        return;

    size_t i = 0;
    char *const data = str->data;
    while (i < len && isspace(data[i]))
        ++i;
    len -= i;
    memmove(data, data + i, len);
    data[len] = '\0';
    str->len = len;
}

void mc_string_trim_right(struct mc_string *str)
{
    assert(str);

    size_t const len = str->len;
    if (len == 0)
        return;

    size_t i = len - 1;
    char *const data = str->data;
    while (isspace(data[i])) {
        if (i-- == 0)
            break;
    }
    ++i;
    data[i] = '\0';
    str->len = i;
}

void mc_string_replace(struct mc_string *str, char const *from, char const *to)
{
    assert(str);
    assert(from);
    assert(to);

    size_t const from_len = strlen(from);
    if (from_len == 0)
        return;

    size_t const to_len = strlen(to);

    struct mc_string new_str;
    mc_string_init(&new_str);

    char *start = str->data;
    char *index = NULL;
    while ((index = strstr(start, from))) {
        mc_string_append_bytes(&new_str, start, index - start);
        mc_string_append_bytes(&new_str, to, to_len);
        start = index + from_len;
    }
    mc_string_append_bytes(&new_str, start, str->len - (start - str->data));

    mc_string_cleanup(str);
    mc_string_move(str, &new_str);
}

void mc_string_to_upper(struct mc_string const *str)
{
    assert(str);
    for (size_t i = 0, len = str->len; i < len; i++)
        str->data[i] = (char)toupper(str->data[i]);
}

void mc_string_to_lower(struct mc_string const *str)
{
    assert(str);
    for (size_t i = 0, len = str->len; i < len; i++)
        str->data[i] = (char)tolower(str->data[i]);
}

void mc_string_repeat(struct mc_string *str, size_t n)
{
    assert(str);

    size_t const len = str->len;
    if (n > (SIZE_MAX - 1) / len) {
        fprintf(stderr, "capacity overflow\n");
        abort();
    }

    if (n <= 1)
        return;

    mc_string_reserve(str, len * (n - 1));

    char *data = str->data;
    for (size_t i = 1; i < n; ++i)
        memcpy(data + len * i, data, len);

    str->len = n * len;
    str->data[str->len] = '\0';
}

bool mc_string_find(struct mc_string const *str, char const *pattern,
                    size_t *index)
{
    assert(str);
    assert(pattern);

    size_t const pattern_len = strlen(pattern);
    if (pattern_len == 0) {
        if (index)
            *index = 0;
        return true;
    }

    if (str->len < pattern_len)
        return false;

    char const *const position = strstr(str->data, pattern);
    if (!position)
        return false;

    if (index)
        *index = position - str->data;

    return true;
}

bool mc_string_rfind(struct mc_string const *str, char const *pattern,
                     size_t *index)
{
    assert(str);
    assert(pattern);

    size_t const len = str->len;
    size_t const pattern_len = strlen(pattern);
    if (pattern_len == 0) {
        if (index)
            *index = len;
        return true;
    }

    if (len < pattern_len)
        return false;

    char const *curr = str->data + len - pattern_len;
    char const *const end = str->data;
    for (; curr >= end; --curr) {
        if (curr[0] != pattern[0])
            continue;

        if (memcmp(curr, pattern, pattern_len) != 0)
            continue;

        break;
    }

    if (curr < end)
        return false;

    if (index)
        *index = curr - end;

    return true;
}

bool mc_string_find_ch(struct mc_string const *str, char pattern, size_t *index)
{
    assert(str);

    if (str->len == 0)
        return false;

    char const *position = strchr(str->data, pattern);
    if (position == NULL)
        return false;

    if (index)
        *index = position - str->data;

    return true;
}

bool mc_string_rfind_ch(struct mc_string const *str, char pattern,
                        size_t *index)
{
    assert(str);

    if (str->len == 0)
        return false;

    char const *position = strrchr(str->data, pattern);
    if (position == NULL)
        return false;

    if (index)
        *index = position - str->data;

    return true;
}

bool mc_string_contains(struct mc_string const *str, char const *pattern)
{
    assert(str);
    if (str->len == 0)
        return false;
    return strstr(str->data, pattern) != NULL;
}

bool mc_string_contains_ch(struct mc_string const *str, char ch)
{
    assert(str);
    if (str->len == 0)
        return false;
    return strchr(str->data, ch) != NULL;
}

bool mc_string_starts_with(struct mc_string const *str, char const *pattern)
{
    assert(str);
    assert(pattern);

    size_t const pattern_len = strlen(pattern);
    if (pattern_len == 0)
        return true;

    size_t const len = str->len;
    if (len == 0 || pattern_len > len)
        return false;

    return memcmp(str->data, pattern, pattern_len) == 0;
}

bool mc_string_ends_with(struct mc_string const *str, char const *pattern)
{
    assert(str);
    assert(pattern);

    size_t const pattern_len = strlen(pattern);
    if (pattern_len == 0)
        return true;

    size_t const len = str->len;
    if (len == 0 || pattern_len > len)
        return false;

    return memcmp(str->data + len - pattern_len, pattern, pattern_len) == 0;
}

void mc_string_strip_prefix(struct mc_string *str, char const *prefix)
{
    assert(str);
    assert(prefix);

    size_t const prefix_len = strlen(prefix);
    if (prefix_len == 0)
        return;

    size_t len = str->len;
    if (len == 0 || prefix_len > len)
        return;

    char *data = str->data;
    if (memcmp(data, prefix, prefix_len) == 0) {
        memmove(data, data + prefix_len, len - prefix_len);
        len -= prefix_len;
        data[len] = '\0';
        str->len = len;
    }
}

void mc_string_strip_suffix(struct mc_string *str, char const *suffix)
{
    assert(str);
    assert(suffix);

    size_t const suffix_len = strlen(suffix);
    if (suffix_len == 0)
        return;

    size_t len = str->len;
    if (len == 0 || suffix_len > len)
        return;

    char *data = str->data;
    if (memcmp(data + len - suffix_len, suffix, suffix_len) == 0) {
        len -= suffix_len;
        data[len] = '\0';
        str->len = len;
    }
}

void mc_string_split(struct mc_string const *str, char const *delim,
                     struct mc_array *parts)
{
    assert(str);
    assert(delim);
    assert(parts);

    mc_array_init(parts, mc_string_get_mc_type());

    char const *s = str->data;
    size_t n = 0;
    while (*s != '\0') {
        s += strspn(s, delim);
        if (*s == '\0')
            break;
        n = strcspn(s, delim);
        if (n == 0)
            break;
        struct mc_string part;
        mc_string_from_bytes(&part, s, n);
        mc_array_push(parts, &part);
        s += n;
    }
}

void mc_string_split_at(struct mc_string *const str, size_t index,
                        struct mc_string *left, struct mc_string *right)
{
    assert(str);
    assert(left);
    assert(right);

    size_t const len = str->len;
    mc_string_bounds_check(__func__, index, len, false);

    mc_string_from_bytes(left, str->data, index);
    mc_string_from_bytes(right, str->data + index, len - index);
}

void mc_string_lines(struct mc_string const *str, struct mc_array *lines)
{
    assert(str);
    assert(lines);
    mc_string_split(str, "\n", lines);
}

void mc_string_move(struct mc_string *dst, struct mc_string *src)
{
    assert(dst);
    assert(src);
    *dst = *src;
    src->data = NULL;
    src->len = 0;
    src->capacity = 0;
}

void mc_string_copy(struct mc_string *dst, const struct mc_string *src)
{
    assert(dst);
    assert(src);
    mc_string_from_bytes(dst, src->data, src->len);
}

int mc_string_compare(struct mc_string const *str1,
                      struct mc_string const *str2)
{
    assert(str1);
    assert(str2);
    if (str1->len > str2->len)
        return 1;
    if (str1->len < str2->len)
        return -1;
    return memcmp(str1->data, str2->data, str1->len);
}

bool mc_string_equal(struct mc_string const *str1, struct mc_string const *str2)
{
    assert(str1);
    assert(str2);
    return mc_string_compare(str1, str2) == 0;
}

size_t mc_string_hash(struct mc_string const *str)
{
    assert(str);
    return MC_HASH(str->data, str->len);
}

MC_DEFINE_TYPE(mc_string, struct mc_string, (mc_cleanup_func)mc_string_cleanup,
               (mc_move_func)mc_string_move, (mc_copy_func)mc_string_copy,
               (mc_compare_func)mc_string_compare,
               (mc_equal_func)mc_string_equal, (mc_hash_func)mc_string_hash)
