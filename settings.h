#ifndef __SETTINGS_H__
#define __SETTINGS_H__


class settings
{
public:

    settings();
    ~settings();

    bool mCreditsPerGame;

    bool mEnableGlow;

    bool mGridSmoothing;
    bool mParticleSmoothing;
    bool mEnemySmoothing;
    bool mPlayerSmoothing;
    bool mStarSmoothing;

    bool mSwapSticks;
    bool mSwapKeyboard;
    int mMusicVolume;
    int mSoundVolume;
    void loadControls(const char* path = nullptr);
    bool saveControls(const char* path = nullptr) const;

};


#endif // __SETTINGS_H__
