#include "window.h"

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "application.h"
#include "error.h"
#include "log.h"

static SDL_Window *window;
static bool was_close_requested;
static bool keep_running;

void create_window(const char *title, const int32_t width, const int32_t height)
{
    window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (window == 0) {
        log_error("Failed to create window: %s", SDL_GetError());
        exit_application(WINDOW_CREATION_ERROR);
    }

    was_close_requested = false;
    keep_running = true;

    log_info("Created window with title=%s, width=%d, height=%d", title, width, height);
}

void destroy_window(void)
{
    SDL_DestroyWindow(window);
    log_info("Destroyed window");
}

bool run_window_event_loop(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            was_close_requested = true;
        }
    }

    return keep_running;
}

bool close_window_requested(void)
{
    return was_close_requested;
}

void exit_window_event_loop(void)
{
    keep_running = false;
}
