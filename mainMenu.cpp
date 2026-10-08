#include "mainMenu.h"
#include "game.h"
#include "menuSelectGameType.h"
#include "font.h"
#include <algorithm>
namespace {
int selection = 0, pauseSelection = 0, controlsSelection = 0;
bool controlsPage = false, saveFailed = false;
int verticalAxis = 0, horizontalAxis = 0;
void commit(const settings& previous) {
    saveFailed = !game::mSettings.saveControls();
    if (saveFailed) game::mSettings = previous;
    sound::setMasterVolumes(game::mSettings.mMusicVolume, game::mSettings.mSoundVolume);
}
void clickSound() {
    game::mSound.unpauseTrack(SOUNDID_MENU_SELECT);
    game::mSound.playTrack(SOUNDID_MENU_SELECT);
}
}
bool mainMenu::isPaused() { return game::mGameMode == game::GAMEMODE_PLAYING && game::mPaused; }
void mainMenu::setPaused(bool paused) {
    game::mPaused = paused;
    controlsPage = false;
    saveFailed = false;
    pauseSelection = 0;
    verticalAxis = horizontalAxis = 0;
    if (paused) game::mSound.pauseAllTracksBut(SOUNDID_MUSICLOOP);
    else game::mSound.unpauseAllTracks();
    clickSound();
}
void mainMenu::open() {
    if (game::mGameMode == game::GAMEMODE_PLAYING) {
        theGame.endGame();
        game::mSound.stopAllTracks();
    }
    game::mSound.unpauseAllTracks();
    SDL_StopTextInput();
    game::mGameMode = game::GAMEMODE_MAINMENU;
    game::mPaused = false;
    controlsPage = false;
    saveFailed = false;
    verticalAxis = horizontalAxis = 0;
    game::mCamera.mCurrentZoom = 1;
    game::mSound.playTrack(SOUNDID_MENU_MUSICLOOP);
}
bool mainMenu::handleEvent(const SDL_Event& event) {
    const bool key = event.type == SDL_KEYDOWN && !event.key.repeat;
    const bool button = event.type == SDL_CONTROLLERBUTTONDOWN;
#ifdef __APPLE__
    if (key && event.key.keysym.sym == SDLK_q && (event.key.keysym.mod & KMOD_GUI)) return true;
#endif
    const bool back = (key && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_BACKSPACE)) ||
        (button && event.cbutton.button == SDL_CONTROLLER_BUTTON_B);
    const bool confirm = (key && event.key.keysym.sym == SDLK_RETURN) ||
        (button && event.cbutton.button == SDL_CONTROLLER_BUTTON_A);
    if (game::mGameMode == game::GAMEMODE_SCORETABLE) {
        if (back || confirm) open();
        return false;
    }
    if (game::mGameMode == game::GAMEMODE_HIGHSCORES) {
        game::mHighscore.handleEvent(event);
        return false;
    }
    if (game::mGameMode == game::GAMEMODE_CHOOSE_GAMETYPE) {
        if (back) open();
        else if ((key && event.key.keysym.sym == SDLK_UP) ||
            (button && event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)) menuSelectGameType::selection = 0;
        else if ((key && event.key.keysym.sym == SDLK_DOWN) ||
            (button && event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)) menuSelectGameType::selection = 1;
        else if (confirm && menuSelectGameType::selection == 0) {
            theGame.startGame(game::GAMETYPE_SINGLEPLAYER);
            clickSound();
        } else if (menuSelectGameType::selection == 1 && key && event.key.keysym.sym == SDLK_SPACE) {
            theGame.startGame(game::GAMETYPE_MULTIPLAYER_COOP);
            clickSound();
        }
        return false;
    }
    if (game::mGameMode == game::GAMEMODE_PLAYING) {
        const bool pauseKey = (key && (event.key.keysym.sym == SDLK_p || event.key.keysym.sym == SDLK_ESCAPE)) ||
            (button && event.cbutton.button == SDL_CONTROLLER_BUTTON_START);
        if (pauseKey) {
            if (isPaused() && controlsPage && key && event.key.keysym.sym == SDLK_ESCAPE) controlsPage = false;
            else setPaused(!game::mPaused);
            return false;
        }
        if (!isPaused()) return false;
    } else if (game::mGameMode != game::GAMEMODE_MAINMENU) return false;
    int direction = 0, adjustment = 0;
    if (key) {
        if (event.key.keysym.sym == SDLK_UP) direction = -1;
        if (event.key.keysym.sym == SDLK_DOWN) direction = 1;
        if (event.key.keysym.sym == SDLK_LEFT) adjustment = -1;
        if (event.key.keysym.sym == SDLK_RIGHT) adjustment = 1;
    }
    if (button) {
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) direction = -1;
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) direction = 1;
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) adjustment = -1;
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) adjustment = 1;
    }
    if (event.type == SDL_CONTROLLERAXISMOTION && event.caxis.axis <= SDL_CONTROLLER_AXIS_LEFTY) {
        const int value = event.caxis.value > 18000 ? 1 : event.caxis.value < -18000 ? -1 : 0;
        int& previous = event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY ? verticalAxis : horizontalAxis;
        if (value && value != previous) {
            if (event.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY) direction = value; else adjustment = value;
        }
        previous = value;
    }
    if (controlsPage) {
        if (back) { controlsPage = false; return false; }
        if (direction) controlsSelection = (controlsSelection + direction + 3) % 3;
        const bool swapShortcut = (key && event.key.keysym.sym == SDLK_TAB) ||
            (button && event.cbutton.button == SDL_CONTROLLER_BUTTON_X);
        if (swapShortcut || adjustment || confirm) {
            const int row = swapShortcut ? 1 : controlsSelection;
            if (row == 2) controlsPage = false;
            else {
                const settings previous = game::mSettings;
                if (row == 0) game::mSettings.mSwapKeyboard = !game::mSettings.mSwapKeyboard;
                else game::mSettings.mSwapSticks = !game::mSettings.mSwapSticks;
                commit(previous);
                clickSound();
            }
        }
        return false;
    }
    if (back && isPaused()) { setPaused(false); return false; }
    // Escape never quits the application from a menu.
    if (back) return false;
    int& selected = isPaused() ? pauseSelection : selection;
    const int count = isPaused() ? 5 : 4;
    if (direction) { selected = (selected + direction + count) % count; clickSound(); }
    if (isPaused()) {
        if ((selected == 2 || selected == 3) && (adjustment || confirm)) {
            const settings previous = game::mSettings;
            int& volume = selected == 2 ? game::mSettings.mMusicVolume : game::mSettings.mSoundVolume;
            volume = std::max(0, std::min(100, volume + (adjustment ? adjustment : 1) * 10));
            commit(previous); clickSound();
        } else if (confirm) {
            if (selected == 0) setPaused(false);
            if (selected == 1) { controlsPage = true; controlsSelection = 0; }
            if (selected == 4) open();
        }
        return false;
    }
    if (confirm) {
        switch (selection) {
        case 0: menuSelectGameType::init(0); game::mGameMode = game::GAMEMODE_CHOOSE_GAMETYPE; break;
        case 1: game::mGameMode = game::GAMEMODE_SCORETABLE; break;
        case 2: controlsPage = true; controlsSelection = 0; break;
        case 3: return true;
        }
    }
    return false;
}
void mainMenu::draw() {
    vector::pen pen(0, 1, .5, 1, 2);
    font::AlphanumericsPrint(font::ALIGN_CENTER, .035, 0, .65, &pen,
        isPaused() ? "Paused" : "Open Geometry Wars");
    if (controlsPage) {
        font::AlphanumericsPrint(font::ALIGN_CENTER, .025, 0, .4, &pen, "Controls");
        for (int i = 0; i < 3; ++i) {
            pen = i == controlsSelection ? vector::pen(1, 1, 1, 1, 3) : vector::pen(0, 1, .5, .8, 2);
            const char* text = "Back";
#ifdef __APPLE__
            const bool arrows = !game::mSettings.mSwapKeyboard;
#else
            const bool arrows = game::mSettings.mSwapKeyboard;
#endif
            if (i == 0) text = arrows ? "Keyboard: Arrows Move - WASD Aim" : "Keyboard: WASD Move - Arrows Aim";
            if (i == 1) text = game::mSettings.mSwapSticks ? "Gamepad: Move Right - Aim Left" : "Gamepad: Move Left - Aim Right";
            font::AlphanumericsPrint(font::ALIGN_CENTER, .018, 0, .15 - i * .22, &pen,
                i == controlsSelection ? "> %s <" : "%s", text);
        }
        pen = vector::pen(1, 1, 1, .8, 2);
        font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.6, &pen, "Up / Down: Select - Enter / A: Change");
        font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.75, &pen,
            saveFailed ? "Could Not Save Settings" : "Esc / B: Back - Tab / X: Swap Gamepad");
        return;
    }
    if (isPaused()) {
        const char* items[] = {"Resume", "Controls", "Music Volume", "Sound Volume", "Quit To Main Menu"};
        for (int i = 0; i < 5; ++i) {
            pen = i == pauseSelection ? vector::pen(1, 1, 1, 1, 3) : vector::pen(0, 1, .5, .8, 2);
            const float y = .32f - i * .18f;
            if (i == 2 || i == 3)
                font::AlphanumericsPrint(font::ALIGN_CENTER, .023, 0, y, &pen,
                    i == pauseSelection ? "> %s: %d%% <" : "%s: %d%%", items[i],
                    i == 2 ? game::mSettings.mMusicVolume : game::mSettings.mSoundVolume);
            else font::AlphanumericsPrint(font::ALIGN_CENTER, .023, 0, y, &pen,
                i == pauseSelection ? "> %s <" : "%s", items[i]);
        }
        pen = vector::pen(1, 1, 1, .8, 2);
        font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.65, &pen, "Left / Right: Volume - Esc: Resume");
        font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.75, &pen,
            saveFailed ? "Could Not Save Settings" : "Enter / A: Confirm - Cmd Q: Quit");
        return;
    }
    const char* items[] = {"New Game", "High Scores", "Controls", "Quit"};
    for (int i = 0; i < 4; ++i) {
        pen = i == selection ? vector::pen(1, 1, 1, 1, 3) : vector::pen(0, 1, .5, .7, 2);
        font::AlphanumericsPrint(font::ALIGN_CENTER, .028, 0, .25 - i * .2, &pen,
            i == selection ? "> %s <" : "%s", items[i]);
    }
    pen = vector::pen(1, 1, 1, .8, 2);
    font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.75, &pen, "Up / Down: Select - Enter / A: Confirm");
}
