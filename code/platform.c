#include "platform.h"

#include <SDL3/SDL.h>
#include <stdbool.h>

bool init_platform()
{
    return SDL_Init(SDL_INIT_VIDEO);
}
