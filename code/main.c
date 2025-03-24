#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>

#include "platform.h"
#include "platform.window.h"

int main()
{
    SDL_Log("Hello!");

    if (!init_platform()) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    }

    if (!create_window("Bodies", 1920, 1080)) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
    }

    bool quit = false;
    while (!quit) {
        int32_t event_count = poll_window_events();

        for (int32_t i = 0; i < event_count; ++i) {
            WindowEvent *event = get_window_event(i);

            switch (event->type) {
            case WINDOW_EVENT_TYPE_QUIT:
            {
                quit = true;
                break;
            }
            }
        }
    }

    return 0;
}
