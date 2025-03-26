#include "log.h"

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdio.h>

#include "memory.h"

typedef struct log_entry_t log_entry_t;
struct log_entry_t
{
    int32_t category;
    SDL_LogPriority priority;
    char *message;
    log_entry_t *next;
};

static bool g_started;
static log_entry_t *g_entries_head;

static void log_message_v(log_category_t category, SDL_LogPriority priority, const char *fmt, va_list ap);
static void stash_log_entry(int32_t category, SDL_LogPriority priority, const char *fmt, va_list ap);
static void process_log_entry(void);

void start_log_system(void)
{
    if (g_started) {
        return;
    }

    SDL_SetLogPriority(LOG_CATEGORY_APPLICATION + SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_TRACE);
    SDL_SetLogPriority(LOG_CATEGORY_MEMORY + SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_TRACE);
    SDL_SetLogPriority(LOG_CATEGORY_WINDOW + SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_TRACE);

    if (g_entries_head != NULL) {
        while (g_entries_head != NULL) {
            process_log_entry();
        }
    }

    g_started = true;
}

void log_debug(log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_DEBUG, fmt, ap);
    va_end(ap);
}

void log_error(log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_ERROR, fmt, ap);
    va_end(ap);
}

void log_info(log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_INFO, fmt, ap);
    va_end(ap);
}

void log_trace(log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_TRACE, fmt, ap);
    va_end(ap);
}

void log_warn(log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_WARN, fmt, ap);
    va_end(ap);
}

static void log_message_v(log_category_t category, SDL_LogPriority priority, const char *fmt, va_list ap)
{
    int32_t sdl_category = SDL_LOG_CATEGORY_CUSTOM + category;
    if (g_started) {
        SDL_LogMessageV(sdl_category, priority, fmt, ap);
    } else {
        stash_log_entry(sdl_category, priority, fmt, ap);
    }
}

static void stash_log_entry(int32_t category, SDL_LogPriority priority, const char *fmt, va_list ap)
{
    heap_allocator_t *heap = mem_system_allocator();
    log_entry_t *entry = heap_alloc(heap, sizeof(log_entry_t), MEM_DEFAULT_ALIGN);
    // todo: check entry has been allocated.
    entry->category = category;
    entry->priority = priority;
    entry->message = heap_alloc(heap, 1024, MEM_DEFAULT_ALIGN); // todo: heap_calloc and no need to assign NULL to next
    entry->next = NULL;
    // todo: check message has been allocated.
    vsnprintf(entry->message, 1024, fmt, ap);
    // todo: check we fit in the buffer and realloc if too small.

    if (g_entries_head == NULL) {
        g_entries_head = entry;
    } else {
        log_entry_t *current = g_entries_head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = entry;
    }
}

static void process_log_entry(void)
{
    log_entry_t *current = g_entries_head;
    g_entries_head = current->next;

    SDL_LogMessage(current->category, current->priority, current->message);

    heap_allocator_t *heap = mem_system_allocator();
    heap_dealloc(heap, current->message);
    heap_dealloc(heap, current);
}
