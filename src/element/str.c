#include <string.h>
#include "myclib/element/core.h"
#include "myclib/hash.h"

#define MC_NAME static_str
#define MC_TYPE const char *
#define MC_MOVE_IMPL(self, source) *self = *source
#define MC_COPY_IMPL(self, source) *self = *source
#define MC_COMPARE_IMPL(self, other) return strcmp(*self, *other)
#define MC_EQUAL_IMPL(self, other) return strcmp(*self, *other) == 0
#define MC_HASH_IMPL(self) return MC_HASH(*self, strlen(*self))
#include "myclib/element/template.h"
