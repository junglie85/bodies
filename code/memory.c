#include "memory.h"

#include <SDL3/SDL.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <tlsf.h>

#include "log.h"

typedef struct memory_system_t memory_system_t;
struct memory_system_t
{
    heap_allocator_t system;
    stack_allocator_t scratch;
};

static memory_system_t g_memory_system;

// O--------------------------------------------------------------------------O
// | Forward Declarations                                                     |
// O--------------------------------------------------------------------------O

size_t memory_align(size_t size, size_t align);

// O--------------------------------------------------------------------------O
// | Memory Stats                                                             |
// O--------------------------------------------------------------------------O

typedef struct memory_stats_t memory_stats_t;
struct memory_stats_t
{
    size_t allocated_bytes;
    size_t total_bytes;
    int32_t allocation_count;
};

void memory_stats_add(memory_stats_t *stats, size_t size)
{
    if (size) {
        stats->allocated_bytes += size;
        ++stats->allocation_count;
    }
}

// O--------------------------------------------------------------------------O
// | Heap Allocator                                                           |
// O--------------------------------------------------------------------------O

static void heap_pool_walker(void *mem, size_t size, int used, void *user)
{
    memory_stats_t *stats = (memory_stats_t *)user;
    memory_stats_add(stats, used ? size : 0);

    if (used) {
        log_warn(LOG_CATEGORY_MEMORY, "Found active allocation. Address %p, size %llu.", mem, size);
    }
}

void heap_init(heap_allocator_t *a, size_t size, void *mem)
{
    a->mem = mem;
    a->total_size = size;
    a->allocated_size = 0;

    a->tlsf = tlsf_create_with_pool(a->mem, a->total_size);

    log_info(LOG_CATEGORY_MEMORY, "Heap allocator initialised with size %llu bytes.", size);
}

void *heap_deinit(heap_allocator_t *a)
{
    memory_stats_t stats = { .allocated_bytes = 0, .total_bytes = a->total_size, .allocation_count = 0 };
    pool_t pool = tlsf_get_pool(a->tlsf);
    tlsf_walk_pool(pool, heap_pool_walker, (void *)&stats);

    // todo: I don't think allocated_size is ever used and can probably be gotten rid of.
    // todo: overhaul the MEMORY_STATS feature.
    if (stats.allocated_bytes)  {
        log_warn(LOG_CATEGORY_MEMORY, "Heap allocator deinitialised. Allocated memory detected. Size %llu, allocated %llu.", stats.total_bytes, stats.allocated_bytes);
    } else {
        log_info(LOG_CATEGORY_MEMORY, "Heap allocator deinitialised. All memory free.");
    }

    // todo: assert that all memory is freed?

    tlsf_destroy(a->tlsf);

    return a->mem;
}

void *heap_alloc(heap_allocator_t *a, size_t size, size_t align)
{
#if FEATURE_MEMORY_STATS
    void *mem = align == 1 ? tlsf_malloc(a->tlsf, size) : tlsf_memalign(a->tlsf, align, size);
    size_t allocated_size = tlsf_block_size(mem);
    a->allocated_size += allocated_size;
    return mem;
#else
    void *mem = tlsf_malloc(a->tlsf, size);
    return mem;
#endif
}

void *heap_calloc(heap_allocator_t *a, size_t count, size_t size, size_t align)
{
    size_t req = count * size;
#if FEATURE_MEMORY_STATS
    void *mem = align == 1 ? tlsf_malloc(a->tlsf, req) : tlsf_memalign(a->tlsf, align, req);
    size_t allocated_size = tlsf_block_size(mem);
    a->allocated_size += allocated_size;
    memset(mem, 0, allocated_size);
    return mem;
#else
    void *mem = tlsf_malloc(a->tlsf, req);
    memset(mem, 0, req);
    return mem;
#endif
}

void *heap_realloc(heap_allocator_t *a, void *mem, size_t size, size_t align)
{
    // tlsf should have enough info in its header to handle alignment.
    (void)align;

#if FEATURE_MEMORY_STATS
    size_t original_size = tlsf_block_size(mem);

    void *new_mem = tlsf_realloc(a->tlsf, mem, size);

    if (new_mem && new_mem != mem) {
        size_t allocated_size = tlsf_block_size(new_mem);
        size_t diff = allocated_size - original_size;
        a->allocated_size += diff;
    }

    return new_mem;
#else
    void *new_mem = tlsf_realloc(a->tlsf, mem, size);
    return new_mem;
#endif
}

void heap_dealloc(heap_allocator_t *a, void *mem)
{
#if FEATURE_MEMORY_STATS
    size_t allocated_size = tlsf_block_size(mem);
    a->allocated_size -= allocated_size;
    tlsf_free(a->tlsf, mem);
#else
    tlsf_free(a->tlsf, mem);
#endif
}

// O--------------------------------------------------------------------------O
// | Linear Allocator                                                         |
// O--------------------------------------------------------------------------O

void linear_init(linear_allocator_t *a, size_t size, void *mem)
{
    a->mem = mem;
    a->total_size = size;
    a->allocated_size = 0;

    log_info(LOG_CATEGORY_MEMORY, "Linear allocator initialised with size %llu bytes.", size);
}

void *linear_deinit(linear_allocator_t *a)
{
    if (a->allocated_size != 0) {
        log_warn(LOG_CATEGORY_MEMORY, "Linear allocator deinitialised. Allocated memory detected. Size %llu, allocated %llu.", a->total_size, a->allocated_size);
    } else {
        log_info(LOG_CATEGORY_MEMORY, "Linear allocator deinitialised. All memory free.");
    }

    linear_reset(a);
    return a->mem;
}

void *linear_alloc(linear_allocator_t *a, size_t size, size_t align)
{
    assert(size > 0);

    size_t offset = memory_align(a->allocated_size, align);
    assert(offset < a->total_size);

    size_t allocated_size = offset + size;
    if (allocated_size > a->total_size) {
        assert(false && "Overflow");
        return NULL;
    }

    a->allocated_size = allocated_size;
    return (uint8_t *)a->mem + offset;
}

void linear_reset(linear_allocator_t *a)
{
    a->allocated_size = 0;
}

// O--------------------------------------------------------------------------O
// | Stack Allocator                                                          |
// O--------------------------------------------------------------------------O

void stack_init(stack_allocator_t *a, size_t size, void *mem)
{
    a->mem = mem;
    a->total_size = size;
    a->allocated_size = 0;

    log_info(LOG_CATEGORY_MEMORY, "Stack allocator initialised with size %llu bytes.", size);
}

void *stack_deinit(stack_allocator_t *a)
{
    if (a->allocated_size != 0) {
        log_warn(LOG_CATEGORY_MEMORY, "Stack allocator deinitialised. Allocated memory detected. Size %llu, allocated %llu.", a->total_size, a->allocated_size);
    } else {
        log_info(LOG_CATEGORY_MEMORY, "Stack allocator deinitialised. All memory free.");
    }

    stack_reset(a);
    return a->mem;
}

void *stack_alloc(stack_allocator_t *a, size_t size, size_t align)
{
    size_t offset = memory_align(a->allocated_size, align);
    assert(offset < a->total_size);

    size_t allocated_size = offset + size;
    if (allocated_size > a->total_size) {
        assert(false && "Overflow");
        return NULL;
    }

    a->allocated_size = allocated_size;
    return (uint8_t *)a->mem + offset;
}

void stack_dealloc(stack_allocator_t *a, void *mem)
{
    uint8_t *ptr = (uint8_t *)mem;

    assert(ptr >= a->mem);
    assert(ptr < (uint8_t *)a->mem + a->total_size);
    assert(ptr < (uint8_t *)a->mem + a->allocated_size);

    size_t size = ptr - a->mem;
    a->allocated_size = size;
}

size_t stack_get_marker(stack_allocator_t *a)
{
    return a->allocated_size;
}

void stack_dealloc_marker(stack_allocator_t *a, size_t marker)
{
    size_t diff = marker - a->allocated_size;
    if (diff > 0) {
        a->allocated_size = marker;
    }
}

void stack_reset(stack_allocator_t *a)
{
    a->allocated_size = 0;
}

// O--------------------------------------------------------------------------O
// | Memory System                                                            |
// O--------------------------------------------------------------------------O

static void *SDL_malloc_on_heap(size_t size)
{
    heap_allocator_t *system = mem_system_allocator();
    void *mem = heap_alloc(system, size, MEM_DEFAULT_ALIGN);
    return mem;
}

static void *SDL_calloc_on_heap(size_t nmemb, size_t size)
{
    assert(nmemb > 0);
    assert(size > 0);

    heap_allocator_t *system = mem_system_allocator();
    void *mem = heap_calloc(system, nmemb, size, MEM_DEFAULT_ALIGN);
    return mem;
}

void *SDL_realloc_on_heap(void *mem, size_t size)
{
    heap_allocator_t *system = mem_system_allocator();
    void *new_mem = heap_realloc(system, mem, size, MEM_DEFAULT_ALIGN);
    return new_mem;
}

void SDL_free_on_heap(void *mem)
{
    heap_allocator_t *system = mem_system_allocator();
    heap_dealloc(system, mem);
}

bool start_memory_system(memory_system_desc_t desc)
{
    // Tell SDL to use our memory allocation functions.
    if (!SDL_SetMemoryFunctions(SDL_malloc_on_heap, SDL_calloc_on_heap, SDL_realloc_on_heap, SDL_free_on_heap)) {
        log_error(LOG_CATEGORY_MEMORY, "Failed to set memory allocation functions. %s", SDL_GetError());
        return false;
    }

    void *system_mem = malloc(desc.system_memory_size);
    heap_init(&g_memory_system.system, desc.system_memory_size, system_mem);

    void *scratch_mem = heap_alloc(&g_memory_system.system, desc.scratch_memory_size, MEM_DEFAULT_ALIGN);
    stack_init(&g_memory_system.scratch, desc.scratch_memory_size, scratch_mem);

    log_info(LOG_CATEGORY_MEMORY, "Memory system started.");

    return true;
}

void stop_memory_system(void)
{
    log_info(LOG_CATEGORY_MEMORY, "Memory system stopped.");

    void *scratch_mem = stack_deinit(&g_memory_system.scratch);
    heap_dealloc(&g_memory_system.system, scratch_mem);

    void *system_mem = heap_deinit(&g_memory_system.system);
    free(system_mem);
}

heap_allocator_t *mem_system_allocator(void)
{
    return &g_memory_system.system;
}

stack_allocator_t *mem_scratch_allocator(void)
{
    return &g_memory_system.scratch;
}

// O--------------------------------------------------------------------------O
// | Helper Macros and Functions                                              |
// O--------------------------------------------------------------------------O

size_t memory_align(size_t size, size_t align)
{
    return (size + (align - 1)) & ~(align - 1);
}
