#pragma once

typedef int (*clone_entry_t) (void *);

int clone(clone_entry_t fn, void *stack, int flags, void *arg);
