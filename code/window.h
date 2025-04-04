#ifndef WINDOW_H
#define WINDOW_H

#include <stdbool.h>
#include <stdint.h>

void create_window(const char *title, int32_t width, int32_t height);

void destroy_window(void);

bool run_window_event_loop(void);

bool close_window_requested(void);

void exit_window_event_loop(void);

void *window_handle(void);

bool window_was_resized(void);

void get_window_size(int32_t *width, int32_t *height);

#endif // WINDOW_H
