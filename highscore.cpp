#include "highscore.h"
#include "game.h"
#include "mainMenu.h"
#include "platform/storage_path.h"
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <cstdlib>
extern vector::pen defaultFontPen;

namespace {
const char letters[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
bool allowed(char c) { return (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == ' '; }
}
highscore::highscore(const char* path) : mEditCurrentPos(0), mScore(0),
    mHorizontalAxis(0), mVerticalAxis(0), mSaveFailed(false),
    mPath(path ? path : storagePath("highscores.txt")) {
    std::memset(mEntries, 0, sizeof(mEntries));
    std::memset(mEditName, 0, sizeof(mEditName));
    load(path == nullptr);
}
highscore::~highscore() {}
bool highscore::isHighScore(int score) { return score > 0 && score > mEntries[9].score; }
int highscore::scoreAt(int index) const { return index >= 0 && index < 10 ? mEntries[index].score : 0; }
const char* highscore::nameAt(int index) const { return index >= 0 && index < 10 ? mEntries[index].name : ""; }
void highscore::init() {
    mScore = game::mPlayers.mPlayer1->mScore;
    mEditCurrentPos = 0;
    std::memset(mEditName, 0, sizeof(mEditName));
    mHorizontalAxis = mVerticalAxis = 0;
    mSaveFailed = false;
    SDL_StartTextInput();
}
void highscore::run() {} // Editing is event-driven, so held fire keys are harmless.
void highscore::changeLetter(int direction) {
    const int length = std::strlen(mEditName);
    if (mEditCurrentPos >= length) {
        if (length >= 12) mEditCurrentPos = 11;
        else { mEditName[length] = 'A'; mEditName[length + 1] = 0; }
    } else {
        const char* found = std::strchr(letters, mEditName[mEditCurrentPos]);
        const int index = found ? found - letters : 0;
        mEditName[mEditCurrentPos] = letters[(index + direction + 37) % 37];
    }
}
void highscore::handleEvent(const SDL_Event& event) {
    if (event.type == SDL_TEXTINPUT) {
        for (const char* p = event.text.text; *p; ++p) {
            char c = *p;
            if (c >= 'a' && c <= 'z') c -= 'a' - 'A';
            if (!allowed(c)) continue;
            int length = std::strlen(mEditName);
            if (mEditCurrentPos >= 12) break;
            mEditName[mEditCurrentPos++] = c;
            if (mEditCurrentPos > length) mEditName[mEditCurrentPos] = 0;
        }
        mSaveFailed = false;
        return;
    }
    const bool key = event.type == SDL_KEYDOWN && !event.key.repeat;
    const bool button = event.type == SDL_CONTROLLERBUTTONDOWN;
    if ((key && event.key.keysym.sym == SDLK_RETURN) ||
        (button && event.cbutton.button == SDL_CONTROLLER_BUTTON_A)) { finish(); return; }
    if ((key && event.key.keysym.sym == SDLK_ESCAPE) ||
        (button && event.cbutton.button == SDL_CONTROLLER_BUTTON_B)) { mainMenu::open(); return; }
    if (key && event.key.keysym.sym == SDLK_BACKSPACE) {
        const int length = std::strlen(mEditName);
        if (mEditCurrentPos > 0) {
            --mEditCurrentPos;
            std::memmove(mEditName + mEditCurrentPos, mEditName + mEditCurrentPos + 1, length - mEditCurrentPos);
        }
        return;
    }
    int horizontal = 0, vertical = 0;
    if (key) {
        if (event.key.keysym.sym == SDLK_LEFT) horizontal = -1;
        if (event.key.keysym.sym == SDLK_RIGHT) horizontal = 1;
        if (event.key.keysym.sym == SDLK_UP) vertical = -1;
        if (event.key.keysym.sym == SDLK_DOWN) vertical = 1;
    }
    if (button) {
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT) horizontal = -1;
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT) horizontal = 1;
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP) vertical = -1;
        if (event.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN) vertical = 1;
    }
    if (event.type == SDL_CONTROLLERAXISMOTION && event.caxis.axis <= SDL_CONTROLLER_AXIS_RIGHTY) {
        const int value = event.caxis.value > 18000 ? 1 : event.caxis.value < -18000 ? -1 : 0;
        int& previous = event.caxis.axis % 2 ? mVerticalAxis : mHorizontalAxis;
        if (value && value != previous) {
            if (event.caxis.axis % 2) vertical = value; else horizontal = value;
        }
        previous = value;
    }
    if (horizontal) mEditCurrentPos = std::max(0, std::min(static_cast<int>(std::strlen(mEditName)), mEditCurrentPos + horizontal));
    if (vertical) changeLetter(vertical);
}
void highscore::finish() {
    char name[13];
    std::strcpy(name, mEditName);
    int length = std::strlen(name);
    while (length && name[length - 1] == ' ') name[--length] = 0;
    if (!length) std::strcpy(name, "AAA");
    ENTRY previous[10]; std::memcpy(previous, mEntries, sizeof(previous));
    for (int i = 0; i < 10; ++i) {
        if (mScore > mEntries[i].score) {
            for (int j = 9; j > i; --j) mEntries[j] = mEntries[j - 1];
            std::strcpy(mEntries[i].name, name); mEntries[i].score = mScore;
            break;
        }
    }
    if (!save()) {
        std::memcpy(mEntries, previous, sizeof(previous));
        mSaveFailed = true;
        return;
    }
    SDL_StopTextInput();
    game::mGameMode = game::GAMEMODE_SCORETABLE;
}
void highscore::drawEnterScore() {
    vector::pen pen(0, 1, .5, 1, 2);
    font::AlphanumericsPrint(font::ALIGN_CENTER, .035, 0, .55, &pen, "New High Score!");
    font::AlphanumericsPrint(font::ALIGN_CENTER, .035, 0, .3, &pen, "%d", mScore);
    pen = vector::pen(1, 1, 1, 1, 2);
    font::AlphanumericsPrint(font::ALIGN_CENTER, .018, 0, .1, &pen, "Enter Your Name - 12 Characters");
    font::AlphanumericsPrint(font::ALIGN_CENTER, .035, 0, -.15, &pen, "%s", mEditName[0] ? mEditName : "_");
    font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.35, &pen, "Editing Character %d", std::min(mEditCurrentPos + 1, 12));
    font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.55, &pen, "Type Name - Enter / A: Save");
    font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.65, &pen, "Gamepad: Up / Down Letter - Left / Right Position");
    font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.75, &pen,
        mSaveFailed ? "Save Failed - Enter To Retry" : "Backspace: Delete - Esc / B: Cancel");
}
void highscore::drawTable() {
    vector::pen pen(0, 1, .5, 1, 2);
    font::AlphanumericsPrint(font::ALIGN_CENTER, .035, 0, .75, &pen, "High Scores - Solo");
    for (int i = 0; i < 10; ++i) {
        const float y = .5f - i * .095f;
        font::AlphanumericsPrint(font::ALIGN_LEFT, .016, -.8, y, &pen, "%d", i + 1);
        font::AlphanumericsPrint(font::ALIGN_LEFT, .016, -.6, y, &pen, "%s", mEntries[i].score ? mEntries[i].name : "---");
        font::AlphanumericsPrint(font::ALIGN_RIGHT, .016, .8, y, &pen, "%d", mEntries[i].score);
    }
    pen = vector::pen(1, 1, 1, 1, 2);
    font::AlphanumericsPrint(font::ALIGN_CENTER, .014, 0, -.75, &pen, "Enter / Esc / A / B: Main Menu");
}
void highscore::load(bool importLegacy) {
    FILE* file = std::fopen(mPath.c_str(), "r");
    if (file) {
        char line[256];
        if (std::fgets(line, sizeof(line), file) && std::strcmp(line, "OPENGW_HIGHSCORES_1\n") == 0) {
            int i = 0;
            while (i < 10 && std::fgets(line, sizeof(line), file)) {
                char* tab = std::strchr(line, '\t');
                if (!tab) continue;
                *tab++ = 0;
                if (std::strlen(line) > 12 || !line[0]) continue;
                bool valid = true;
                for (char* p = line; *p; ++p) if (!allowed(*p)) valid = false;
                char* end; long score = std::strtol(tab, &end, 10);
                if (!valid || score <= 0 || score > 2147483647L || (*end != '\n' && *end != 0)) continue;
                std::strcpy(mEntries[i].name, line); mEntries[i++].score = score;
            }
        }
        std::fclose(file);
    } else if (importLegacy) {
        struct LegacyEntry { char name[4]; int score; } entries[10];
        FILE* legacy = std::fopen(storagePath("scores.sav").c_str(), "rb");
        if (legacy) {
            if (std::fread(entries, sizeof(entries), 1, legacy) == 1) {
                for (int i = 0; i < 10; ++i) {
                    if (entries[i].score <= 0 || entries[i].name[3] != 0) continue;
                    bool valid = true;
                    for (int j = 0; j < 3; ++j) if (!allowed(entries[i].name[j])) valid = false;
                    if (valid) { std::strcpy(mEntries[i].name, entries[i].name); mEntries[i].score = entries[i].score; }
                }
            }
            std::fclose(legacy);
        }
    }
    std::stable_sort(mEntries, mEntries + 10, [](const ENTRY& a, const ENTRY& b) { return a.score > b.score; });
}
bool highscore::save() {
    const std::string temporary = mPath + ".tmp";
    FILE* file = std::fopen(temporary.c_str(), "w");
    if (!file) return false;
    bool ok = std::fprintf(file, "OPENGW_HIGHSCORES_1\n") > 0;
    for (const auto& entry : mEntries) if (entry.score)
        ok = (std::fprintf(file, "%s\t%d\n", entry.name, entry.score) > 0) && ok;
    ok = !std::ferror(file) && ok;
    const bool closed = std::fclose(file) == 0;
    if (ok && closed && std::rename(temporary.c_str(), mPath.c_str()) == 0) return true;
    std::remove(temporary.c_str());
    return false;
}
