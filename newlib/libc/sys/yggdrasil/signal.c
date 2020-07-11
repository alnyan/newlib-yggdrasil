//#include <bits/syscall.h>
#include <ygg/signum.h>
#include <ygg/syscall.h>
//#include "include/signal.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include "syscalls.h"
#include "debug.h"

extern void *malloc(size_t size);
extern __attribute__((noreturn)) void exit(int code);
extern int sigaltstack(const stack_t *ss, stack_t *old_ss);
extern char *strsignal(int signum);

static void __attribute__((optimize("O0"))) __kernel_sigentry(uintptr_t addr) {
    (void) ASM_SYSCALL1(SYSCALL_NRX_SIGENTRY, addr);
}
static void __attribute__((optimize("O0"), noreturn)) __kernel_sigreturn(void) {
    (void) ASM_SYSCALL0(SYSCALL_NR_SIGRETURN);
    while (1);
}

static sighandler_t signal_handlers[NSIG] = {0};

void __SIG_IGN(int signum) {
    const char *str;
    if (signum < 32 && (str = strsignal(signum))) {
        printf("Ignored: %s\n", str);
    } else {
        printf("Ignored: signal %d\n", signum);
    }
}

void __SIG_DFL(int signum) {
    int pid = getpid();
    const char *str;

    if (!signum || signum >= 32) {
        printf("%d: invalid signal received: %d\n", pid, signum);
    } else {
        if ((str = strsignal(signum))) {
            printf("%d: %s\n", pid, str);
        } else {
            printf("%d: signal %d\n", pid, signum);
        }
    }

    exit(signum << 8);
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
#define SIGNAL_STACK_SIZE   4096
    void *alt_stack = malloc(SIGNAL_STACK_SIZE);
    if (!alt_stack) {
        const char *msg = "Failed to allocate signal stack\n";
        write(STDERR_FILENO, msg, sizeof(msg) - 1);
        exit(-1);
    }
    stack_t ss = {
        .ss_sp = alt_stack,
        .ss_size = SIGNAL_STACK_SIZE
    };

    for (size_t i = 0; i < 32; ++i) {
        signal_handlers[i] = __SIG_DFL;
    }
    signal_handlers[SIGUSR1] = __libc_debug_trigger;
    signal_handlers[SIGUSR2] = __SIG_IGN;

    __kernel_sigentry((uintptr_t) __libc_signal_handle);
    // Set main thread's signal stack
    sigaltstack(&ss, NULL);
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

