#pragma once
#include <ygg/mman.h>
#include <sys/types.h>

#if defined(__cplusplus)
extern "C" {
#endif

void *mmap(void *hint, size_t len, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t len);

#if defined(__cplusplus)
}
#endif
