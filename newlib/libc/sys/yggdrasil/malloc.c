#include <ygg/syscall.h>
#include <sys/mman.h>
#include "syscalls.h"
#include <assert.h>
#include <string.h>
#include <errno.h>

#include "list.h"

struct _reent;

#define BLOCK_ALLOC                 1
#define BLOCK_MMAP                  2
#define BLOCK_MAGIC_MASK            0xFFFFFFC
#define BLOCK_MAGIC                 0xBAD8EA0

// Fits ~56 blocks of 256
#define SMALL_ZONE_ELEM             256
#define SMALL_ZONE_SIZE             (4 * 0x1000)
// Fits ~31 block of 2K
#define MID_ZONE_ELEM               2048
#define MID_ZONE_SIZE               (16 * 0x1000)
// Fits ~15 blocks of 8K
#define LARGE_ZONE_ELEM             8192
#define LARGE_ZONE_SIZE             (32 * 0x1000)
// Larger objects are allocated directly using mmap()

struct block {
    struct block *prev, *next;
    uint32_t flags, size;
};

struct zone {
    size_t size;
    struct list_head list;
};

static LIST_HEAD(small_zone_list);
static LIST_HEAD(mid_zone_list);
static LIST_HEAD(large_zone_list);

static struct zone *zone_create(size_t size) {
    void *pages = mmap(NULL, size, 0, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (pages == MAP_FAILED) {
        return NULL;
    }

    struct zone *zone = pages;
    struct block *head = pages + sizeof(struct zone);

    zone->size = size;
    list_head_init(&zone->list);

    head->size = size - sizeof(struct zone);
    head->flags = BLOCK_MAGIC;
    head->prev = NULL;
    head->next = NULL;
    ygg_debug_trace("zone_create(ptr=%p, size=%u)\n", (uintptr_t) zone, zone->size);

    return zone;
}

static void zone_destroy(struct zone *zone) {
    assert(!((uintptr_t) zone & 0xFFF));
    ygg_debug_trace("zone_destroy(ptr=%p, size=%u)\n", (uintptr_t) zone, zone->size);
    munmap(zone, zone->size);
}

///

static int __libc_validate_zone(struct zone *zone) {
    struct block *block;

    for (block = (struct block *) &zone[1]; block; block = block->next) {
        if ((block->flags & BLOCK_MAGIC_MASK) != BLOCK_MAGIC) {
            ygg_debug_trace("Invalid block magic: b(ptr=%p, size=%S, flags=%08x)\n", block,
                    block->size, block->flags);
            return -1;
        }

        if (block->next) {
            uintptr_t nextptr = ((uintptr_t) &block[1]) + block->size;
            if (nextptr != (uintptr_t) block->next) {
                ygg_debug_trace("Invalid block->next: b0(ptr=%p, size=%S), b1=%p\n", block,
                        block->size, block->next);
                return -1;
            }
        }
    }

    return 0;
}

static void __libc_validate_zones(const char *cause) {
    struct zone *zone;

    list_for_each_entry(zone, &small_zone_list, list) {
        if (__libc_validate_zone(zone) != 0) {
            ygg_debug_trace("Error in small zone @ %p\n", zone);
            while (1);
        }
    }
    list_for_each_entry(zone, &mid_zone_list, list) {
        if (__libc_validate_zone(zone) != 0) {
            ygg_debug_trace("Error in medium zone @ %p\n", zone);
            while (1);
        }
    }
    list_for_each_entry(zone, &large_zone_list, list) {
        if (__libc_validate_zone(zone) != 0) {
            ygg_debug_trace("Error in large zone @ %p\n", zone);
            while (1);
        }
    }
}

// Small zone allocation function
static void *zone_alloc(struct zone *zone, size_t count) {
    struct block *begin = (struct block *) &zone[1];

    // Some alignment fuck ups led me to this
    count = (count + 15) & ~15;

    __libc_validate_zones("pre-malloc");

    for (struct block *block = begin; block; block = block->next) {
        if (block->flags & BLOCK_ALLOC) {
            continue;
        }

        if (count == block->size) {
            block->flags |= BLOCK_ALLOC;
            void *ptr = &block[1];
            memset(ptr, 0, count);
            ygg_debug_trace("malloc(%S) -> %p\n", count, ptr);
            __libc_validate_zones("post-malloc");
            return ptr;
        } else if (block->size >= count + sizeof(struct block)) {
            // Insert new block after this one
            struct block *cur_next = block->next;
            struct block *new_block = (struct block *) (((uintptr_t) block) + sizeof(struct block) + count);
            if (cur_next) {
                cur_next->prev = new_block;
            }
            new_block->next = cur_next;
            new_block->prev = block;
            new_block->size = block->size - sizeof(struct block) - count;
            new_block->flags = BLOCK_MAGIC;
            block->next = new_block;
            block->size = count;
            block->flags |= 1;
            void *ptr = &block[1];
            memset(ptr, 0, count);
            ygg_debug_trace("malloc(%S) -> %p\n", count, ptr);
            __libc_validate_zones("post-malloc");
            return ptr;
        }
    }

    return NULL;
}

///

static void *alloc_from(struct list_head *head, size_t zone_size, size_t size) {
    struct zone *zone;
    void *ptr;

try_again:
    list_for_each_entry(zone, head, list) {
        if ((ptr = zone_alloc(zone, size))) {
            return ptr;
        }
    }

    zone = zone_create(zone_size);
    if (!zone) {
        return NULL;
    }
    list_add(&zone->list, head);

    goto try_again;
}

void *_malloc_r(struct _reent *ree, size_t size) {
    if (size <= SMALL_ZONE_ELEM) {
        return alloc_from(&small_zone_list, SMALL_ZONE_SIZE, size);
    } else if (size <= MID_ZONE_ELEM) {
        return alloc_from(&mid_zone_list, MID_ZONE_SIZE, size);
    } else if (size <= LARGE_ZONE_ELEM) {
        return alloc_from(&large_zone_list, LARGE_ZONE_SIZE, size);
    } else {
        while (1);
    }
}

void _free_r(struct _reent *ree, void *ptr) {
    if (!ptr) {
        return;
    }
    struct block *block = ptr - sizeof(struct block);
    ygg_debug_trace("free(ptr=%p, size=%u)\n", ptr, block->size);

    if ((block->flags & BLOCK_MAGIC_MASK) != BLOCK_MAGIC) {
        ygg_debug_trace("Tried to free an invalid block\n");
        while (1);
    }
    if (!(block->flags & BLOCK_ALLOC)) {
        ygg_debug_trace("Double free error\n");
        while (1);
    }

    __libc_validate_zones("pre-free\n");

    block->flags &= ~BLOCK_ALLOC;
    struct block *prev, *next;
    prev = block->prev;
    next = block->next;

    if (prev && !(prev->flags & BLOCK_ALLOC)) {
        block->flags = 0;
        prev->next = next;
        if (next) {
            next->prev = prev;
        }
        prev->size += block->size + sizeof(struct block);

        block = prev;
    }

    if (next && !(next->flags & BLOCK_ALLOC)) {
        next->flags = 0;

        if (next->next) {
            next->next->prev = block;
        }
        block->next = next->next;
        block->size += next->size + sizeof(struct block);
    }

    if (!block->prev && !block->next) {
        struct zone *zone = (void *) block - sizeof(struct zone);
        assert(!((uintptr_t) zone & 0xFFF));
        list_del(&zone->list);
        zone_destroy(zone);
    }

    __libc_validate_zones("post-free\n");
}

void *_calloc_r(struct _reent *ree, size_t size, size_t nmemb) {
    void *ptr = _malloc_r(ree, size * nmemb);
    if (!ptr) {
        return NULL;
    }
    memset(ptr, 0, size * nmemb);
    return ptr;
}

void *_realloc_r(struct _reent *ree, void *old, size_t size) {
    // TODO: maybe optimize this
    void *ptr = _malloc_r(ree, size);
    if (!ptr) {
        return NULL;
    }


    if (old) {
        struct block *old_block = old - sizeof(struct block);
        size_t less_size = size;
        if (less_size > old_block->size) {
            less_size = old_block->size;
        }
        assert((old_block->flags & BLOCK_MAGIC_MASK) == BLOCK_MAGIC);
        memcpy(ptr, old, less_size);
        _free_r(ree, old);
    }

    return ptr;
}

void *realloc(void *ptr, size_t size) {
    return _realloc_r(NULL, ptr, size);
}

void *malloc(size_t size) {
    return _malloc_r(NULL, size);
}

void free(void *ptr) {
    _free_r(NULL, ptr);
}

static void __libc_dump_zone(struct zone *zone) {
    struct block *block;
    ygg_debug_trace("\tZone @ %p:\n", zone);

    for (block = (struct block *) &zone[1]; block; block = block->next) {
        if ((block->flags & BLOCK_MAGIC_MASK) != BLOCK_MAGIC) {
            ygg_debug_trace("\t\tINVALID BLOCK MAGIC @ %p\n", block);
            break;
        }

        ygg_debug_trace("\t\t%s @ %p, %S\n", (block->flags & BLOCK_ALLOC) ? "USED" : "FREE",
                                             &block[1], block->size);
    }
}

void __libc_malloc_debug(void) {
    struct zone *zone;
    ygg_debug_trace("--- libc malloc debug ---\n");
    ygg_debug_trace("Small zones (%S):\n", SMALL_ZONE_SIZE);
    list_for_each_entry(zone, &small_zone_list, list) {
        __libc_dump_zone(zone);
    }
    ygg_debug_trace("Mid zones (%S):\n", MID_ZONE_SIZE);
    list_for_each_entry(zone, &mid_zone_list, list) {
        __libc_dump_zone(zone);
    }
    ygg_debug_trace("Large zones (%S):\n", LARGE_ZONE_SIZE);
    list_for_each_entry(zone, &large_zone_list, list) {
        __libc_dump_zone(zone);
    }
    ygg_debug_trace("--- --- ---\n");
}
