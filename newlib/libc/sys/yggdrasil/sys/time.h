#pragma once
#include <sys/_timeval.h>
#include <sys/_timespec.h>

struct timeval;
struct timezone;
struct timespec;

int gettimeofday(struct timeval *tv, void *tz);
