#include "platform.window.h"

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#define WINDOW_EVENT_SIZE 20

static SDL_Window *window;
static WindowEvent *window_events;
static int32_t window_event_count;

bool create_window(const char *title, int32_t width, int32_t height)
{
    window = SDL_CreateWindow(title, width, height, SDL_WINDOW_RESIZABLE);
    bool success = window != 0;
    if (success) {
        window_events = SDL_malloc(sizeof(WindowEvent) * WINDOW_EVENT_SIZE);
    }

    return success;
}

int32_t poll_window_events()
{
    window_event_count = 0;

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            WindowEvent *e = window_events + window_event_count;
            *e = (WindowEvent){ .type = WINDOW_EVENT_TYPE_QUIT };
            ++window_event_count;
        }
    }

    return window_event_count;
}

WindowEvent *get_window_event(int32_t id)
{
    return &window_events[id];
}
