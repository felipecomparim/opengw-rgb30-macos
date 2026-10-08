#pragma once
#include <unistd.h>

inline SDL_Event scoreKey(SDL_Keycode code) {
    SDL_Event event = {};
    event.type = SDL_KEYDOWN; event.key.keysym.sym = code;
    return event;
}
inline bool highscoreSelfTest() {
    char path[] = "/tmp/opengw-scores-XXXXXX";
    const int fd = mkstemp(path);
    if (fd < 0) return false;
    FILE* fixture = fdopen(fd, "w");
    std::fprintf(fixture, "OPENGW_HIGHSCORES_1\n");
    for (int i = 0; i < 10; ++i) std::fprintf(fixture, "PLAYER%d\t%d\n", i, 1000 - i * 100);
    std::fclose(fixture);
    const int previousScore = game::mPlayers.mPlayer1->mScore;
    highscore table(path);
    bool ok = table.isHighScore(1500) && !table.isHighScore(50) && !table.isHighScore(0);
    game::mPlayers.mPlayer1->mScore = 1500;
    table.init();
    SDL_Event text = {}; text.type = SDL_TEXTINPUT;
    std::strcpy(text.text.text, "felipe m4"); table.handleEvent(text);
    table.handleEvent(scoreKey(SDLK_BACKSPACE));
    std::strcpy(text.text.text, "4"); table.handleEvent(text);
    table.handleEvent(scoreKey(SDLK_RETURN));
    highscore reloaded(path);
    ok = ok && reloaded.scoreAt(0) == 1500 && !std::strcmp(reloaded.nameAt(0), "FELIPE M4") &&
        reloaded.scoreAt(1) == 1000 && reloaded.scoreAt(9) == 200 && game::mGameMode == game::GAMEMODE_SCORETABLE;
    game::mPlayers.mPlayer1->mScore = 1200;
    reloaded.init();
    std::strcpy(text.text.text, "SECOND"); reloaded.handleEvent(text);
    reloaded.handleEvent(scoreKey(SDLK_RETURN));
    highscore again(path);
    ok = ok && again.scoreAt(0) == 1500 && again.scoreAt(1) == 1200 && !std::strcmp(again.nameAt(1), "SECOND");
    // Saving must fail visibly and retain the pending score if the folder is missing.
    highscore unavailable((std::string(path) + "/missing").c_str());
    unavailable.init();
    game::mGameMode = game::GAMEMODE_HIGHSCORES;
    unavailable.handleEvent(scoreKey(SDLK_RETURN));
    ok = ok && game::mGameMode == game::GAMEMODE_HIGHSCORES && unavailable.scoreAt(0) == 0;
    unavailable.handleEvent(scoreKey(SDLK_ESCAPE));
    ok = ok && game::mGameMode == game::GAMEMODE_MAINMENU;
    mainMenu::handleEvent(scoreKey(SDLK_DOWN));
    mainMenu::handleEvent(scoreKey(SDLK_RETURN));
    ok = ok && game::mGameMode == game::GAMEMODE_SCORETABLE;
    mainMenu::handleEvent(scoreKey(SDLK_ESCAPE));
    mainMenu::handleEvent(scoreKey(SDLK_UP));
    mainMenu::handleEvent(scoreKey(SDLK_RETURN));
    ok = ok && game::mGameMode == game::GAMEMODE_CHOOSE_GAMETYPE;
    mainMenu::handleEvent(scoreKey(SDLK_RETURN));
    ok = ok && game::mGameMode == game::GAMEMODE_PLAYING;
    // Verify the real game-over state machine routes both score outcomes.
    game::mPlayers.mPlayer1->mScore = 0;
    game::mGameMode = game::GAMEMODE_GAMEOVER_TRANSITION;
    for (int i = 0; i < 183; ++i) theGame.run();
    ok = ok && game::mGameMode == game::GAMEMODE_SCORETABLE;
    game::mPlayers.mPlayer1->mScore = 2147483647;
    game::mGameMode = game::GAMEMODE_GAMEOVER_TRANSITION;
    for (int i = 0; i < 183; ++i) theGame.run();
    ok = ok && game::mGameMode == (game::mHighscore.isHighScore(2147483647) ?
        game::GAMEMODE_HIGHSCORES : game::GAMEMODE_SCORETABLE);
    SDL_StopTextInput();
    game::mPlayers.mPlayer1->mScore = previousScore;
    std::remove(path);
    std::fprintf(stderr, "High-score entry/ranking/persistence/menu/game-over self-test %s\n", ok ? "passed" : "FAILED");
    return ok;
}
