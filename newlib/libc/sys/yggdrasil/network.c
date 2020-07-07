#include <sys/socket.h>
#include <ygg/syscall.h>

#include "syscalls.h"

extern int errno;

int _noopt socket(int domain, int type, int proto) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_SOCKET, domain, type, proto));
}

int _noopt bind(int fd, const struct sockaddr *sa, socklen_t salen) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_BIND, fd, sa, salen));
}

int _noopt connect(int fd, const struct sockaddr *sa, socklen_t salen) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_CONNECT, fd, sa, salen));
}

int _noopt accept(int fd, const struct sockaddr *sa, socklen_t *salen) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_ACCEPT, fd, sa, salen));
}

ssize_t _noopt recvfrom(int fd, void *buf, size_t len, int _, struct sockaddr *sa, socklen_t *salen) {
    return SET_ERRNO(ssize_t, ASM_SYSCALL5(SYSCALL_NR_RECVFROM, fd, buf, len, sa, salen));
}

ssize_t _noopt sendto(int fd, const void *buf, size_t len, int _, const struct sockaddr *sa, socklen_t *salen) {
    return SET_ERRNO(ssize_t, ASM_SYSCALL5(SYSCALL_NR_SENDTO, fd, buf, len, sa, salen));
}

ssize_t recv(int fd, void *buf, size_t len, int _) {
    return recvfrom(fd, buf, len, _, NULL, NULL);
}

ssize_t send(int fd, const void *buf, size_t len, int _) {
    return sendto(fd, buf, len, _, NULL, 0);
}
