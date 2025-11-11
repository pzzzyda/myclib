#include "myclib/time.h"

#if defined(_WIN64) || defined(_WIN32)
#include <windows.h>
#else
#include <time.h>
#endif

double mc_get_current_time(void)
{
#if defined(_WIN64) || defined(_WIN32)
    static LARGE_INTEGER freq = {0};
    LARGE_INTEGER counter;

    if (freq.QuadPart == 0) {
        QueryPerformanceFrequency(&freq);
    }

    QueryPerformanceCounter(&counter);
    return (double)counter.QuadPart / (double)freq.QuadPart;
#else
    struct timespec ts;
#ifdef __APPLE__
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
#endif
    return (double)ts.tv_sec + (double)ts.tv_nsec / 1e9;
#endif
}

double mc_get_current_time_ms(void)
{
    return mc_get_current_time() * 1e3;
}

double mc_get_current_time_us(void)
{
    return mc_get_current_time() * 1e6;
}

double mc_get_current_time_ns(void)
{
    return mc_get_current_time() * 1e9;
}
