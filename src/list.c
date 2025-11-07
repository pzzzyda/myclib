#include <stdio.h>
#include <stdlib.h>
#include "myclib/list.h"
#include "myclib/utils.h"
#include "myclib/aligned_malloc.h"

void mc_list_init(struct mc_list *list, struct mc_type const *elem_type)
{
    assert(list);
    assert(elem_type);
    assert(elem_type->size > 0);
    assert(mc_is_pow_of_two(elem_type->alignment));
    assert(elem_type->move);
    list->elem_type = elem_type;
    list->head = NULL;
    list->tail = NULL;
    list->len = 0;
    list->node_alignment =
        mc_max2(elem_type->alignment, alignof(struct mc_list_node));
    size_t const mask = elem_type->alignment - 1;
    list->elem_offset = (sizeof(struct mc_list_node) + mask) & ~mask;
    list->node_size = list->elem_offset + elem_type->size;
}

void mc_list_destroy(struct mc_list *list)
{
    assert(list);
    mc_list_clear(list);
    list->elem_type = NULL;
    list->node_alignment = 0;
    list->node_size = 0;
    list->elem_offset = 0;
}

size_t mc_list_len(struct mc_list const *list)
{
    assert(list);
    return list->len;
}

bool mc_list_is_empty(struct mc_list const *list)
{
    assert(list);
    return list->len == 0;
}

static void *mc_list_node_elem(struct mc_list const *list,
                               struct mc_list_node *node)
{
    assert(list);
    assert(node);
    return mc_ptr_add(node, list->elem_offset);
}

static struct mc_list_node *mc_list_allocate_node(struct mc_list *list)
{
    struct mc_list_node *node =
        mc_aligned_malloc(list->node_alignment, list->node_size);

    if (!node) {
        fprintf(stderr, "memory allocation of %zu bytes failed\n",
                list->node_size);
        abort();
    }

    return node;
}

static struct mc_list_node *mc_list_new_node(struct mc_list *list, void *elem)
{
    struct mc_list_node *node = mc_list_allocate_node(list);
    node->prev = NULL;
    node->next = NULL;
    list->elem_type->move(mc_list_node_elem(list, node), elem);
    return node;
}

static void mc_list_delete_node(struct mc_list *list, struct mc_list_node *node)
{
    if (list->elem_type->destroy)
        list->elem_type->destroy(mc_list_node_elem(list, node));
    mc_aligned_free(node);
}

static void mc_list_append_node(struct mc_list *list, struct mc_list_node *node)
{
    if (list->tail) {
        list->tail->next = node;
        node->prev = list->tail;
        list->tail = node;
    } else {
        list->head = node;
        list->tail = node;
    }
}

void mc_list_push_back(struct mc_list *list, void *elem)
{
    assert(list);
    assert(elem);

    mc_list_append_node(list, mc_list_new_node(list, elem));
    ++list->len;
}

void mc_list_push_front(struct mc_list *list, void *elem)
{
    assert(list);
    assert(elem);

    struct mc_list_node *node = mc_list_new_node(list, elem);
    if (list->head) {
        list->head->prev = node;
        node->next = list->head;
        list->head = node;
    } else {
        list->head = node;
        list->tail = node;
    }
    ++list->len;
}

bool mc_list_pop_back(struct mc_list *list, void *out_elem)
{
    assert(list);

    if (list->tail) {
        if (out_elem)
            list->elem_type->move(out_elem,
                                  mc_list_node_elem(list, list->tail));
        struct mc_list_node *prev = list->tail->prev;
        mc_list_delete_node(list, list->tail);
        list->tail = prev;
        if (prev)
            prev->next = NULL;
        else
            list->head = NULL;
        --list->len;
        return true;
    }
    return false;
}

bool mc_list_pop_front(struct mc_list *list, void *out_elem)
{
    assert(list);

    if (list->head) {
        if (out_elem)
            list->elem_type->move(out_elem,
                                  mc_list_node_elem(list, list->head));
        struct mc_list_node *next = list->head->next;
        mc_list_delete_node(list, list->head);
        list->head = next;
        if (next)
            next->prev = NULL;
        else
            list->tail = NULL;
        --list->len;
        return true;
    }
    return false;
}

static struct mc_list_node *mc_list_node_at(struct mc_list *list, size_t index)
{
    assert(list);
    assert(index < list->len);

    struct mc_list_node *node = list->head;
    for (size_t i = 0; i < index; ++i)
        node = node->next;

    return node;
}

static void mc_list_bounds_check(char const *func_name, size_t index,
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

void mc_list_insert(struct mc_list *list, size_t index, void *elem)
{
    assert(list);
    assert(index <= list->len);
    assert(elem);

    if (index == list->len) {
        mc_list_push_back(list, elem);
        return;
    }

    mc_list_bounds_check(__func__, index, list->len, false);

    struct mc_list_node *node = mc_list_new_node(list, elem);
    struct mc_list_node *next = mc_list_node_at(list, index);
    node->next = next;
    node->prev = next->prev;
    if (next->prev)
        next->prev->next = node;
    else
        list->head = node;

    next->prev = node;
    ++list->len;
}

void mc_list_remove(struct mc_list *list, size_t index, void *out_elem)
{
    assert(list);
    assert(index < list->len);

    mc_list_bounds_check(__func__, index, list->len, false);

    struct mc_list_node *node = mc_list_node_at(list, index);
    if (out_elem)
        list->elem_type->move(out_elem, mc_list_node_elem(list, node));

    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;

    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;

    mc_list_delete_node(list, node);
    --list->len;
}

void mc_list_clear(struct mc_list *list)
{
    assert(list);

    if (list->len == 0)
        return;

    while (list->head) {
        struct mc_list_node *head = list->head;
        list->head = head->next;
        mc_list_delete_node(list, head);
    }

    list->tail = NULL;
    list->len = 0;
}

void mc_list_for_each(struct mc_list *list,
                      void (*func)(void *elem, void *user_data),
                      void *user_data)
{
    assert(list);
    assert(func);

    struct mc_list_node *node = list->head;
    while (node) {
        func(mc_list_node_elem(list, node), user_data);
        node = node->next;
    }
}

void mc_list_move(struct mc_list *dst, struct mc_list *src)
{
    assert(dst);
    assert(src);

    *dst = *src;
    src->head = NULL;
    src->tail = NULL;
    src->len = 0;
}

void mc_list_copy(struct mc_list *dst, struct mc_list const *src)
{
    assert(dst);
    assert(src);

    mc_copy_func const copy = mc_type_get_copy_forced(__func__, src->elem_type);

    mc_list_init(dst, src->elem_type);

    struct mc_list_node *node = src->head;
    while (node) {
        struct mc_list_node *new_node = mc_list_allocate_node(dst);
        copy(mc_list_node_elem(dst, new_node), mc_list_node_elem(src, node));
        mc_list_append_node(dst, new_node);
        node = node->next;
    }

    dst->len = src->len;
}

int mc_list_compare(struct mc_list const *list1, struct mc_list const *list2)
{
    assert(list1);
    assert(list2);

    mc_compare_func const compare =
        mc_type_get_compare_forced(__func__, list1->elem_type);

    if (list1->len != list2->len)
        return list1->len < list2->len ? -1 : 1;

    struct mc_list_node *node1 = list1->head;
    struct mc_list_node *node2 = list2->head;
    while (node1) {
        int cmp = compare(mc_list_node_elem(list1, node1),
                          mc_list_node_elem(list2, node2));
        if (cmp != 0)
            return cmp;
        node1 = node1->next;
        node2 = node2->next;
    }

    return 0;
}

bool mc_list_equal(struct mc_list const *list1, struct mc_list const *list2)
{
    assert(list1);
    assert(list2);

    if (list1->len != list2->len)
        return false;

    mc_equal_func const equal =
        mc_type_get_equal_forced(__func__, list1->elem_type);

    struct mc_list_node *node1 = list1->head;
    struct mc_list_node *node2 = list2->head;
    while (node1) {
        if (!equal(mc_list_node_elem(list1, node1),
                   mc_list_node_elem(list2, node2)))
            return false;
        node1 = node1->next;
        node2 = node2->next;
    }

    return true;
}

size_t mc_list_hash(struct mc_list const *list)
{
    assert(list);

    mc_hash_func const hash =
        mc_type_get_hash_forced(__func__, list->elem_type);

    size_t h = 17;
    struct mc_list_node *node = list->head;
    while (node) {
        h = h * 31 + hash(mc_list_node_elem(list, node));
        node = node->next;
    }
    return h;
}

MC_DEFINE_TYPE(mc_list, struct mc_list, (mc_destroy_func)mc_list_destroy,
               (mc_move_func)mc_list_move, (mc_copy_func)mc_list_copy,
               (mc_compare_func)mc_list_compare, (mc_equal_func)mc_list_equal,
               (mc_hash_func)mc_list_hash)
