#include <stddef.h>
#include <stdint.h>
#include "myclib/hash.h"

uint64_t mc_hash_fnv1a64(const void *data, size_t len)
{
	const uint64_t offset = 0xcbf29ce484222325ULL;
	uint64_t hash = offset;
	const uint8_t *bytes = data;
	for (size_t i = 0; i < len; i++) {
		hash ^= bytes[i];
		hash *= 0x100000001b3ULL;
	}
	return hash;
}

uint32_t mc_hash_fnv1a32(const void *data, size_t len)
{
	const uint32_t offset = 0x811c9dc5;
	uint32_t hash = offset;
	const uint8_t *bytes = data;
	for (size_t i = 0; i < len; i++) {
		hash ^= bytes[i];
		hash *= 0x01000193;
	}
	return hash;
}
