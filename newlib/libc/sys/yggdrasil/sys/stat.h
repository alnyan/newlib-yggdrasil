#pragma once
#include <sys/types.h>
#define _STAT_H_
#include <ygg/stat.h>

#if defined(__cplusplus)
extern "C" {
#endif

int mknod(const char *filename, int mode, unsigned int dev);
int stat(const char *name, struct stat *st);
int lstat(const char *name, struct stat *st);
int fstat(int fd, struct stat *st);
int mkdir(const char *path, mode_t mode);

#if defined(__cplusplus)
}
#endif
