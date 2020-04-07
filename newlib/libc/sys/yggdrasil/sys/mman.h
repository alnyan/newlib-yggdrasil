#pragma once
#include <ygg/mman.h>
#include <sys/types.h>

void *mmap(void *hint, size_t len, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t len);
