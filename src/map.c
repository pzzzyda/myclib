#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "myclib/map.h"
#include "myclib/aligned_malloc.h"
#include "myclib/utils.h"

struct mc_hash_entry {
    void *storage;
    size_t hash_value;
};

#define MC_HASH_ENTRY_ALIGNMENT (alignof(struct mc_hash_entry))
#define MC_HASH_ENTRY_SIZE (sizeof(struct mc_hash_entry))

static inline bool mc_hash_entry_is_empty(struct mc_hash_entry const *entry)
{
    return entry->hash_value == 0 && entry->storage == NULL;
}

static inline bool mc_hash_entry_is_tombstone(struct mc_hash_entry const *entry)
{
    return entry->hash_value != 0 && entry->storage == NULL;
}

static inline bool mc_hash_entry_is_valid(struct mc_hash_entry const *entry)
{
    return entry->hash_value != 0 && entry->storage != NULL;
}

static inline void *mc_hash_table_entry_key(struct mc_hash_table const *table,
                                            struct mc_hash_entry const *entry)
{
    return mc_ptr_add(entry->storage, table->key_offset);
}

static inline void *mc_hash_table_entry_value(struct mc_hash_table const *table,
                                              struct mc_hash_entry const *entry)
{
    return mc_ptr_add(entry->storage, table->value_offset);
}

static void *
mc_hash_table_allocate_entry_storage(struct mc_hash_table const *table)
{
    void *ptr = mc_aligned_malloc(table->entry_alignment, table->entry_size);
    if (!ptr) {
        fprintf(stderr, "memory allocation of %zu bytes failed\n",
                table->entry_size);
        abort();
    }
    return ptr;
}

static void mc_hash_table_init_entry(struct mc_hash_table const *table,
                                     struct mc_hash_entry *entry, void *key,
                                     void *value, size_t hash_value)
{
    entry->storage = mc_hash_table_allocate_entry_storage(table);
    table->key_type->move(mc_hash_table_entry_key(table, entry), key);
    table->value_type->move(mc_hash_table_entry_value(table, entry), value);
    entry->hash_value = hash_value;
}

static void mc_hash_table_copy_entry(struct mc_hash_table const *table,
                                     struct mc_hash_entry *dst,
                                     struct mc_hash_entry const *src)
{
    dst->storage = mc_hash_table_allocate_entry_storage(table);
    table->key_type->copy(mc_hash_table_entry_key(table, dst),
                          mc_hash_table_entry_key(table, src));
    table->value_type->copy(mc_hash_table_entry_value(table, dst),
                            mc_hash_table_entry_value(table, src));
    dst->hash_value = src->hash_value;
}

static void
mc_hash_table_cleanup_entry_storage(struct mc_hash_table const *table,
                                    struct mc_hash_entry *entry)
{
    mc_cleanup_func cleanup_key = table->key_type->cleanup;
    mc_cleanup_func cleanup_value = table->value_type->cleanup;

    if (cleanup_key)
        cleanup_key(mc_hash_table_entry_key(table, entry));

    if (cleanup_value)
        cleanup_value(mc_hash_table_entry_value(table, entry));
}

static void mc_hash_table_cleanup_entry(struct mc_hash_table const *table,
                                        struct mc_hash_entry *entry)
{
    mc_hash_table_cleanup_entry_storage(table, entry);
    mc_aligned_free(entry->storage);
    entry->storage = NULL;
}

static void
mc_hash_table_replace_entry_content(struct mc_hash_table const *table,
                                    struct mc_hash_entry *entry, void *key,
                                    void *value)
{
    mc_hash_table_cleanup_entry_storage(table, entry);
    table->key_type->move(mc_hash_table_entry_key(table, entry), key);
    table->value_type->move(mc_hash_table_entry_value(table, entry), value);
}

static void mc_hash_table_init(struct mc_hash_table *table,
                               struct mc_type const *key_type,
                               struct mc_type const *value_type,
                               size_t capacity)
{
    size_t total_size;

    table->key_type = key_type;
    table->value_type = value_type;
    if (key_type->alignment > value_type->alignment) {
        table->entry_alignment = key_type->alignment;
        table->key_offset = 0;
        size_t mask = value_type->alignment - 1;
        table->value_offset = (key_type->size + mask) & ~mask;
        table->entry_size = table->value_offset + value_type->size;
    } else {
        table->entry_alignment = value_type->alignment;
        table->value_offset = 0;
        size_t mask = key_type->alignment - 1;
        table->key_offset = (value_type->size + mask) & ~mask;
        table->entry_size = table->key_offset + key_type->size;
    }

    if (capacity == 0) {
        table->entries = NULL;
        table->capacity = 0;
        return;
    }

    total_size = MC_HASH_ENTRY_SIZE * capacity;
    table->entries = mc_aligned_malloc(MC_HASH_ENTRY_ALIGNMENT, total_size);
    if (!table->entries) {
        fprintf(stderr, "memory allocation of %zu bytes failed\n", total_size);
        abort();
    }
    memset(table->entries, 0, total_size);
    table->capacity = capacity;
}

static void mc_hash_table_free_entries(struct mc_hash_table *table)
{
    if (table->capacity > 0) {
        mc_aligned_free(table->entries);
        table->entries = NULL;
        table->capacity = 0;
    }
}

static size_t mc_hash_table_calculate_probe_distance(size_t expected_idx,
                                                     size_t current_idx,
                                                     size_t capacity)
{
    return current_idx >= expected_idx ? current_idx - expected_idx
                                       : current_idx + capacity - expected_idx;
}

static void mc_hash_table_insert_with_probing(struct mc_hash_table *table,
                                              struct mc_hash_entry *entry)
{
    size_t capacity = table->capacity;
    size_t mask = capacity - 1;
    size_t start = entry->hash_value & mask;
    size_t probe_index = start;
    size_t distance = 0;
    struct mc_hash_entry *entries = table->entries;

    while (true) {
        struct mc_hash_entry *curr_entry = &entries[probe_index];

        if (!mc_hash_entry_is_valid(curr_entry)) {
            *curr_entry = *entry;
            return;
        }

        size_t curr_distance = mc_hash_table_calculate_probe_distance(
            curr_entry->hash_value & mask, probe_index, capacity);

        if (distance > curr_distance) {
            struct mc_hash_entry tmp = *curr_entry;
            *curr_entry = *entry;
            *entry = tmp;
            distance = curr_distance;
        }

        probe_index = (probe_index + 1) & mask;
        if (probe_index == start)
            break;
        ++distance;
    }
}

static void mc_hash_table_remove_all_entries(struct mc_hash_table *table)
{
    struct mc_hash_entry *entries = table->entries;
    for (size_t i = 0, capacity = table->capacity; i < capacity; ++i) {
        if (mc_hash_entry_is_valid(&entries[i]))
            mc_hash_table_cleanup_entry(table, &entries[i]);
        entries[i].hash_value = 0;
    }
}

static void mc_hash_table_cleanup(struct mc_hash_table *table)
{
    mc_hash_table_remove_all_entries(table);
    mc_hash_table_free_entries(table);
    table->key_type = NULL;
    table->value_type = NULL;
}

static void *mc_hash_table_lookup_entry(struct mc_hash_table const *table,
                                        void const *key, size_t hash_value)
{
    size_t mask = table->capacity - 1;
    size_t start = hash_value & mask;
    size_t index = start;
    struct mc_hash_entry *entries = table->entries;
    mc_equal_func equal = table->key_type->equal;

    while (true) {
        struct mc_hash_entry *entry = &entries[index];

        if (mc_hash_entry_is_empty(entry))
            break;

        if (mc_hash_entry_is_tombstone(entry))
            goto next;

        if (entry->hash_value != hash_value)
            goto next;

        void *entry_key = mc_hash_table_entry_key(table, entry);
        if (equal(entry_key, key))
            return entry;

    next:
        index = (index + 1) & mask;
        if (index == start)
            break;
    }

    return NULL;
}

static void mc_hash_table_rehash_entries(struct mc_hash_table *table,
                                         struct mc_hash_table *old_table)
{
    struct mc_hash_entry *old_entries = old_table->entries;

    for (size_t i = 0, capacity = old_table->capacity; i < capacity; ++i) {
        if (!mc_hash_entry_is_valid(&old_entries[i]))
            continue;

        mc_hash_table_insert_with_probing(table, &old_entries[i]);
    }
}

static void mc_hash_table_remove_entry(struct mc_hash_table *table,
                                       struct mc_hash_entry *entry,
                                       void *out_key, void *out_value)
{
    if (out_key)
        table->key_type->move(out_key, mc_hash_table_entry_key(table, entry));

    if (out_value)
        table->value_type->move(out_value,
                                mc_hash_table_entry_value(table, entry));

    mc_hash_table_cleanup_entry(table, entry);
}

static void *mc_hash_table_lookup_value(struct mc_hash_table const *table,
                                        void const *key, size_t hash_value)
{
    struct mc_hash_entry *entry;

    entry = mc_hash_table_lookup_entry(table, key, hash_value);
    if (entry)
        return mc_hash_table_entry_value(table, entry);

    return NULL;
}

static void mc_hash_table_for_each(struct mc_hash_table const *table,
                                   void (*func)(void const *key, void *value,
                                                void *user_data),
                                   void *user_data)
{
    struct mc_hash_entry *entries = table->entries;

    for (size_t i = 0, capacity = table->capacity; i < capacity; ++i) {
        if (!mc_hash_entry_is_valid(&entries[i]))
            continue;

        func(mc_hash_table_entry_key(table, &entries[i]),
             mc_hash_table_entry_value(table, &entries[i]), user_data);
    }
}

void mc_map_init(struct mc_map *map, struct mc_type const *key_type,
                 struct mc_type const *value_type)
{
    assert(map);
    assert(key_type);
    assert(key_type->size > 0);
    assert(mc_is_pow_of_two(key_type->alignment));
    assert(key_type->hash);
    assert(key_type->equal);
    assert(value_type);
    assert(value_type->size > 0);
    assert(mc_is_pow_of_two(value_type->alignment));
    mc_hash_table_init(&map->table, key_type, value_type, 0);
    map->len = 0;
}

void mc_map_cleanup(struct mc_map *map)
{
    assert(map);

    mc_hash_table_cleanup(&map->table);

    map->len = 0;
}

size_t mc_map_len(struct mc_map const *map)
{
    assert(map);
    return map->len;
}

size_t mc_map_capacity(struct mc_map const *map)
{
    assert(map);
    return map->table.capacity;
}

bool mc_map_is_empty(struct mc_map const *map)
{
    assert(map);
    return map->len == 0;
}

static size_t mc_map_scramble_hash(size_t hash_value)
{
    hash_value ^= (hash_value >> 20) ^ (hash_value >> 12);
    return hash_value ^ (hash_value >> 7) ^ (hash_value >> 4);
}

void mc_map_insert(struct mc_map *map, void *key, void *value)
{
    struct mc_hash_entry *entry;
    struct mc_hash_entry new_entry;
    size_t hash_value;

    assert(map);
    assert(key);
    assert(value);

    if (map->len >= map->table.capacity)
        mc_map_reserve(map, map->len ? map->len : 8);

    hash_value = map->table.key_type->hash(key);
    hash_value = mc_map_scramble_hash(hash_value);
    entry = mc_hash_table_lookup_entry(&map->table, key, hash_value);
    if (entry) {
        mc_hash_table_replace_entry_content(&map->table, entry, key, value);
        return;
    }

    mc_hash_table_init_entry(&map->table, &new_entry, key, value, hash_value);
    mc_hash_table_insert_with_probing(&map->table, &new_entry);
    ++map->len;
}

bool mc_map_remove(struct mc_map *map, void const *key, void *out_key,
                   void *out_value)
{
    struct mc_hash_entry *entry;
    size_t hash_value;

    assert(map);
    assert(key);

    hash_value = map->table.key_type->hash(key);
    hash_value = mc_map_scramble_hash(hash_value);
    entry = mc_hash_table_lookup_entry(&map->table, key, hash_value);
    if (!entry)
        return false;

    mc_hash_table_remove_entry(&map->table, entry, out_key, out_value);
    --map->len;
    return true;
}

void mc_map_clear(struct mc_map *map)
{
    assert(map);

    if (map->len > 0) {
        mc_hash_table_remove_all_entries(&map->table);
        map->len = 0;
    }
}

static void mc_map_resize_table(struct mc_map *map, size_t capacity)
{
    struct mc_hash_table new_table;

    if (capacity == 0) {
        mc_hash_table_free_entries(&map->table);
        return;
    }

    mc_hash_table_init(&new_table, map->table.key_type, map->table.value_type,
                       capacity);

    if (map->len > 0)
        mc_hash_table_rehash_entries(&new_table, &map->table);

    mc_hash_table_free_entries(&map->table);
    map->table = new_table;
}

void mc_map_reserve(struct mc_map *map, size_t additional)
{
    size_t new_capacity;

    assert(map);

    if (additional > SIZE_MAX / MC_HASH_ENTRY_SIZE - map->len)
        goto capacity_overflow;

    new_capacity = mc_next_pow_of_two(map->len + additional);
    if (new_capacity == SIZE_MAX)
        goto capacity_overflow;

    if (new_capacity > map->table.capacity)
        mc_map_resize_table(map, new_capacity);

    return;

capacity_overflow:
    fprintf(stderr, "%s: capacity overflow\n", __func__);
    abort();
}

void mc_map_shrink_to_fit(struct mc_map *map)
{
    size_t new_capacity;

    assert(map);

    if (map->len == 0) {
        mc_hash_table_free_entries(&map->table);
        return;
    }

    new_capacity = mc_next_pow_of_two(map->len);
    if (new_capacity < map->table.capacity)
        mc_map_resize_table(map, new_capacity);
}

void *mc_map_get(struct mc_map const *map, void const *key)
{
    size_t hash_value;

    assert(map);
    assert(key);

    if (map->len == 0)
        return NULL;

    hash_value = map->table.key_type->hash(key);
    hash_value = mc_map_scramble_hash(hash_value);
    return mc_hash_table_lookup_value(&map->table, key, hash_value);
}

bool mc_map_contains_key(struct mc_map const *map, void const *key)
{
    assert(map);
    assert(key);

    return mc_map_get(map, key) != NULL;
}

void mc_map_for_each(struct mc_map const *map,
                     void (*func)(void const *key, void *value,
                                  void *user_data),
                     void *user_data)
{
    assert(map);
    assert(func);
    mc_hash_table_for_each(&map->table, func, user_data);
}

void mc_map_move(struct mc_map *dst, struct mc_map *src)
{
    assert(dst);
    assert(src);

    *dst = *src;

    src->table.entries = NULL;
    src->table.capacity = 0;
    src->len = 0;
}

void mc_map_copy(struct mc_map *dst, struct mc_map const *src)
{
    struct mc_hash_entry *src_entries;
    struct mc_hash_table *dst_table;

    assert(dst);
    assert(src);

    mc_type_get_copy_forced(__func__, src->table.key_type);
    mc_type_get_copy_forced(__func__, src->table.value_type);

    mc_map_init(dst, src->table.key_type, src->table.value_type);

    if (src->len == 0)
        return;

    mc_map_reserve(dst, src->len);

    src_entries = src->table.entries;
    dst_table = &dst->table;
    for (size_t i = 0, capacity = src->table.capacity; i < capacity; ++i) {
        if (!mc_hash_entry_is_valid(src_entries + i))
            continue;

        struct mc_hash_entry entry;
        mc_hash_table_copy_entry(dst_table, &entry, src_entries + i);
        mc_hash_table_insert_with_probing(dst_table, &entry);
    }

    dst->len = src->len;
}

void mc_map_iter_init(struct mc_iter *iter, struct mc_map const *map)
{
    assert(iter);
    assert(map);
    iter->container = map;
    iter->current = mc_map_is_empty(map) ? NULL : map->table.entries;
    iter->key = NULL;
    iter->value = NULL;
    iter->next = mc_map_iter_next;
}

bool mc_map_iter_next(struct mc_iter *iter)
{
    assert(iter);
    struct mc_hash_entry *curr = iter->current;
    if (!curr)
        return false;
    struct mc_map const *map = iter->container;
    struct mc_hash_entry const *const end =
        &map->table.entries[map->table.capacity];
    while (curr < end && !mc_hash_entry_is_valid(curr))
        ++curr;
    if (curr >= end)
        return false;
    iter->current = curr + 1;
    iter->key = mc_hash_table_entry_key(&map->table, curr);
    iter->value = mc_hash_table_entry_value(&map->table, curr);
    return true;
}

MC_DEFINE_TYPE(mc_map, struct mc_map, (mc_cleanup_func)mc_map_cleanup,
               (mc_move_func)mc_map_move, (mc_copy_func)mc_map_copy, NULL, NULL,
               NULL)
