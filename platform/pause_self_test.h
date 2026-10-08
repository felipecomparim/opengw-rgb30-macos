#pragma once
#include <unistd.h>

inline bool pauseSelfTest() {
    char directory[] = "/tmp/opengw-pause-XXXXXX";
    if (!mkdtemp(directory)) return false;
    const char* previousEnvironment = std::getenv("OPENGW_SAVE_DIR");
    const bool hadEnvironment = previousEnvironment != nullptr;
    const std::string oldDirectory = previousEnvironment ? previousEnvironment : "";
    const settings previous = game::mSettings;
    setenv("OPENGW_SAVE_DIR", directory, 1);
    game::mSettings.mMusicVolume = game::mSettings.mSoundVolume = 100;
    sound::setMasterVolumes(100, 100);
    game::mGameMode = game::GAMEMODE_PLAYING;
    game::mPaused = false;
    bool ok = !mainMenu::handleEvent(scoreKey(SDLK_ESCAPE)) && mainMenu::isPaused();
    const Point3d position = game::mPlayers.mPlayer1->getPos();
    for (int i = 0; i < 60; ++i) theGame.run();
    const Point3d after = game::mPlayers.mPlayer1->getPos();
    ok = ok && position.x == after.x && position.y == after.y;
    mainMenu::handleEvent(scoreKey(SDLK_DOWN)); // Controls.
    mainMenu::handleEvent(scoreKey(SDLK_RETURN));
    mainMenu::handleEvent(scoreKey(SDLK_RETURN)); // Keyboard layout.
    ok = ok && game::mSettings.mSwapKeyboard != previous.mSwapKeyboard;
    mainMenu::handleEvent(scoreKey(SDLK_DOWN));
    mainMenu::handleEvent(scoreKey(SDLK_RETURN)); // Gamepad layout.
    ok = ok && game::mSettings.mSwapSticks != previous.mSwapSticks;
    mainMenu::handleEvent(scoreKey(SDLK_ESCAPE)); // Back to pause, not resume.
    ok = ok && mainMenu::isPaused();
    mainMenu::handleEvent(scoreKey(SDLK_DOWN)); // Music.
    mainMenu::handleEvent(scoreKey(SDLK_LEFT));
    mainMenu::handleEvent(scoreKey(SDLK_DOWN)); // Sound effects.
    mainMenu::handleEvent(scoreKey(SDLK_LEFT));
    mainMenu::handleEvent(scoreKey(SDLK_LEFT));
    ok = ok && game::mSettings.mMusicVolume == 90 && game::mSettings.mSoundVolume == 80;
    settings loaded;
    ok = ok && loaded.mMusicVolume == 90 && loaded.mSoundVolume == 80 &&
        loaded.mSwapSticks == game::mSettings.mSwapSticks && loaded.mSwapKeyboard == game::mSettings.mSwapKeyboard;
    sound::setMasterVolumes(0, 50);
    ok = ok && sound::masterGain(SOUNDID_MUSICLOOP) == 0 && sound::masterGain(SOUNDID_MENU_MUSICLOOP) == 0 &&
        sound::masterGain(SOUNDID_PLAYERFIRE1) == .5f;
    sound::setMasterVolumes(30, 0);
    ok = ok && sound::masterGain(SOUNDID_MUSICLOOP) > .29f && sound::masterGain(SOUNDID_PLAYERFIRE1) == 0;
    for (int i = 0; i < 20; ++i) mainMenu::handleEvent(scoreKey(SDLK_LEFT));
    ok = ok && game::mSettings.mSoundVolume == 0;
    for (int i = 0; i < 20; ++i) mainMenu::handleEvent(scoreKey(SDLK_RIGHT));
    ok = ok && game::mSettings.mSoundVolume == 100;
    mainMenu::handleEvent(scoreKey(SDLK_ESCAPE));
    ok = ok && !mainMenu::isPaused() && game::mGameMode == game::GAMEMODE_PLAYING;
    mainMenu::handleEvent(scoreKey(SDLK_ESCAPE));
    for (int i = 0; i < 4; ++i) mainMenu::handleEvent(scoreKey(SDLK_DOWN));
    mainMenu::handleEvent(scoreKey(SDLK_RETURN)); // Abandon game, not quit app.
    ok = ok && game::mGameMode == game::GAMEMODE_MAINMENU && !game::mPaused &&
        !mainMenu::handleEvent(scoreKey(SDLK_ESCAPE));
#ifdef __APPLE__
    SDL_Event quit = scoreKey(SDLK_q); quit.key.keysym.mod = KMOD_GUI;
    ok = ok && mainMenu::handleEvent(quit);
#endif
    game::mSettings = previous;
    sound::setMasterVolumes(previous.mMusicVolume, previous.mSoundVolume);
    if (hadEnvironment) setenv("OPENGW_SAVE_DIR", oldDirectory.c_str(), 1);
    else unsetenv("OPENGW_SAVE_DIR");
    std::remove((std::string(directory) + "/controls.cfg").c_str());
    rmdir(directory);
    std::fprintf(stderr, "Pause/control/audio-volume/persistence/quit self-test %s\n", ok ? "passed" : "FAILED");
    return ok;
}
