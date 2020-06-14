#include "syscalls.h"
#include <errno.h>

extern int main(int argc, char **argv);
extern void __libc_signal_init(void);
extern char **environ;

static size_t __libc_vecp_pages;

void _start(uintptr_t arg) {
    // arg: argp page number << 24 | 12 bit page count << 12 | 12 bit argc
    char **argp = (char **) ((arg >> 12) & ~0xFFF);
    int argc = arg & 0xFFF;
    __libc_vecp_pages = (arg >> 12) & 0xFFF;

    // TODO: better handling for this
    environ = &argp[argc + 1];

    __libc_signal_init();

    int ex = main(argc, argp);
    exit(ex);
}
