#ifndef MYCLIB_ATTRIBUTE_H
#define MYCLIB_ATTRIBUTE_H

#if defined(__GNUC__) || defined(__clang__)
#define MC_COMPILER_SUPPORTS_ATTRIBUTE 1

#define MC_ATTRIBUTE(attr) __attribute__((attr))
#define MC_ATTR_CONSTRUCTOR __attribute__((constructor))
#define MC_ATTR_DESTRUCTOR __attribute__((destructor))
#define MC_ATTR_PRINTF(fmt_idx, args_idx)                                      \
    __attribute__((format(printf, fmt_idx, args_idx)))

#else
#define MC_COMPILER_SUPPORTS_ATTRIBUTE 0

#define MC_ATTRIBUTE(attr)
#define MC_ATTR_CONSTRUCTOR
#define MC_ATTR_DESTRUCTOR
#define MC_ATTR_PRINTF(fmt_idx, args_idx)

#endif

#endif
