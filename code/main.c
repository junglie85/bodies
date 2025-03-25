#include "application.h"

int main(void)
{
    init();
    create_window("Bodies", 1920, 1080);

    while (run_event_loop()) {
    }

    return 0;
}
