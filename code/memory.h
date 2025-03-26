#ifndef MEMORY_H
#define MEMORY_H

#include <stdbool.h>
#include <stdint.h>

// O--------------------------------------------------------------------------O
// | Heap Allocator                                                           |
// O--------------------------------------------------------------------------O

typedef struct heap_allocator_t heap_allocator_t;
struct heap_allocator_t
{
    void *tlsf;
    void *mem;
    size_t total_size;
    size_t allocated_size;
};

void heap_init(heap_allocator_t *a, size_t size, void *mem);
void *heap_deinit(heap_allocator_t *a);
void *heap_alloc(heap_allocator_t *a, size_t size, size_t alignment);
void *heap_calloc(heap_allocator_t *a, size_t count, size_t size, size_t align);
void *heap_realloc(heap_allocator_t *a, void *mem, size_t size, size_t align);
void heap_dealloc(heap_allocator_t *a, void *mem);

// O--------------------------------------------------------------------------O
// | Linear Allocator                                                         |
// O--------------------------------------------------------------------------O

typedef struct linear_allocator_t linear_allocator_t;
struct linear_allocator_t
{
    void *mem;
    size_t total_size;
    size_t allocated_size;
};

void linear_init(linear_allocator_t *a, size_t size, void *mem);
void *linear_deinit(linear_allocator_t *a);
void *linear_alloc(linear_allocator_t *a, size_t size, size_t alignment);
void linear_reset(linear_allocator_t *a);

// O--------------------------------------------------------------------------O
// | Stack Allocator                                                          |
// O--------------------------------------------------------------------------O

typedef struct stack_allocator_t stack_allocator_t;
struct stack_allocator_t
{
    void *mem;
    size_t total_size;
    size_t allocated_size;
};

void stack_init(stack_allocator_t *a, size_t size, void *mem);
void *stack_deinit(stack_allocator_t *a);
void *stack_alloc(stack_allocator_t *a, size_t size, size_t alignment);
void stack_dealloc(stack_allocator_t *a, void *mem);
size_t stack_get_marker(stack_allocator_t *a);
void stack_dealloc_marker(stack_allocator_t *a, size_t marker);
void stack_reset(stack_allocator_t *a);

// O--------------------------------------------------------------------------O
// | Memory System                                                            |
// O--------------------------------------------------------------------------O

static size_t MEM_DEFAULT_ALIGN = 8; // 64-bits for x64 architectures. Is this sensible?

typedef struct memory_system_desc_t memory_system_desc_t;
struct memory_system_desc_t
{
    size_t system_memory_size;
    size_t scratch_memory_size;
};

bool start_memory_system(memory_system_desc_t desc);
void stop_memory_system(void);

heap_allocator_t *mem_system_allocator(void);
stack_allocator_t *mem_scratch_allocator(void);

// O--------------------------------------------------------------------------O
// | Helper Macros                                                            |
// O--------------------------------------------------------------------------O

#define KB(x) ((x) * 1024)
#define MB(x) ((x) * 1024 * 1024)
#define GB(x) ((x) * 1024 * 1024 * 1024)

#endif // MEMORY_H
