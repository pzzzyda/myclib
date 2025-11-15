#ifndef MYCLIB_ATTRIBUTE_H
#define MYCLIB_ATTRIBUTE_H

#if defined(__GNUC__) || defined(__clang__)
#define MC_COMPILER_SUPPORTS_ATTRIBUTE 1

#define MC_ATTRIBUTE(attr) __attribute__((attr))

#else
#define MC_COMPILER_SUPPORTS_ATTRIBUTE 0

#define MC_ATTRIBUTE(attr)

#endif

#endif
