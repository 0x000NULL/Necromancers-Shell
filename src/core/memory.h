#ifndef MEMORY_H
#define MEMORY_H

#include <stddef.h>
#include <stdbool.h>

/**
 * Memory Pool Allocator
 *
 * Provides fast, fixed-size block allocation with minimal fragmentation.
 * Tracks allocations for debugging and leak detection.
 *
 * Usage:
 *   MemoryPool* pool = pool_create(1024, 100); // 100 blocks of 1024 bytes
 *   void* ptr = pool_alloc(pool);
 *   pool_free(pool, ptr);
 *   pool_destroy(pool);
 */

/* Opaque pool structure */
typedef struct MemoryPool MemoryPool;

/* Memory statistics */
typedef struct {
    size_t total_bytes;        /* Total pool size */
    size_t block_size;         /* Size of each block */
    size_t block_count;        /* Total blocks */
    size_t allocated_blocks;   /* Currently allocated */
    size_t peak_usage;         /* Peak allocation count */
    size_t total_allocs;       /* Lifetime allocations */
    size_t total_frees;        /* Lifetime frees */
} MemoryStats;

/**
 * Create a memory pool
 *
 * @param block_size Size of each block (bytes)
 * @param block_count Number of blocks
 * @return Pool pointer or NULL on failure
 */
MemoryPool* pool_create(size_t block_size, size_t block_count);

/**
 * Destroy a memory pool
 * Logs warning if blocks still allocated
 *
 * @param pool Pool to destroy
 */
void pool_destroy(MemoryPool* pool);

/**
 * Allocate a block from pool
 *
 * @param pool Pool to allocate from
 * @return Pointer to block or NULL if pool exhausted
 */
void* pool_alloc(MemoryPool* pool);

/**
 * Free a block back to pool
 *
 * @param pool Pool that owns the block
 * @param ptr Block to free (must be from this pool)
 */
void pool_free(MemoryPool* pool, void* ptr);

/**
 * Reset pool (free all blocks at once)
 * Much faster than individual frees
 *
 * @param pool Pool to reset
 */
void pool_reset(MemoryPool* pool);

/**
 * Get pool statistics
 *
 * @param pool Pool to query
 * @param stats Output statistics
 */
void pool_get_stats(const MemoryPool* pool, MemoryStats* stats);

/**
 * Check for memory leaks
 * Logs any allocated blocks
 *
 * @param pool Pool to check
 * @return true if leaks found
 */
bool pool_check_leaks(const MemoryPool* pool);

/**
 * Print pool statistics (uses logger)
 *
 * @param pool Pool to print
 */
void pool_print_stats(const MemoryPool* pool);

#endif /* MEMORY_H */
