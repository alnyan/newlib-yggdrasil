#pragma once

typedef int (*clone_entry_t) (void *);

#if defined(__cplusplus)
extern "C" {
#endif

int clone(clone_entry_t fn, void *stack, int flags, void *arg);

#if defined(__cplusplus)
}
#endif
