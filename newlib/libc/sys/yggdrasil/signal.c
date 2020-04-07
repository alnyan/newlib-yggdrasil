//#include <bits/syscall.h>
#include <ygg/syscall.h>
#include <signal.h>
#include <stdio.h>

#include "syscalls.h"

typedef _sig_func_ptr sighandler_t;

static void __attribute__((optimize("O0"))) __kernel_sigentry(uintptr_t addr) {
    (void) ASM_SYSCALL1(SYSCALL_NRX_SIGENTRY, addr);
}
static void __attribute__((optimize("O0"), noreturn)) __kernel_sigreturn(void) {
    (void) ASM_SYSCALL0(SYSCALL_NR_SIGRETURN);
    while (1);
}

static const char *const signal_messages[16] = {
    [SIGKILL - 1] = "killed",
    [SIGINT - 1] = "interrupted",
    [SIGTERM - 1] = "terminated",
    [SIGSYS - 1] = "invalid system call",
    NULL
};
static sighandler_t signal_handlers[16] = {0};
static sighandler_t user_handlers[2] = {0};

void __SIG_IGN(int signum) {
    printf("Ignored: %d\n", signum);
}

void __SIG_DFL(int signum) {
    int pid = getpid();

    if (!signum || signum > 16) {
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
    if (signum == SIGUSR1) {
        user_handlers[0](signum);
    } else if (signum == SIGUSR2) {
        user_handlers[1](signum);
    } else if (signum >= 16) {
        __SIG_DFL(signum);
    } else {
        signal_handlers[signum](signum);
    }

    __kernel_sigreturn();
}

void __libc_signal_init(void) {
    for (size_t i = 0; i < 16; ++i) {
        signal_handlers[i] = __SIG_DFL;
    }
    user_handlers[0] = __SIG_IGN;
    user_handlers[1] = __SIG_IGN;

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

    if (signum == SIGUSR1 || signum == SIGUSR2) {
        int n = signum == SIGUSR2;
        old_handler = user_handlers[n];
        user_handlers[n] = new_handler;
        return old_handler;
    } else if (signum >= 16) {
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

