#include "application.h"

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "error.h"
#include "log.h"

void startup_application(void)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_error("Failed to initialize SDL: %s", SDL_GetError());
        exit_application(APPLICATION_INITIALISATION_ERROR);
    }

    log_info("Started application");
}

void shutdown_application(void)
{
    log_info("Shutdown application");
}

void exit_application(const int32_t exit_code)
{
    exit(exit_code);
}
