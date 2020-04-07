#include "syscalls.h"
#include <errno.h>

extern int main(int argc, char **argv);
extern void __libc_signal_init(void);

static int __libc_argc;

void _start(char **argp) {
    __libc_signal_init();
    __libc_argc = 0;

    if (argp) {
        while (argp[__libc_argc]) {
            ++__libc_argc;
        }
    }

    int ex = main(__libc_argc, argp);
    exit(ex);
}
