#include "sys/utsname.h"
#include <ygg/syscall.h>
#include <errno.h>
#include "syscalls.h"

int _noopt uname(struct utsname *u) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NR_UNAME, u));
}
