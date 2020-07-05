/* note these headers are all provided by newlib - you don't need to provide them */
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <sys/times.h>
#include <sys/errno.h>
#include <sys/time.h>
#include <stdarg.h>
#include <stdio.h>

#include <ygg/termios.h>
#include <ygg/syscall.h>

#include "syscalls.h"

////

void _noopt ygg_debug_trace(const char *msg, ...) {
    uintptr_t argv[5];
    va_list args;
    va_start(args, msg);
    for (size_t i = 0; i < 5; ++i) {
        argv[i] = va_arg(args, uintptr_t);
    }
    va_end(args);
    (void) ASM_SYSCALL6(SYSCALL_NRX_TRACE, msg, argv[0], argv[1], argv[2], argv[3], argv[4]);
}
void *_noopt mmap(void *addr, size_t len, int prot, int flags, int fd, off_t off) {
    intptr_t res = (intptr_t) ASM_SYSCALL6(SYSCALL_NR_MMAP, addr, len, prot, flags, fd, off);
    if (res < 0) {
        errno = (int) -res;
        return (void *) -1;
    } else {
        return (void *) res;
    }
}

int _noopt munmap(void *addr, size_t len) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_MUNMAP, addr, len));
}

////

int _noopt select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *xfds, struct timeval *tv) {
    return SET_ERRNO(int, ASM_SYSCALL5(SYSCALL_NR_SELECT, nfds, rfds, wfds, xfds, tv));
}
int _noopt open(const char *name, int flags, ...) {
    va_list args;
    int mode;
    va_start(args, flags);
    if (flags & O_CREAT) {
        mode = va_arg(args, int);
    } else {
        mode = 0;
    }
    va_end(args);
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_OPEN, name, flags, mode));
}
int _noopt read(int file, char *ptr, int len) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_READ, file, ptr, len));
}
int _noopt write(int file, char *ptr, int len) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_WRITE, file, ptr, len));
}
int _noopt close(int file) {
    (void) ASM_SYSCALL1(SYSCALL_NR_CLOSE, file);
    return 0;
}
int _noopt lseek(int file, int ptr, int dir) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_LSEEK, file, ptr, dir));
}
int _noopt stat(const char *file, struct stat *st) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_STAT, file, st));
}
int _noopt lstat(const char *file, struct stat *st) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_LSTAT, file, st));
}
int _noopt fstat(int fd, struct stat *st) {
    return -1;
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_FSTAT, fd, st));
}
int _noopt unlink(char *name) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NR_UNLINK, name));
}
int _noopt rmdir(const char *name) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NR_RMDIR, name));
}
int _noopt mkdir(const char *name, mode_t mode) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_MKDIR, name, mode));
}
int _noopt ioctl(int fd, unsigned int cmd, void *arg) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_IOCTL, fd, cmd, arg));
}
int _noopt access(const char *path, int mode) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_ACCESS, path, mode));
}
int _noopt pipe(int *filedes) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NR_PIPE, filedes));
}
int _noopt dup(int from) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NR_DUP, from));
}
int _noopt dup2(int from, int to) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_DUP2, from, to));
}
int _noopt sync(void) {
    return SET_ERRNO(int, ASM_SYSCALL0(SYSCALL_NR_SYNC));
}
int _noopt chown(const char *path, uid_t uid, gid_t gid) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_CHOWN, path, uid, gid));
}
int _noopt chmod(const char *path, mode_t mode) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_CHMOD, path, mode));
}
int _noopt chdir(const char *dir) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NR_CHDIR, dir));
}
int _noopt mknod(const char *filename, int mode, unsigned int dev) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_MKNOD, filename, mode, dev));
}
char *_noopt getcwd(char *buf, size_t lim) {
    if (SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_GETCWD, buf, lim)) == 0) {
        return buf;
    } else {
        return NULL;
    }
}
int _noopt isatty(int file) {
    struct winsize ws;
    if (ioctl(file, TIOCGWINSZ, &ws) != 0) {
        return 0;
    }
    return 1;
}

////

void _noopt _exit(int status) {
    (void) ASM_SYSCALL1(SYSCALL_NR_EXIT, status);
    while (1);
}
int _noopt execve(char *name, char **argv, char **env) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NR_EXECVE, name, argv, env));
}
int _noopt fork() {
    return SET_ERRNO(int, ASM_SYSCALL0(SYSCALL_NR_FORK));
}
int _noopt getpid() {
    return ASM_SYSCALL0(SYSCALL_NR_GETPID);
}
int _noopt kill(int pid, int sig) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_KILL, pid, sig));
}
int _noopt waitpid(pid_t pid, int *status, int _0) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NRX_WAITPID, pid, status));
}
int _noopt gettimeofday(struct timeval *p, void *tz) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_GETTIMEOFDAY, p, tz));
}
int _noopt setpgid(pid_t pid, pid_t pgid) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_SETPGID, pid, pgid));
}
pid_t _noopt getpgid(pid_t pid) {
    return SET_ERRNO(pid_t, ASM_SYSCALL1(SYSCALL_NR_GETPGID, pid));
}

////

int _noopt setgid(int gid) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NR_SETGID, gid));
}
int _noopt setuid(int uid) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NR_SETUID, uid));
}
uid_t _noopt getuid(void) {
    return ASM_SYSCALL0(SYSCALL_NR_GETUID);
}
gid_t _noopt getgid(void) {
    return ASM_SYSCALL0(SYSCALL_NR_GETGID);
}

////

int _noopt reboot(int magic1, int magic2, unsigned int cmd, void *arg) {
    return SET_ERRNO(int, ASM_SYSCALL4(SYSCALL_NR_REBOOT, magic1, magic2, cmd, arg));
}
int _noopt mount(const char *src, const char *dst, const char *fs, unsigned long flags, void *data) {
    return SET_ERRNO(int, ASM_SYSCALL5(SYSCALL_NR_MOUNT, src, dst, fs, flags, data));
}
int _noopt umount(const char *dir) {
    return SET_ERRNO(int, ASM_SYSCALL1(SYSCALL_NRX_UMOUNT, dir));
}
int _noopt nanosleep(const struct timespec *req, struct timespec *rem) {
    return SET_ERRNO(int, ASM_SYSCALL2(SYSCALL_NR_NANOSLEEP, req, rem));
}

////

int _noopt netctl(const char *iface, uint32_t cmd, void *arg) {
    return SET_ERRNO(int, ASM_SYSCALL3(SYSCALL_NRX_NETCTL, iface, cmd, arg));
}

// Not implemented yet:

int _noopt link(char *old, char *new) {
    // TODO
    return -1;
}
clock_t _noopt times(struct tms *buf) {
    // TODO
    return 0;
}
int _noopt wait(int *status) {
    // TODO
    return -1;
}
