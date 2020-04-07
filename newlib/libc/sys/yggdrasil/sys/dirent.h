#pragma once
#include <ygg/dirent.h>

typedef struct DIR_private DIR;
struct dirent;

DIR *opendir(const char *path);
int closedir(DIR *dirp);
struct dirent *readdir(DIR *dirp);

//DIR *opendir(const char *);
//struct dirent *readdir(DIR *);
//int readdir_r(DIR *__restrict, struct dirent *__restrict,
//              struct dirent **__restrict);
//void rewinddir(DIR *);
//int closedir(DIR *);
//
///* internal prototype */
//void _seekdir(DIR *dir, long offset);
//DIR *_opendir(const char *);
