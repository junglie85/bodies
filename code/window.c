#include "window.h"

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "application.h"
#include "error.h"
#include "log.h"

static SDL_Window *g_window;
static bool g_was_close_requested;
static bool g_keep_running;
static bool g_size_changed;
static int32_t g_window_width;
static int32_t g_window_height;

void create_window(const char *title, const int32_t width, const int32_t height)
{
    g_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (g_window == NULL) {
        log_error(LOG_CATEGORY_WINDOW, "Failed to create window: %s.", SDL_GetError());
        exit_application(WINDOW_CREATION_ERROR);
    }

    g_was_close_requested = false;
    g_keep_running = true;
    g_size_changed = false;

    SDL_GetWindowSize(g_window, &g_window_width, &g_window_height);

    log_info(LOG_CATEGORY_WINDOW, "Created window with title=%s, width=%d, height=%d.", title, width, height);
}

void destroy_window(void)
{
    SDL_DestroyWindow(g_window);
    log_info(LOG_CATEGORY_WINDOW, "Destroyed window.");
}

bool run_window_event_loop(void)
{
    g_size_changed = false;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
            g_was_close_requested = true;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
        {
            const int32_t new_width = event.window.data1;
            const int32_t new_height = event.window.data2;

            if (new_width != g_window_width || new_height != g_window_height) {
                g_size_changed = true;
                g_window_width = new_width;
                g_window_height = new_height;

                log_info(LOG_CATEGORY_WINDOW, "Window resized, width=%d, height=%d.", g_window_width, g_window_height);
            }
        } break;

        default:
            break;
        }
    }

    return g_keep_running;
}

bool close_window_requested(void)
{
    return g_was_close_requested;
}

void exit_window_event_loop(void)
{
    g_keep_running = false;
}

void *window_handle(void)
{
    return g_window;
}

bool window_was_resized(void)
{
    return g_size_changed;
}

void get_window_size(int32_t *width, int32_t *height)
{
    *width = g_window_width;
    *height = g_window_height;
}
