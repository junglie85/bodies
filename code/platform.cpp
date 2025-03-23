module;

#include <SDL3/SDL.h>

module platform;

bool init_platform() {
    return SDL_Init(SDL_INIT_VIDEO);
}
