module;

#include <SDL3/SDL.h>

module platform.window;

// todo: private?
SDL_Window *window = nullptr;

bool create_window(const std::string &title, std::int32_t width, std::int32_t height) {
    window = SDL_CreateWindow(title.c_str(), width, height, SDL_WINDOW_RESIZABLE);
    return window != nullptr;
}

void poll_window_events() {
    window_events.clear();

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            window_events.emplace_back(WindowEvent{.type = EventType::QUIT});
        }
    }
}
