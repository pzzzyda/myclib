#ifndef MYCLIB_MAP_H
#define MYCLIB_MAP_H

#include "myclib/type.h"

struct mc_hash_entry;

struct mc_hash_table {
    struct mc_hash_entry *entries;
    struct mc_type const *key_type;
    struct mc_type const *value_type;
    size_t key_offset;
    size_t value_offset;
    size_t entry_alignment;
    size_t entry_size;
    size_t capacity;
};

struct mc_map {
    struct mc_hash_table table;
    size_t len;
};

MC_DECLARE_TYPE(mc_map);

void mc_map_init(struct mc_map *map, struct mc_type const *key_type,
                 struct mc_type const *value_type);

void mc_map_cleanup(struct mc_map *map);

size_t mc_map_len(struct mc_map const *map);

size_t mc_map_capacity(struct mc_map const *map);

bool mc_map_is_empty(struct mc_map const *map);

void mc_map_insert(struct mc_map *map, void *key, void *value);

bool mc_map_remove(struct mc_map *map, void const *key, void *out_key,
                   void *out_value);

void mc_map_clear(struct mc_map *map);

void mc_map_reserve(struct mc_map *map, size_t additional);

void mc_map_shrink_to_fit(struct mc_map *map);

void *mc_map_get(struct mc_map const *map, void const *key);

bool mc_map_contains_key(struct mc_map const *map, void const *key);

void mc_map_move(struct mc_map *dst, struct mc_map *src);

void mc_map_copy(struct mc_map *dst, struct mc_map const *src);

#endif
