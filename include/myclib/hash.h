#ifndef MYCLIB_HASH_H
#define MYCLIB_HASH_H

#include <stddef.h>
#include <stdint.h>
#include "myclib/macros.h"

uint64_t mc_hash_fnv1a64(const void *data, size_t len);
uint32_t mc_hash_fnv1a32(const void *data, size_t len);

#if defined(MC_ARCH_64)
#define MC_HASH(data, len) mc_hash_fnv1a64(data, len)
#elif defined(MC_ARCH_32)
#define MC_HASH(data, len) mc_hash_fnv1a32(data, len)
#else
#error "Unknown architecture!"
#endif

#endif
