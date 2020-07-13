#pragma once
#include <ygg/wait.h>

#if defined(__cplusplus)
extern "C" {
#endif

int waitpid(int pid, int *wstatus, int options);

#if defined(__cplusplus)
}
#endif
