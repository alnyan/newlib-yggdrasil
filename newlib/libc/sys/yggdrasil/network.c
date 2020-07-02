#include <sys/socket.h>
#include <ygg/syscall.h>

#include "syscalls.h"

extern int errno;

int _noopt socket(int domain, int type, int proto) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_SOCKET, domain, type, proto));
}

int _noopt bind(int fd, const struct sockaddr *sa, const socklen_t salen) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_BIND, fd, sa, salen));
}

ssize_t _noopt recvfrom(int fd, void *buf, size_t len, int _, struct sockaddr *sa, socklen_t *salen) {
    return SET_ERRNO(ssize_t, ASM_SYSCALL5(SYSCALL_NR_RECVFROM, fd, buf, len, sa, salen));
}

ssize_t _noopt sendto(int fd, const void *buf, size_t len, int _, const struct sockaddr *sa, socklen_t *salen) {
    return SET_ERRNO(ssize_t, ASM_SYSCALL5(SYSCALL_NR_SENDTO, fd, buf, len, sa, salen));
}
