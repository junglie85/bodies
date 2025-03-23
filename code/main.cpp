import platform.window;
import platform;

#include <SDL3/SDL.h>

int main() {
    if (!init_platform()) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    }

    if (!create_window("Bodies", 1920, 1080)) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
    }

    bool quit = false;
    while (!quit) {
        poll_window_events();

        for (const auto &event: window_events) {
            switch (event.type) {
                case EventType::QUIT: {
                    quit = true;
                    break;
                }
            }
        }
    }

    return 0;
}
