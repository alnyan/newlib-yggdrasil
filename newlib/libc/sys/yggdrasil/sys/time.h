#pragma once
#include <ygg/time.h>
#include <sys/_timeval.h>
#include <sys/_timespec.h>

struct timeval;
struct timezone;
struct timespec;

#if defined(__cplusplus)
extern "C" {
#endif

int nanosleep(const struct timespec *ts, struct timespec *res);
int gettimeofday(struct timeval *tv, void *tz);

#if defined(__cplusplus)
}
#endif
