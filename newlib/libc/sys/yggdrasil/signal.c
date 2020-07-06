//#include <bits/syscall.h>
#include <ygg/signum.h>
#include <ygg/syscall.h>
//#include "include/signal.h"
#include <signal.h>
#include <stdio.h>

#include "syscalls.h"
#include "debug.h"

static void __attribute__((optimize("O0"))) __kernel_sigentry(uintptr_t addr) {
    (void) ASM_SYSCALL1(SYSCALL_NRX_SIGENTRY, addr);
}
static void __attribute__((optimize("O0"), noreturn)) __kernel_sigreturn(void) {
    (void) ASM_SYSCALL0(SYSCALL_NR_SIGRETURN);
    while (1);
}

static const char *const signal_messages[NSIG] = {
    [SIGKILL - 1] = "killed",
    [SIGINT - 1] = "interrupted",
    [SIGTERM - 1] = "terminated",
    [SIGSYS - 1] = "invalid system call",
    NULL
};
static sighandler_t signal_handlers[NSIG] = {0};

void __SIG_IGN(int signum) {
    if (signum < 32 && signal_messages[signum - 1]) {
        printf("Ignored: %s\n", signal_messages[signum - 1]);
    } else {
        printf("Ignored: signal %d\n", signum);
    }
}

void __SIG_DFL(int signum) {
    int pid = getpid();

    if (!signum || signum >= 32) {
        printf("%d: invalid signal received: %d\n", pid, signum);
    } else {
        if (signal_messages[signum - 1]) {
            printf("%d: %s\n", pid, signal_messages[signum - 1]);
        } else {
            printf("%d: signal %d\n", pid, signum);
        }
    }

    exit(1);
}

static void __libc_signal_handle(int signum) {
    if (signum >= 32) {
        __SIG_DFL(signum);
    } else {
        signal_handlers[signum](signum);
    }

    __kernel_sigreturn();
}

void __libc_signal_init(void) {
    for (size_t i = 0; i < 32; ++i) {
        signal_handlers[i] = __SIG_DFL;
    }
    signal_handlers[SIGUSR1] = __libc_debug_trigger;
    signal_handlers[SIGUSR2] = __SIG_IGN;

    __kernel_sigentry((uintptr_t) __libc_signal_handle);
}

sighandler_t signal(int signum, sighandler_t new_handler) {
    sighandler_t old_handler;

    if (new_handler == SIG_DFL) {
        new_handler = __SIG_DFL;
    } else if (new_handler == SIG_IGN) {
        new_handler = __SIG_IGN;
    } else if (new_handler == SIG_ERR) {
        new_handler = __SIG_DFL;
    }

    if (signum >= 32) {
        printf("Fuck\n");
        exit(1);
    }

    old_handler = signal_handlers[signum];
    signal_handlers[signum] = new_handler;
    return old_handler;
}

//int raise(int signum) {
//    return kill(getpid(), signum);
//}

//void abort(void) {
//    kill(getpid(), SIGABRT);
//}

