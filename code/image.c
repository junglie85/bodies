#include "image.h"

#include "memory.h"

static void *stbi_malloc_wrapper(size_t size)
{
    heap_allocator_t *heap = mem_system_allocator();
    return heap_alloc(heap, size, MEM_DEFAULT_ALIGN);
}

static void *stbi_realloc_wrapper(void *ptr, size_t new_size)
{
    heap_allocator_t *heap = mem_system_allocator();
    return heap_realloc(heap, ptr, new_size, MEM_DEFAULT_ALIGN);
}

static void stbi_free_wrapper(void *ptr)
{
    heap_allocator_t *heap = mem_system_allocator();
    heap_dealloc(heap, ptr);
}

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#define STBI_MALLOC(sz)        stbi_malloc_wrapper(sz)
#define STBI_REALLOC(p, newsz) stbi_realloc_wrapper(p, newsz)
#define STBI_FREE(p)           stbi_free_wrapper(p)
#include <SDL3/SDL.h>
#include <stb_image.h>

#include "log.h"

image_t load_image(const char *filename)
{
    stack_allocator_t *scratch = mem_scratch_allocator();

    // todo: get base path once and cache it at startup?
    const char *base_path = SDL_GetBasePath();
    char *full_path = NULL;

    // todo: better file path handling.
    int32_t len = SDL_snprintf(NULL, 0, "%s../data/%s", base_path, filename);
    full_path = stack_alloc(scratch, len + 1, MEM_DEFAULT_ALIGN);
    if (full_path == NULL) {
        log_error(LOG_CATEGORY_IMAGE, "Failed to allocate %z bytes in scratch buffer.", len);
        return (image_t){};
    }
    SDL_snprintf(full_path, len + 1, "%s../data/%s", base_path, filename);

    SDL_IOStream *io = SDL_IOFromFile(full_path, "rb");
    if (io == NULL) {
        log_error(LOG_CATEGORY_IMAGE, "Failed to open file %s, %s.", full_path, SDL_GetError());
        stack_dealloc(scratch, full_path);
        return (image_t){};
    }

    const int64_t required_size = SDL_GetIOSize(io);
    if (required_size < 0) {
        log_error(LOG_CATEGORY_IMAGE, "Failed to determine file size %s.", SDL_GetError());
        SDL_CloseIO(io);
        stack_dealloc(scratch, full_path);
        return (image_t){};
    }

    void *buffer = stack_alloc(scratch, required_size, MEM_DEFAULT_ALIGN);
    if (buffer == NULL) {
        log_error(LOG_CATEGORY_IMAGE, "Failed to allocate %z bytes in scratch buffer.", required_size);
        SDL_CloseIO(io);
        stack_dealloc(scratch, full_path);
        return (image_t){};
    }

    const size_t size = SDL_ReadIO(io, buffer, required_size);
    if (size != required_size) {
        if (size == 0) {
            log_error(LOG_CATEGORY_IMAGE, "Failed to load image %s data, %s.", full_path, SDL_GetError());
        } else {
            log_warn(LOG_CATEGORY_IMAGE, "Load image %s data requested %lld bytes but only read %lld bytes.", full_path, required_size, size);
        }

        stack_dealloc(scratch, buffer);
        stack_dealloc(scratch, full_path);
        SDL_CloseIO(io);

        return (image_t){};
    }

    SDL_CloseIO(io);

    const int32_t required_channels = 4;
    int32_t width, height, comp;
    void *data = stbi_load_from_memory(buffer, (int32_t)size, &width, &height, &comp, required_channels);
    if (data == NULL) {
        log_error(LOG_CATEGORY_MEMORY, "Failed to load image %s from data, %s", full_path, stbi_failure_reason());
        stack_dealloc(scratch, buffer);
        stack_dealloc(scratch, full_path);
        return (image_t){};
    }

    stack_dealloc(scratch, buffer);
    stack_dealloc(scratch, full_path);

    const int32_t pitch = width * required_channels;

    return (image_t){
        .data = data,
        .width = width,
        .height = height,
        .pitch = pitch,
        .format = IMAGE_FORMAT_R8G8B8A8_UNORM,
    };
}

void free_image(image_t *image)
{
    if (image != NULL) {
        stbi_image_free(image->data);
        image->data = NULL;
        image->width = 0;
        image->height = 0;
    }
}

int32_t image_bytes_per_pixel(const image_t *const image)
{
    if (image == NULL) {
        return 0;
    }

    switch (image->format) {
    case IMAGE_FORMAT_R8G8B8A8_UNORM:
        return 4;
    default:
        return 0;
    }
}
