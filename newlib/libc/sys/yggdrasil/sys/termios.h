#pragma once
#include <ygg/termios.h>

#if defined(__cplusplus)
extern "C" {
#endif

int tcgetattr(int fd, struct termios *termios_p);
int tcsetattr(int fd, int optional_actions, const struct termios *termios_p);

#if defined(__cplusplus)
}
#endif

