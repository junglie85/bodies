// module;
//
// #include <SDL3/SDL.h>
//
// export module core.log;
//
// import std;
//
// export template<typename... Args>
// void log_info(std::string_view fmt, Args &&... args) {
//     const auto s = std::vformat(fmt, std::make_format_args(args...));
//     SDL_Log(s.c_str());
// }
