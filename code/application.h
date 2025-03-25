#ifndef APPLICATION_H
#define APPLICATION_H

#include <stdbool.h>
#include <stdint.h>

void init(void);

void create_window(const char *title, int32_t width, int32_t height);

bool run_event_loop(void);

void exit_application(int32_t exit_code);

#endif // APPLICATION_H
