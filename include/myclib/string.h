#ifndef MYCLIB_STRING_H
#define MYCLIB_STRING_H

#include "myclib/type.h"
#include "myclib/array.h"

struct mc_string {
    char *data;
    size_t len;
    size_t capacity;
};

MC_DECLARE_TYPE(mc_string);

void mc_string_init(struct mc_string *str);
void mc_string_from(struct mc_string *str, char const *s);
void mc_string_from_bytes(struct mc_string *str, void const *bytes, size_t len);
void mc_string_format(struct mc_string *str, char const *fmt, ...);
void mc_string_join(struct mc_array const *parts, char const *separator,
                    struct mc_string *str);

void mc_string_destroy(struct mc_string *str);

size_t mc_string_len(struct mc_string const *str);
size_t mc_string_capacity(struct mc_string const *str);
bool mc_string_is_empty(struct mc_string const *str);

char const *mc_string_c_str(struct mc_string *str);

void mc_string_append(struct mc_string *str, char const *s);
void mc_string_append_bytes(struct mc_string *str, void const *bytes,
                            size_t len);
void mc_string_append_format(struct mc_string *str, char const *fmt, ...);
void mc_string_insert(struct mc_string *str, size_t index, char const *s);
void mc_string_remove(struct mc_string *str, char const *s);
void mc_string_clear(struct mc_string *str);

void mc_string_reserve(struct mc_string *str, size_t additional);
void mc_string_reserve_exact(struct mc_string *str, size_t additional);
void mc_string_shrink_to_fit(struct mc_string *str);

void mc_string_replace(struct mc_string *str, char const *from, char const *to);
void mc_string_repeat(struct mc_string *str, size_t n);
void mc_string_to_upper(struct mc_string const *str);
void mc_string_to_lower(struct mc_string const *str);
void mc_string_trim(struct mc_string *str);
void mc_string_trim_left(struct mc_string *str);
void mc_string_trim_right(struct mc_string *str);
void mc_string_strip_prefix(struct mc_string *str, char const *prefix);
void mc_string_strip_suffix(struct mc_string *str, char const *suffix);

bool mc_string_find(struct mc_string const *str, char const *pattern,
                    size_t *index);
bool mc_string_rfind(struct mc_string const *str, char const *pattern,
                     size_t *index);
bool mc_string_find_ch(struct mc_string const *str, char pattern,
                       size_t *index);
bool mc_string_rfind_ch(struct mc_string const *str, char pattern,
                        size_t *index);
bool mc_string_contains(struct mc_string const *str, char const *pattern);
bool mc_string_contains_ch(struct mc_string const *str, char ch);
bool mc_string_starts_with(struct mc_string const *str, char const *pattern);
bool mc_string_ends_with(struct mc_string const *str, char const *pattern);

void mc_string_split(struct mc_string const *str, char const *delim,
                     struct mc_array *parts);
void mc_string_split_at(struct mc_string *const str, size_t index,
                        struct mc_string *left, struct mc_string *right);
void mc_string_lines(struct mc_string const *str, struct mc_array *lines);

void mc_string_move(struct mc_string *dst, struct mc_string *src);
void mc_string_copy(struct mc_string *dst, struct mc_string const *src);
int mc_string_compare(struct mc_string const *str1,
                      struct mc_string const *str2);
bool mc_string_equal(struct mc_string const *str1,
                     struct mc_string const *str2);
size_t mc_string_hash(struct mc_string const *str);

#endif
