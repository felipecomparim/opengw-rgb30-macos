#include "game.h"
#include "menuSelectGameType.h"
#include "mainMenu.h"
#include "sincos.h"
#include <unistd.h>
#include <ctime>
#include <cstring>
#include <csignal>
#include "profile.h"
namespace profile {
Uint64 elapsed[COUNT] = {};
Uint64 calls[COUNT] = {};
}

// Keep the original static game services in one translation unit, after SDL
// startup and before the scene/game constructors that depend on them.
namespace {
volatile std::sig_atomic_t quitRequested = 0;
void requestQuit(int) { quitRequested = 1; }
struct Runtime {
    Runtime() {
        const char* data = std::getenv("OPENGW_DATA_DIR");
        char* base = SDL_GetBasePath();
        if (chdir(data ? data : (base ? base : ".")) != 0) {
            std::perror("OpenGW data directory");
            std::exit(1);
        }
        SDL_free(base);
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) != 0) {
            std::fprintf(stderr, "SDL_Init: %s\n", SDL_GetError());
            std::exit(1);
        }
        const char* mapping = std::getenv("SDL_GAMECONTROLLERCONFIG_FILE");
        if (mapping) SDL_GameControllerAddMappingsFromFile(mapping);
        SDL_GameControllerAddMappingsFromFile("gamecontrollerdb.txt");
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        make_sin_cos_tables();
    }
    ~Runtime() { SDL_Quit(); }
} runtime;
}
#include "game_objects.inc"
scene oglScene;
game theGame;
#include "controls_self_test.h"
#include "highscore_self_test.h"
#include "pause_self_test.h"

int main(int argc, char** argv) {
    std::signal(SIGTERM, requestQuit);
    std::signal(SIGINT, requestQuit);
#ifdef __APPLE__
    bool windowed = true;
#else
    bool windowed = false;
#endif
    int frameLimit = 0;
    bool selfTest = false;
    bool startGame = false;
    bool benchmark = false;
    bool showMenu = false;
    const char* screenshot = nullptr;
    for (int i = 1; i < argc; ++i) {
        if (!std::strcmp(argv[i], "--windowed")) windowed = true;
        else if (!std::strcmp(argv[i], "--fullscreen")) windowed = false;
        else if (!std::strcmp(argv[i], "--self-test")) selfTest = true;
        else if (!std::strcmp(argv[i], "--start-game")) startGame = true;
        else if (!std::strcmp(argv[i], "--benchmark")) benchmark = startGame = true;
        else if (!std::strcmp(argv[i], "--menu")) showMenu = true;
        else if (!std::strcmp(argv[i], "--screenshot") && i + 1 < argc) screenshot = argv[++i];
        else if (!std::strcmp(argv[i], "--frames") && i + 1 < argc) frameLimit = std::atoi(argv[++i]);
        else {
            std::fprintf(stderr, "Usage: opengw [--windowed|--fullscreen] [--frames N] [--self-test] [--start-game] [--menu] [--benchmark] [--screenshot file.bmp]\n");
            return 1;
        }
    }
    // RGB30 defaults: a single render pass avoids the desktop CPU blur/readback.
    game::mSettings.mEnableGlow = false;
    game::mSettings.mPlayerSmoothing = false;
    game::mSettings.mStarSmoothing = false;
    if (selfTest) {
        if (!controlsSelfTest()) return 4;
        SDL_SetHint("OPENGW_VERIFY_GRID", "1");
        if (!game::mSound.isTrackPlaying(SOUNDID_MENU_MUSICLOOP)) return 2;
        for (int i = 0; i < 180; ++i) oglScene.run();
        game::mPlayers.mPlayer1->mJoined = true;
        theGame.startGame(game::GAMETYPE_SINGLEPLAYER);
        SDL_SetHint("OPENGW_BENCHMARK", "1");
        if (theGame.numPlayers() != 1) {
            std::fputs("Single-player initialization incorrectly joined extra players\n", stderr);
            return 3;
        }
        for (int i = 0; i < 600; ++i) oglScene.run();
        if (!highscoreSelfTest()) return 5;
        if (!pauseSelfTest()) return 6;
        std::puts("OpenGW simulation/audio initialization self-test passed");
        return 0;
    }
    if (startGame) {
        game::mPlayers.mPlayer1->mJoined = true;
        theGame.startGame(game::GAMETYPE_SINGLEPLAYER);
    }
    if (benchmark) SDL_SetHint("OPENGW_BENCHMARK", "1");
    if (showMenu) {
        menuSelectGameType::init(0);
        game::mGameMode = game::GAMEMODE_CHOOSE_GAMETYPE;
    }
    if (!startGame && !showMenu) mainMenu::open();
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 0);
    SDL_Window* window = SDL_CreateWindow("OpenGW", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 720, 720,
        SDL_WINDOW_OPENGL | (windowed ? SDL_WINDOW_RESIZABLE : SDL_WINDOW_FULLSCREEN_DESKTOP));
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if (!context) {
        std::fprintf(stderr, "SDL_GL_CreateContext: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        return 1;
    }
    SDL_GL_SetSwapInterval(1);
    SDL_ShowCursor(SDL_DISABLE);
    std::fprintf(stderr, "Renderer: %s\n", glGetString(GL_RENDERER));
    bool running = true;
    int frames = 0;
    const Uint64 frequency = SDL_GetPerformanceFrequency();
    const Uint64 step = frequency / 60;
    Uint64 previous = SDL_GetPerformanceCounter();
    const Uint64 started = previous;
    Uint64 lastReport = previous;
    int reportFrames = 0;
    Uint64 accumulator = step;
    while (running && !quitRequested && (!frameLimit || frames < frameLimit)) {
        const Uint64 begin = SDL_GetPerformanceCounter();
        accumulator += begin - previous;
        previous = begin;
        // Limit catch-up after suspend or a slow frame.
        if (accumulator > step * 5) accumulator = step * 5;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            const bool inMenu = game::mGameMode == game::GAMEMODE_MAINMENU ||
                game::mGameMode == game::GAMEMODE_SCORETABLE || game::mGameMode == game::GAMEMODE_HIGHSCORES ||
                game::mGameMode == game::GAMEMODE_CHOOSE_GAMETYPE;
            if (mainMenu::handleEvent(event) || event.type == SDL_QUIT) running = false;
            if (inMenu && game::mGameMode == game::GAMEMODE_CHOOSE_GAMETYPE &&
                event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) mainMenu::open();
            const bool controlMenu = game::mGameMode == game::GAMEMODE_CHOOSE_GAMETYPE;
            const bool swapButton = (event.type == SDL_CONTROLLERBUTTONDOWN &&
                event.cbutton.button == SDL_CONTROLLER_BUTTON_X) ||
                (event.type == SDL_KEYDOWN && !event.key.repeat && event.key.keysym.sym == SDLK_TAB);
            if (controlMenu && swapButton) {
                game::mSettings.mSwapSticks = !game::mSettings.mSwapSticks;
                if (!game::mSettings.saveControls()) {
                    game::mSettings.mSwapSticks = !game::mSettings.mSwapSticks;
                    std::perror("Cannot save controls.cfg");
                } else {
                    game::mSound.playTrack(SOUNDID_MENU_SELECT);
                }
            }
        }
        SDL_GameControllerUpdate();
#ifndef __APPLE__
        for (int i = 0; i < SDL_NumJoysticks(); ++i) {
            SDL_GameController* pad = SDL_GameControllerFromInstanceID(SDL_JoystickGetDeviceInstanceID(i));
            if (pad && SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_START) &&
                SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_BACK)) running = false;
        }
#endif
        while (running && accumulator >= step) {
            profile::Scope timer(profile::SIMULATION);
            if (benchmark) {
                // Reproducible stress case: continuous firing and several
                // moving forces deforming the live grid.
                const float phase = SDL_GetTicks() * .001f;
                for (int n = 0; n < 8; ++n) {
                    attractor::Attractor* force = game::mAttractors.getAttractor();
                    if (!force) break;
                    force->pos = Point3d(66 + 20 * std::cos(phase + n), 44 + 18 * std::sin(phase + n));
                    force->radius = 12;
                    force->strength = 10.f;
                    force->attractsParticles = true;
                    force->enabled = true;
                }
            }
            oglScene.run();
            accumulator -= step;
        }
        int width, height;
        SDL_GL_GetDrawableSize(window, &width, &height);
        if (width > 0 && height > 0) {
            oglScene.size(width, height);
            glViewport(0, 0, width, height);
            {
                profile::Scope timer(profile::RENDER);
                oglScene.draw(scene::RENDERPASS_PRIMARY);
            }
            if (screenshot && frameLimit && frames == frameLimit - 1) {
                SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormat(0, width, height, 24, SDL_PIXELFORMAT_RGB24);
                if (surface) {
                    glPixelStorei(GL_PACK_ALIGNMENT, 1);
                    for (int y = 0; y < height; ++y)
                        glReadPixels(0, height - y - 1, width, 1, GL_RGB, GL_UNSIGNED_BYTE,
                            static_cast<unsigned char*>(surface->pixels) + y * surface->pitch);
                    if (SDL_SaveBMP(surface, screenshot) != 0) std::fprintf(stderr, "Screenshot: %s\n", SDL_GetError());
                    SDL_FreeSurface(surface);
                }
            }
            {
                profile::Scope timer(profile::SWAP);
                SDL_GL_SwapWindow(window);
            }
        }
        ++frames;
        ++reportFrames;
        Uint64 now = SDL_GetPerformanceCounter();
        if (now - lastReport >= frequency * 5) {
            const char* labels[] = {"grid-update", "particle-update", "grid-draw", "particle-draw", "simulation", "render", "swap"};
            std::fprintf(stderr, "Performance %.1f FPS", reportFrames * static_cast<double>(frequency) / (now - lastReport));
            for (int p = 0; p < profile::COUNT; ++p) {
                std::fprintf(stderr, " %s=%.2fms", labels[p], profile::calls[p] ?
                    profile::elapsed[p] * 1000.0 / frequency / profile::calls[p] : 0.0);
                profile::elapsed[p] = profile::calls[p] = 0;
            }
            std::fputc('\n', stderr);
            lastReport = now;
            reportFrames = 0;
        }
        const Uint64 elapsed = SDL_GetPerformanceCounter() - begin;
        if (elapsed < step) SDL_Delay(static_cast<Uint32>((step - elapsed) * 1000 / frequency));
    }
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    const double seconds = static_cast<double>(SDL_GetPerformanceCounter() - started) / frequency;
    std::fprintf(stderr, "Rendered %d frames in %.2f s (%.1f FPS)\n", frames, seconds, frames / seconds);
    return 0;
}
