#include "controls.h"
#include "game.h"
#include <cmath>
#include <cstdio>

namespace {
SDL_GameController* pads[4] = {};
bool key(SDL_Scancode code) { return SDL_GetKeyboardState(nullptr)[code] != 0; }
void refresh() {
    for (auto& pad : pads) {
        if (pad && !SDL_GameControllerGetAttached(pad)) {
            SDL_GameControllerClose(pad);
            pad = nullptr;
        }
    }
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (!SDL_IsGameController(i)) continue;
        if (SDL_GameControllerFromInstanceID(SDL_JoystickGetDeviceInstanceID(i))) continue;
        for (auto& pad : pads) {
            if (!pad) {
                pad = SDL_GameControllerOpen(i);
                if (pad) std::fprintf(stderr, "Controller: %s\n", SDL_GameControllerName(pad));
                break;
            }
        }
    }
}
bool button(int player, SDL_GameControllerButton which) {
    refresh();
    return player >= 0 && player < 4 && pads[player] && SDL_GameControllerGetButton(pads[player], which);
}
Point3d stick(int player, SDL_GameControllerAxis xAxis, SDL_GameControllerAxis yAxis) {
    refresh();
    if (player < 0 || player >= 4 || !pads[player]) return Point3d(0, 0, 0);
    // The game uses x=up/down and y=left/right.
    float x = -SDL_GameControllerGetAxis(pads[player], yAxis) / 32768.f;
    float y = -SDL_GameControllerGetAxis(pads[player], xAxis) / 32768.f;
    const float length = std::sqrt(x*x + y*y);
    if (length < .20f) return Point3d(0, 0, 0);
    const float scale = (std::fmin(length, 1.f) - .20f) / (.80f * length);
    return Point3d(x * scale, y * scale, 0);
}
Point3d normalize(Point3d p) {
    float length = std::sqrt(p.x*p.x + p.y*p.y);
    if (length > 1) { p.x /= length; p.y /= length; }
    return p;
}
}
controls::controls() : mNumJoysticks(0) { for (auto& p : mControllers) p = nullptr; }
controls::~controls() { for (auto& p : pads) if (p) SDL_GameControllerClose(p); }
Point3d controls::getLeftStick(int p) {
    if (!p && SDL_GetHintBoolean("OPENGW_BENCHMARK", SDL_FALSE)) {
        const float phase = SDL_GetTicks() * .001f;
        return Point3d(.65f * std::sin(phase), .65f * std::cos(phase), 0);
    }
    return normalize(readKeyboardLeftStick(p) + readXBoxControllerLeftStick(p));
}
Point3d controls::getRightStick(int p) {
    if (!p && SDL_GetHintBoolean("OPENGW_BENCHMARK", SDL_FALSE)) {
        const float phase = SDL_GetTicks() * .003f;
        return Point3d(std::sin(phase), std::cos(phase), 0);
    }
    return normalize(readKeyboardRightStick(p) + readXBoxControllerRightStick(p));
}
bool controls::getTriggerButton(int p) { return readKeyboardTrigger(p) || readXBoxControllerTrigger(p); }
bool controls::getStartButton(int p) { return readKeyboardStart(p) || readXBoxStart(p); }
bool controls::getBackButton(int p) { return readKeyboardBack(p) || readXBoxBack(p); }
bool controls::getPauseButton(int p) { return readKeyboardPause(p) || readXBoxPause(p); }
Point3d controls::readKeyboardLeftStick(int p) {
    if (p) return Point3d(0, 0, 0);
#ifdef __APPLE__
    const bool arrows = !game::mSettings.mSwapKeyboard;
#else
    const bool arrows = game::mSettings.mSwapKeyboard;
#endif
    if (arrows) return Point3d(key(SDL_SCANCODE_UP)-key(SDL_SCANCODE_DOWN), key(SDL_SCANCODE_LEFT)-key(SDL_SCANCODE_RIGHT), 0);
    return Point3d(key(SDL_SCANCODE_W)-key(SDL_SCANCODE_S), key(SDL_SCANCODE_A)-key(SDL_SCANCODE_D), 0);
}
Point3d controls::readKeyboardRightStick(int p) {
    if (p) return Point3d(0, 0, 0);
#ifdef __APPLE__
    const bool arrows = game::mSettings.mSwapKeyboard;
#else
    const bool arrows = !game::mSettings.mSwapKeyboard;
#endif
    if (arrows) return Point3d(key(SDL_SCANCODE_UP)-key(SDL_SCANCODE_DOWN), key(SDL_SCANCODE_LEFT)-key(SDL_SCANCODE_RIGHT), 0);
    return Point3d(key(SDL_SCANCODE_W)-key(SDL_SCANCODE_S), key(SDL_SCANCODE_A)-key(SDL_SCANCODE_D), 0);
}
bool controls::readKeyboardTrigger(int p) { return !p && key(SDL_SCANCODE_SPACE); }
bool controls::readKeyboardStart(int p) {
    return key(static_cast<SDL_Scancode>(SDL_SCANCODE_1 + p)) || (!p && key(SDL_SCANCODE_RETURN));
}
bool controls::readKeyboardBack(int p) { return !p && key(SDL_SCANCODE_BACKSPACE); }
bool controls::readKeyboardPause(int p) { return !p && key(SDL_SCANCODE_P); }
Point3d controls::readXBoxControllerLeftStick(int p) {
    const bool swapped = game::mSettings.mSwapSticks;
    Point3d v = stick(p, swapped ? SDL_CONTROLLER_AXIS_RIGHTX : SDL_CONTROLLER_AXIS_LEFTX,
        swapped ? SDL_CONTROLLER_AXIS_RIGHTY : SDL_CONTROLLER_AXIS_LEFTY);
    v.x += button(p, SDL_CONTROLLER_BUTTON_DPAD_UP)-button(p, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    v.y += button(p, SDL_CONTROLLER_BUTTON_DPAD_LEFT)-button(p, SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    return normalize(v);
}
Point3d controls::readXBoxControllerRightStick(int p) {
    const bool swapped = game::mSettings.mSwapSticks;
    Point3d v = stick(p, swapped ? SDL_CONTROLLER_AXIS_LEFTX : SDL_CONTROLLER_AXIS_RIGHTX,
        swapped ? SDL_CONTROLLER_AXIS_LEFTY : SDL_CONTROLLER_AXIS_RIGHTY);
    // Face buttons provide directional firing as an alternative to the stick.
    v.x += button(p, SDL_CONTROLLER_BUTTON_Y)-button(p, SDL_CONTROLLER_BUTTON_A);
    v.y += button(p, SDL_CONTROLLER_BUTTON_X)-button(p, SDL_CONTROLLER_BUTTON_B);
    return normalize(v);
}
bool controls::readXBoxControllerTrigger(int p) {
    refresh();
    return button(p, SDL_CONTROLLER_BUTTON_LEFTSHOULDER) || button(p, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER) ||
        (p >= 0 && p < 4 && pads[p] && (SDL_GameControllerGetAxis(pads[p], SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 12000 ||
        SDL_GameControllerGetAxis(pads[p], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 12000));
}
bool controls::readXBoxStart(int p) { return button(p, SDL_CONTROLLER_BUTTON_A); }
bool controls::readXBoxBack(int p) { return button(p, SDL_CONTROLLER_BUTTON_B); }
bool controls::readXBoxPause(int p) { return button(p, SDL_CONTROLLER_BUTTON_START); }
