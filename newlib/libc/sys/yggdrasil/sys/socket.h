#pragma once
#include <ygg/socket.h>

int socket(int domain, int type, int protocol);
int bind(int fd, const struct sockaddr *sa, const socklen_t salen);

ssize_t recvfrom(int fd, void *buf, size_t len, int flags, struct sockaddr *src, socklen_t *addrlen);
ssize_t sendto(int fd, const void *buf, size_t len, int flags, const struct sockaddr *src, socklen_t *addrlen);
