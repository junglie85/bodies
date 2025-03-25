#include "application.h"

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "error.h"
#include "log.h"

static SDL_Window *window;

void init(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_error("Failed to initialize SDL: %s", SDL_GetError());
        exit_application(APPLICATION_INITIALISATION_ERROR);
    }

    log_info("Initialised application");
}

void create_window(const char *title, const int32_t width, const int32_t height)
{
    window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    if (window == 0) {
        log_error("Failed to create window: %s", SDL_GetError());
        exit_application(WINDOW_CREATION_ERROR);
    }

    log_info("Created window with title=%s, width=%d, height=%d", title, width, height);
}

bool run_event_loop(void)
{
    bool keep_running = true;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            keep_running = false;
        }
    }

    return keep_running;
}

void exit_application(const int32_t exit_code)
{
    exit(exit_code);
}
