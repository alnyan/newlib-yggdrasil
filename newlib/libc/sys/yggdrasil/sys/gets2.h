#pragma once
#include <sys/types.h>

ssize_t gets_safe(int fd, char *buf, size_t lim);
