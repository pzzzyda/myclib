#ifndef MYCLIB_ALIGNED_MALLOC_H
#define MYCLIB_ALIGNED_MALLOC_H

#include <stddef.h>

void *mc_aligned_malloc(size_t alignment, size_t size);
void mc_aligned_free(void *ptr);

#endif
