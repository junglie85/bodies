#include "application.h"
#include "window.h"

int main(void)
{
    init_application();
    create_window("Bodies", 1920, 1080);

    while (run_window_event_loop()) {
        if (close_window_requested()) {
            exit_window_event_loop();
        }
    }

    return 0;
}
