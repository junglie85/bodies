export module platform.window;

import std;

export enum class EventType {
    QUIT,
};

export struct WindowEvent {
    EventType type;
};

export bool create_window(const std::string &title, std::int32_t width, std::int32_t height);

export void poll_window_events();

export std::vector<WindowEvent> window_events;
