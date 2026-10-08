#pragma once
#include "defines.h"

class mainMenu {
public:
    static void open();
    static bool handleEvent(const SDL_Event& event);
    static void draw();
    static void setPaused(bool paused);
    static bool isPaused();
};
