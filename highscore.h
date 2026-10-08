#pragma once
#include <SDL.h>
#include <string>

class highscore {
public:
    explicit highscore(const char* path = nullptr);
    ~highscore();
    void init();
    void run();
    void handleEvent(const SDL_Event& event);
    bool isHighScore(int score);
    void drawEnterScore();
    void drawTable();
    // Also used to verify that a submitted score survives a fresh load.
    int scoreAt(int index) const;
    const char* nameAt(int index) const;
private:
    struct ENTRY { char name[13]; int score; };
    ENTRY mEntries[10];
    int mEditCurrentPos;
    char mEditName[13];
    int mScore;
    int mHorizontalAxis, mVerticalAxis;
    bool mSaveFailed;
    std::string mPath;
    void load(bool importLegacy);
    bool save();
    void finish();
    void changeLetter(int direction);
};
