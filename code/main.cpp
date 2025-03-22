import platform.window;

#include <sdl3/sdl.h>

int main() {
    greet();

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Failed to initialize SDL: %s", SDL_GetError());
    }

    SDL_Log("Welcome to Bodies!");

    SDL_Window *window = SDL_CreateWindow("Bodies", 1920, 1080, SDL_WINDOW_RESIZABLE);
    if (window == nullptr) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return -1;
    }

    bool quit = false;
    while (!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                quit = true;
            }
        }
    }

    return 0;
}
