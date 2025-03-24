#ifndef PLATFORM_WINDOW_H
#define PLATFORM_WINDOW_H

#include <stdbool.h>
#include <stdint.h>

typedef enum WindowEventType WindowEventType;
enum WindowEventType
{
    WINDOW_EVENT_TYPE_QUIT,
};

typedef struct WindowEvent WindowEvent;
struct WindowEvent
{
    WindowEventType type;
};

bool create_window(const char *title, int32_t width, int32_t height);

int32_t poll_window_events();

WindowEvent *get_window_event(int32_t id);

#endif
