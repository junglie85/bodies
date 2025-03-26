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

void create_window(const char *title, const int32_t width, const int32_t height)
{
    g_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (g_window == NULL) {
        log_error(LOG_CATEGORY_WINDOW, "Failed to create window: %s.", SDL_GetError());
        exit_application(WINDOW_CREATION_ERROR);
    }

    g_was_close_requested = false;
    g_keep_running = true;

    log_info(LOG_CATEGORY_WINDOW, "Created window with title=%s, width=%d, height=%d.", title, width, height);
}

void destroy_window(void)
{
    SDL_DestroyWindow(g_window);
    log_info(LOG_CATEGORY_WINDOW, "Destroyed window.");
}

bool run_window_event_loop(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            g_was_close_requested = true;
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
