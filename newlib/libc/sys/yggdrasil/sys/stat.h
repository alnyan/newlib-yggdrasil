#pragma once
#include <sys/types.h>
#define _STAT_H_
#include <ygg/stat.h>

int stat(const char *name, struct stat *st);
int fstat(int fd, struct stat *st);
int mkdir(const char *path, mode_t mode);
