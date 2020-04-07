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
    ygg_debug_trace("zone_create", (uintptr_t) zone, zone->size);

    return zone;
}

static void zone_destroy(struct zone *zone) {
    assert(!((uintptr_t) zone & 0xFFF));
    ygg_debug_trace("zone_destroy", (uintptr_t) zone, zone->size);
    munmap(zone, zone->size);
}

///

// When splitting, at least this much memory should be available in next chunk
#define SPLIT_MINIMUM   32
// Small zone allocation function
static void *zone_alloc(struct zone *zone, size_t size) {
    size = (size + 0xF) & ~0xF;
    assert(zone->size > size);

    struct block *head = (struct block *) &zone[1];
    assert((head->flags & BLOCK_MAGIC) == BLOCK_MAGIC);

    for (struct block *blk = head; blk; blk = blk->next) {
        if (blk->flags & BLOCK_ALLOC) {
            continue;
        }

        if (size + sizeof(struct block) + SPLIT_MINIMUM <= blk->size) {
            // Split this chunk
            struct block *new_blk = (void *) &blk[1] + size;
            new_blk->size = blk->size - sizeof(struct block) - size;
            new_blk->flags = BLOCK_MAGIC;
            new_blk->next = blk->next;
            new_blk->prev = blk;

            blk->next = new_blk;
            blk->size = size;
            blk->flags |= BLOCK_ALLOC;
            ygg_debug_trace("malloc", size, &blk[1]);

            return &blk[1];
        } else if (size <= blk->size) {
            // Just mark the chunk as "used"
            blk->flags |= BLOCK_ALLOC;
            ygg_debug_trace("malloc", size, &blk[1]);
            return &blk[1];
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
    ygg_debug_trace("free", ptr, block->size);

    if ((block->flags & BLOCK_MAGIC) != BLOCK_MAGIC) {
        while (1);
    }
    if (!(block->flags & BLOCK_ALLOC)) {
        while (1);
    }

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
        assert(old_block->flags & BLOCK_MAGIC == BLOCK_MAGIC);
        memcpy(ptr, old, old_block->size);
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
}
