#pragma once
#include <string>
#include <cstdlib>
#ifdef __APPLE__
#include <SDL.h>
#endif

inline std::string storagePath(const char* name) {
    const char* overridePath = std::getenv("OPENGW_SAVE_DIR");
    if (overridePath && *overridePath) return std::string(overridePath) + "/" + name;
#ifdef __APPLE__
    char* directory = SDL_GetPrefPath("OpenGW", "OpenGW");
    if (directory) {
        const std::string path = std::string(directory) + name;
        SDL_free(directory);
        return path;
    }
#endif
    return name;
}
