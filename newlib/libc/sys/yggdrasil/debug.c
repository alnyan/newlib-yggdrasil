extern void __libc_malloc_debug(void);

void __libc_debug_trigger(int p0) {
    __libc_malloc_debug();
}
