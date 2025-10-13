#include "core/memory.h"
#include "utils/logger.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>

/* Block header for tracking */
typedef struct BlockHeader {
    struct BlockHeader* next;  /* Next in free list */
    bool is_allocated;         /* Allocation flag */
    #ifdef DEBUG
    const char* file;          /* Allocation location */
    int line;
    #endif
} BlockHeader;

/* Pool structure */
struct MemoryPool {
    void* memory;              /* Raw memory block */
    BlockHeader* free_list;    /* Free block list */
    size_t block_size;         /* Actual block size (including header) */
    size_t user_block_size;    /* User-visible block size */
    size_t block_count;
    size_t allocated_count;
    size_t peak_usage;
    size_t total_allocs;
    size_t total_frees;
};

/* Helper: align size to pointer boundary */
static inline size_t align_size(size_t size) {
    const size_t alignment = sizeof(void*);
    return (size + alignment - 1) & ~(alignment - 1);
}

MemoryPool* pool_create(size_t block_size, size_t block_count) {
    if (block_size == 0 || block_count == 0) {
        LOG_ERROR("Invalid pool parameters: block_size=%zu, count=%zu",
                  block_size, block_count);
        return NULL;
    }

    /* Allocate pool structure */
    MemoryPool* pool = malloc(sizeof(MemoryPool));
    if (!pool) {
        LOG_ERROR("Failed to allocate pool structure");
        return NULL;
    }

    /* Calculate actual block size (user size + header, aligned) */
    size_t actual_block_size = align_size(block_size + sizeof(BlockHeader));
    size_t total_size = actual_block_size * block_count;

    /* Allocate memory block */
    pool->memory = malloc(total_size);
    if (!pool->memory) {
        LOG_ERROR("Failed to allocate %zu bytes for pool", total_size);
        free(pool);
        return NULL;
    }

    /* Initialize pool */
    pool->block_size = actual_block_size;
    pool->user_block_size = block_size;
    pool->block_count = block_count;
    pool->allocated_count = 0;
    pool->peak_usage = 0;
    pool->total_allocs = 0;
    pool->total_frees = 0;

    /* Build free list */
    pool->free_list = NULL;
    for (size_t i = 0; i < block_count; i++) {
        BlockHeader* block = (BlockHeader*)((char*)pool->memory + i * actual_block_size);
        block->is_allocated = false;
        block->next = pool->free_list;
        pool->free_list = block;
    }

    LOG_DEBUG("Created memory pool: %zu blocks of %zu bytes (%zu total)",
              block_count, block_size, total_size);

    return pool;
}

void pool_destroy(MemoryPool* pool) {
    if (!pool) return;

    /* Check for leaks */
    if (pool->allocated_count > 0) {
        LOG_WARN("Destroying pool with %zu blocks still allocated (potential leak)",
                 pool->allocated_count);
        pool_check_leaks(pool);
    }

    LOG_DEBUG("Destroying pool: %zu total allocs, %zu peak usage",
              pool->total_allocs, pool->peak_usage);

    free(pool->memory);
    free(pool);
}

void* pool_alloc(MemoryPool* pool) {
    if (!pool) {
        LOG_ERROR("pool_alloc called with NULL pool");
        return NULL;
    }

    /* Check if pool exhausted */
    if (!pool->free_list) {
        LOG_ERROR("Pool exhausted: %zu/%zu blocks allocated",
                  pool->allocated_count, pool->block_count);
        return NULL;
    }

    /* Pop from free list */
    BlockHeader* block = pool->free_list;
    pool->free_list = block->next;

    /* Mark as allocated */
    block->is_allocated = true;
    block->next = NULL;

    /* Update statistics */
    pool->allocated_count++;
    pool->total_allocs++;
    if (pool->allocated_count > pool->peak_usage) {
        pool->peak_usage = pool->allocated_count;
    }

    /* Return user-visible pointer (after header) */
    void* user_ptr = (char*)block + sizeof(BlockHeader);

    #ifdef DEBUG
    memset(user_ptr, 0xCD, pool->user_block_size);  /* Debug pattern */
    #endif

    return user_ptr;
}

void pool_free(MemoryPool* pool, void* ptr) {
    if (!pool) {
        LOG_ERROR("pool_free called with NULL pool");
        return;
    }

    if (!ptr) {
        /* Freeing NULL is allowed (like free()) */
        return;
    }

    /* Get block header */
    BlockHeader* block = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));

    /* Validate block */
    #ifdef DEBUG
    /* Check if pointer is within pool bounds */
    ptrdiff_t offset = (char*)block - (char*)pool->memory;
    if (offset < 0 || (size_t)offset >= pool->block_size * pool->block_count) {
        LOG_ERROR("Attempting to free pointer not from this pool: %p", ptr);
        return;
    }

    /* Check if properly aligned */
    if (offset % pool->block_size != 0) {
        LOG_ERROR("Attempting to free misaligned pointer: %p", ptr);
        return;
    }
    #endif

    /* Check double-free */
    if (!block->is_allocated) {
        LOG_ERROR("Double free detected: %p", ptr);
        return;
    }

    /* Mark as free */
    block->is_allocated = false;

    #ifdef DEBUG
    memset(ptr, 0xDD, pool->user_block_size);  /* Debug pattern for freed memory */
    #endif

    /* Push to free list */
    block->next = pool->free_list;
    pool->free_list = block;

    /* Update statistics */
    pool->allocated_count--;
    pool->total_frees++;
}

void pool_reset(MemoryPool* pool) {
    if (!pool) return;

    LOG_DEBUG("Resetting pool: freeing %zu allocated blocks", pool->allocated_count);

    /* Rebuild free list */
    pool->free_list = NULL;
    for (size_t i = 0; i < pool->block_count; i++) {
        BlockHeader* block = (BlockHeader*)((char*)pool->memory + i * pool->block_size);
        block->is_allocated = false;
        block->next = pool->free_list;
        pool->free_list = block;
    }

    pool->allocated_count = 0;
}

void pool_get_stats(const MemoryPool* pool, MemoryStats* stats) {
    if (!pool || !stats) return;

    stats->total_bytes = pool->block_size * pool->block_count;
    stats->block_size = pool->user_block_size;
    stats->block_count = pool->block_count;
    stats->allocated_blocks = pool->allocated_count;
    stats->peak_usage = pool->peak_usage;
    stats->total_allocs = pool->total_allocs;
    stats->total_frees = pool->total_frees;
}

bool pool_check_leaks(const MemoryPool* pool) {
    if (!pool) return false;

    if (pool->allocated_count == 0) {
        LOG_INFO("No memory leaks detected in pool");
        return false;
    }

    LOG_WARN("Memory leak detected: %zu blocks still allocated", pool->allocated_count);

    /* In debug mode, could walk memory and report allocated blocks */
    #ifdef DEBUG
    size_t leaked = 0;
    for (size_t i = 0; i < pool->block_count; i++) {
        BlockHeader* block = (BlockHeader*)((char*)pool->memory + i * pool->block_size);
        if (block->is_allocated) {
            leaked++;
            void* user_ptr = (char*)block + sizeof(BlockHeader);
            LOG_WARN("  Leaked block #%zu at %p", leaked, user_ptr);
        }
    }
    #endif

    return true;
}

void pool_print_stats(const MemoryPool* pool) {
    if (!pool) return;

    MemoryStats stats;
    pool_get_stats(pool, &stats);

    float usage_pct = (float)stats.allocated_blocks / stats.block_count * 100.0f;

    LOG_INFO("Memory Pool Statistics:");
    LOG_INFO("  Block size:     %zu bytes", stats.block_size);
    LOG_INFO("  Block count:    %zu", stats.block_count);
    LOG_INFO("  Total size:     %zu bytes", stats.total_bytes);
    LOG_INFO("  Allocated:      %zu (%.1f%%)", stats.allocated_blocks, usage_pct);
    LOG_INFO("  Peak usage:     %zu", stats.peak_usage);
    LOG_INFO("  Total allocs:   %zu", stats.total_allocs);
    LOG_INFO("  Total frees:    %zu", stats.total_frees);
    LOG_INFO("  Net allocations: %zu", stats.total_allocs - stats.total_frees);
}
