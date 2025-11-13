#ifndef MYCLIB_ITER_H
#define MYCLIB_ITER_H

#include <stdbool.h>

struct mc_iter {
    void const *container;
    void *current;
    void *value;
    void const *key;
    bool (*next)(struct mc_iter *iter);
};

#endif
