#pragma once
#include <unistd.h>

// Exercise actual SDL axis mappings and the on-disk preference without a window.
inline bool controlsSelfTest() {
    const bool original = game::mSettings.mSwapSticks;
    const int device = SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER, 4, 4, 0);
    if (device < 0) return false;
    char guid[33], mapping[512];
    SDL_JoystickGetGUIDString(SDL_JoystickGetDeviceGUID(device), guid, sizeof(guid));
    std::snprintf(mapping, sizeof(mapping), "%s,OpenGW test,a:b0,b:b1,x:b2,y:b3,leftx:a0,lefty:a1,rightx:a2,righty:a3,", guid);
    SDL_GameControllerAddMapping(mapping);
    SDL_Joystick* joystick = SDL_JoystickOpen(device);
    if (!joystick) { SDL_JoystickDetachVirtual(device); return false; }
    SDL_JoystickSetVirtualAxis(joystick, 0, 32767);
    SDL_JoystickSetVirtualAxis(joystick, 3, -32767);
    SDL_JoystickUpdate();
    game::mSettings.mSwapSticks = false;
    const Point3d normalMove = game::mControls.getLeftStick(0);
    const Point3d normalAim = game::mControls.getRightStick(0);
    game::mSettings.mSwapSticks = true;
    const Point3d swappedMove = game::mControls.getLeftStick(0);
    const Point3d swappedAim = game::mControls.getRightStick(0);
    bool ok = normalMove.y < -.99f && normalAim.x > .99f &&
        swappedMove.x > .99f && swappedAim.y < -.99f;
    // Face buttons still fire with either stick layout.
    SDL_JoystickSetVirtualAxis(joystick, 0, 0);
    SDL_JoystickSetVirtualAxis(joystick, 3, 0);
    SDL_JoystickSetVirtualButton(joystick, 2, 1);
    SDL_JoystickUpdate();
    ok = ok && game::mControls.getRightStick(0).y > .99f;
    char path[] = "/tmp/opengw-controls-XXXXXX";
    const int fd = mkstemp(path);
    if (fd < 0) ok = false;
    else {
        close(fd);
        ok = game::mSettings.saveControls(path) && ok;
        game::mSettings.mSwapSticks = false;
        game::mSettings.loadControls(path);
        ok = game::mSettings.mSwapSticks && ok;
        game::mSettings.mSwapSticks = false;
        ok = game::mSettings.saveControls(path) && ok;
        game::mSettings.mSwapSticks = true;
        game::mSettings.loadControls(path);
        ok = !game::mSettings.mSwapSticks && ok;
        std::remove(path);
    }
    game::mSettings.mSwapSticks = original;
    SDL_JoystickClose(joystick);
    SDL_JoystickDetachVirtual(device);
    game::mControls.getLeftStick(0); // Close the detached controller handle.
    std::fprintf(stderr, "Stick layout/persistence self-test %s\n", ok ? "passed" : "FAILED");
    return ok;
}
