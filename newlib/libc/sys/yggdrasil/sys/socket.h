#pragma once
#include <ygg/socket.h>

#if defined(__cplusplus)
extern "C" {
#endif

int socket(int domain, int type, int protocol);
int bind(int fd, const struct sockaddr *sa, socklen_t salen);
int accept(int fd, const struct sockaddr *sa, socklen_t *salen);
int connect(int fd, const struct sockaddr *sa, socklen_t salen);

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src, socklen_t *addrlen);
ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *src, socklen_t *addrlen);

ssize_t recv(int fd, void *buf, size_t len, int flags);
ssize_t send(int fd, const void *buf, size_t len, int flags);

#if defined(__cplusplus)
}
#endif
