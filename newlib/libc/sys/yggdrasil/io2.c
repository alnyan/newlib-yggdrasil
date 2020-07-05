#include "sys/gets2.h"
#include "sys/dirent.h"
#include "sys/termios.h"
#include <sys/fcntl.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <ygg/syscall.h>
#include "syscalls.h"

// TODO: move somewhere
int usleep(unsigned long usec) {
    struct timespec req = {
        .tv_sec = usec / 1000000,
        .tv_nsec = (usec % 1000000) * 1000
    };
    return nanosleep(&req, NULL);
}

extern void *malloc(size_t count);
extern void free(void *p);

struct DIR_private {
    int fd;
    union {
        struct dirent buf;
        char __data[512];
    };
};

DIR *opendir(const char *path) {
    int fd;
    DIR *res;

    if (!(res = malloc(sizeof(struct DIR_private)))) {
        errno = ENOMEM;
        return NULL;
    }


    if ((fd = open(path, O_DIRECTORY | O_RDONLY, 0)) < 0) {
        fd = errno;
        free(res);
        errno = fd;
        // errno is set
        return NULL;
    }

    res->fd = fd;
    memset(&res->buf, 0, sizeof(res->buf));

    return res;
}

int closedir(DIR *dirp) {
    if (!dirp) {
        return -1;
    }
    close(dirp->fd);
    free(dirp);
    return 0;
}

static ssize_t _noopt _readdir(int fd, struct dirent *entp) {
    return SET_ERRNO(ssize_t, ASM_SYSCALL2(SYSCALL_NR_READDIR, fd, entp));
}

struct dirent *readdir(DIR *dirp) {
    ssize_t res;
    if (!dirp) {
        errno = EBADF;
        return NULL;
    }
    if ((res = _readdir(dirp->fd, &dirp->buf)) <= 0) {
        return NULL;
    }
    return &dirp->buf;
}

int tcgetattr(int fd, struct termios *tc) {
    return ioctl(fd, TCGETS, tc);
}

int tcsetattr(int fd, int opt, const struct termios *tc) {
    switch (opt) {
    case TCSANOW:
    // No other variants now
    default:
        return ioctl(fd, TCSETS, (void *) tc);
    }
}

int tcsetpgrp(int fd, int pgrp) {
    ioctl(fd, TIOCSPGRP, &pgrp);
}
