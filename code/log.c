#include "log.h"

#include <SDL3/SDL.h>
#include <assert.h>
#include <stdbool.h>

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
    SDL_SetLogPriority(LOG_CATEGORY_GPU + SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_TRACE);
    SDL_SetLogPriority(LOG_CATEGORY_MEMORY + SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_TRACE);
    SDL_SetLogPriority(LOG_CATEGORY_WINDOW + SDL_LOG_CATEGORY_CUSTOM, SDL_LOG_PRIORITY_TRACE);

    if (g_entries_head != NULL) {
        while (g_entries_head != NULL) {
            process_log_entry();
        }
    }

    g_started = true;
}

void log_debug(const log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_DEBUG, fmt, ap);
    va_end(ap);
}

void log_error(const log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_ERROR, fmt, ap);
    va_end(ap);
}

void log_info(const log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_INFO, fmt, ap);
    va_end(ap);
}

void log_trace(const log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_TRACE, fmt, ap);
    va_end(ap);
}

void log_warn(const log_category_t category, const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    log_message_v(category, SDL_LOG_PRIORITY_WARN, fmt, ap);
    va_end(ap);
}

static void log_message_v(const log_category_t category, const SDL_LogPriority priority, const char *fmt, va_list ap)
{
    const int32_t sdl_category = SDL_LOG_CATEGORY_CUSTOM + category;
    if (g_started) {
        SDL_LogMessageV(sdl_category, priority, fmt, ap);
    } else {
        stash_log_entry(sdl_category, priority, fmt, ap);
    }
}

static void stash_log_entry(const int32_t category, const SDL_LogPriority priority, const char *fmt, va_list ap)
{
    heap_allocator_t *heap = mem_system_allocator();

    log_entry_t *entry = heap_calloc(heap, 1, sizeof(log_entry_t), MEM_DEFAULT_ALIGN);
    if (entry == NULL) {
        return;
    }

    entry->category = category;
    entry->priority = priority;

    const int32_t len = SDL_vsnprintf(NULL, 0, fmt, ap);
    size_t len_plus_term;
    if (!SDL_size_add_check_overflow(len, 1, &len_plus_term)) {
        heap_dealloc(heap, entry);
        return;
    }
    entry->message = heap_alloc(heap, len_plus_term, MEM_DEFAULT_ALIGN);
    if (entry->message == NULL) {
        heap_dealloc(heap, entry);
        return;
    }

    const int32_t written = SDL_vsnprintf(entry->message, len_plus_term, fmt, ap);
    if (written < 0) {
        heap_dealloc(heap, entry->message);
        heap_dealloc(heap, entry);
        return;
    }

    assert(len == written);

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
