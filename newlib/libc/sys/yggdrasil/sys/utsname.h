#pragma once
#include <ygg/utsname.h>

#if defined(__cplusplus)
extern "C" {
#endif

int uname(struct utsname *buf);

#if defined(__cplusplus)
}
#endif

