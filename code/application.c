#include "application.h"

#include <SDL3/SDL.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "error.h"
#include "log.h"
#include "memory.h"

void start_application(void)
{
    if (!start_memory_system((memory_system_desc_t){
            .system_memory_size = GB(1),
            .scratch_memory_size = MB(5),
        })) {
        log_error(LOG_CATEGORY_APPLICATION, "Failed to initialize memory system.");
        exit_application(APPLICATION_INITIALIZATION_ERROR);
    }

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        log_error(LOG_CATEGORY_APPLICATION, "Failed to initialize SDL: %s", SDL_GetError());
        exit_application(APPLICATION_INITIALIZATION_ERROR);
    }

    start_log_system();

    log_info(LOG_CATEGORY_APPLICATION, "Started application.");

#if FEATURE_MEMORY_STATS
    log_debug(LOG_CATEGORY_MEMORY, "FEATURE_MEMORY_STATS: enabled.");
#elif
    log_debug(LOG_CATEGORY_MEMORY, "FEATURE_MEMORY_STATS: disabled.");
#endif
}

void stop_application(void)
{
    log_info(LOG_CATEGORY_APPLICATION, "Shutdown application.");

    stop_memory_system();
}

void exit_application(const int32_t exit_code)
{
    log_error(LOG_CATEGORY_APPLICATION, "Exited application.");
    exit(exit_code);
}
