#ifndef MYCLIB_LIST_H
#define MYCLIB_LIST_H

#include "myclib/type.h"
#include "myclib/iter.h"

struct mc_list_node {
    struct mc_list_node *prev;
    struct mc_list_node *next;
};

struct mc_list {
    struct mc_type const *elem_type;
    struct mc_list_node *head;
    struct mc_list_node *tail;
    size_t len;
    size_t node_alignment;
    size_t node_size;
    size_t elem_offset;
};

MC_DECLARE_TYPE(mc_list);

void mc_list_init(struct mc_list *list, struct mc_type const *elem_type);

void mc_list_cleanup(struct mc_list *list);

size_t mc_list_len(struct mc_list const *list);
bool mc_list_is_empty(struct mc_list const *list);

void mc_list_push_back(struct mc_list *list, void *elem);
void mc_list_push_front(struct mc_list *list, void *elem);
bool mc_list_pop_back(struct mc_list *list, void *out_elem);
bool mc_list_pop_front(struct mc_list *list, void *out_elem);
void mc_list_insert(struct mc_list *list, size_t index, void *elem);
void mc_list_remove(struct mc_list *list, size_t index, void *out_elem);
void mc_list_clear(struct mc_list *list);

void mc_list_for_each(struct mc_list *list,
                      void (*func)(void *elem, void *user_data),
                      void *user_data);

void mc_list_move(struct mc_list *dst, struct mc_list *src);
void mc_list_copy(struct mc_list *dst, struct mc_list const *src);
int mc_list_compare(struct mc_list const *list1, struct mc_list const *list2);
bool mc_list_equal(struct mc_list const *list1, struct mc_list const *list2);
size_t mc_list_hash(struct mc_list const *list);

void mc_list_iter_init(struct mc_iter *iter, struct mc_list const *list);
bool mc_list_iter_next(struct mc_iter *iter);

#endif
