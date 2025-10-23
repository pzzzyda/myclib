#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "myclib/aligned_malloc.h"

#define ISPOWOF2(X) (((X) & ((X) - 1)) == 0)
#define PTRSZ (sizeof(void *))

void *mc_aligned_malloc(size_t alignment, size_t size)
{
	if (!alignment || !size)
		return NULL;

	if (!ISPOWOF2(alignment))
		return NULL;

	if (alignment > alignof(max_align_t))
		return NULL;

	size_t extras = PTRSZ + (alignment - 1);
	void *raw_ptr = malloc(size + extras);
	if (!raw_ptr)
		return NULL;

	uintptr_t raw_addr = (uintptr_t)raw_ptr;
	uintptr_t mask = alignment - 1;
	uintptr_t aligned_addr = (raw_addr + PTRSZ + mask) & ~mask;

	void **raw_ptr_storage = (void **)(aligned_addr - PTRSZ);
	*raw_ptr_storage = raw_ptr;

	return (void *)aligned_addr;
}

void mc_aligned_free(void *ptr)
{
	if (!ptr)
		return;

	void **raw_ptr_storage = (void *)((uintptr_t)ptr - PTRSZ);
	free(*raw_ptr_storage);
}
