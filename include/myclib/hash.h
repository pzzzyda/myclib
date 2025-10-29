#ifndef MYCLIB_HASH_H
#define MYCLIB_HASH_H

#include <stddef.h>
#include <stdint.h>

uint64_t mc_hash_fnv1a64(const void *data, size_t len);
uint32_t mc_hash_fnv1a32(const void *data, size_t len);

#if SIZE_MAX == UINT64_MAX
#define MC_HASH(data, len) mc_hash_fnv1a64(data, len)
#else
#define MC_HASH(data, len) mc_hash_fnv1a32(data, len)
#endif

#endif
