#pragma once
#include <sys/types.h>

#define ASM_REGISTER(name) \
        volatile register uint64_t name asm (#name)

#define ASM_SYSCALL0(r0)                    ({ \
        ASM_REGISTER(rax) = (uint64_t) (r0); \
        asm volatile ("syscall":::"memory"); \
        rax; \
    })

#define ASM_SYSCALL1(r0, r1)                ({ \
        ASM_REGISTER(rdi) = (uint64_t) (r1); \
        /*
         * Should be the last one because memory accesses for arguments
         * fuck up %rax
         */ \
        ASM_REGISTER(rax) = (uint64_t) (r0); \
        asm volatile ("syscall":::"memory"); \
        rax; \
    })

#define ASM_SYSCALL2(r0, r1, r2)        ({ \
        ASM_REGISTER(rdi) = (uint64_t) (r1); \
        ASM_REGISTER(rsi) = (uint64_t) (r2); \
        /*
         * Should be the last one because memory accesses for arguments
         * fuck up %rax
         */ \
        ASM_REGISTER(rax) = (uint64_t) (r0); \
        asm volatile ("syscall":::"memory", "rax", "rdi", "rsi", "rdx"); \
        rax; \
    })

#define ASM_SYSCALL3(r0, r1, r2, r3)        ({ \
        ASM_REGISTER(rdi) = (uint64_t) (r1); \
        ASM_REGISTER(rsi) = (uint64_t) (r2); \
        ASM_REGISTER(rdx) = (uint64_t) (r3); \
        /*
         * Should be the last one because memory accesses for arguments
         * fuck up %rax
         */ \
        ASM_REGISTER(rax) = (uint64_t) (r0); \
        asm volatile ("syscall":::"memory", "rax", "rdi", "rsi", "rdx"); \
        rax; \
    })

#define ASM_SYSCALL4(r0, r1, r2, r3, r4)        ({ \
        ASM_REGISTER(rdi) = (uint64_t) (r1); \
        ASM_REGISTER(rsi) = (uint64_t) (r2); \
        ASM_REGISTER(rdx) = (uint64_t) (r3); \
        ASM_REGISTER(r10) = (uint64_t) (r4); \
        /*
         * Should be the last one because memory accesses for arguments
         * fuck up %rax
         */ \
        ASM_REGISTER(rax) = (uint64_t) (r0); \
        asm volatile ("syscall":::"memory", "rax", "rdi", "rsi", "rdx", "r10"); \
        rax; \
    })

#define ASM_SYSCALL5(r0, r1, r2, r3, r4, r5)        ({ \
        ASM_REGISTER(rdi) = (uint64_t) (r1); \
        ASM_REGISTER(rsi) = (uint64_t) (r2); \
        ASM_REGISTER(rdx) = (uint64_t) (r3); \
        ASM_REGISTER(r10) = (uint64_t) (r4); \
        ASM_REGISTER( r8) = (uint64_t) (r5); \
        /*
         * Should be the last one because memory accesses for arguments
         * fuck up %rax
         */ \
        ASM_REGISTER(rax) = (uint64_t) (r0); \
        asm volatile ("syscall":::"memory", "rax", "rdi", "rsi", "rdx", "r10", "r8"); \
        rax; \
    })

#define ASM_SYSCALL6(r0, r1, r2, r3, r4, r5, r6)        ({ \
        ASM_REGISTER(rdi) = (uint64_t) (r1); \
        ASM_REGISTER(rsi) = (uint64_t) (r2); \
        ASM_REGISTER(rdx) = (uint64_t) (r3); \
        ASM_REGISTER(r10) = (uint64_t) (r4); \
        ASM_REGISTER( r8) = (uint64_t) (r5); \
        ASM_REGISTER( r9) = (uint64_t) (r6); \
        /*
         * Should be the last one because memory accesses for arguments
         * fuck up %rax
         */ \
        ASM_REGISTER(rax) = (uint64_t) (r0); \
        asm volatile ("syscall":::"memory", "rax", "rdi", "rsi", "rdx", "r10", "r8", "r9"); \
        rax; \
    })

#define SET_ERRNO(t, r)                     ({ \
        t res = (t) r; \
        if (res < 0) { \
            errno = -res; \
        } \
        ((int) res) < 0 ? (t) -1 : res; \
    })

#define _noopt __attribute__((optimize("O0")))

extern _noopt void ygg_debug_trace(const char *fmt, ...);
