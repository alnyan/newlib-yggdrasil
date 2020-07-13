#pragma once
#include <ygg/select.h>

#if defined(__cplusplus)
extern "C" {
#endif

int select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *xfds, struct timeval *tv);

#if defined(__cplusplus)
}
#endif

