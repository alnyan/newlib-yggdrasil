#pragma once
#include <ygg/signum.h>

#define __GNU_VISIBLE 1

// TODO
#define SIG_BLOCK       0
#define SIG_SETMASK     0

typedef void (*_sig_func_ptr)(int);
int kill(int pid, int signum);
