#include "settings.h"

#include "game.h"
#include <cstdio>
#include <cstring>
#include <string>
#include "platform/storage_path.h"


settings::settings()
{
    mCreditsPerGame = 0; // 0 = Freeplay

    mEnableGlow = true;

    mGridSmoothing = false;
    mParticleSmoothing = false;
    mEnemySmoothing = false;
    mPlayerSmoothing = true;
    mStarSmoothing = true;
    mSwapSticks = false;
    mSwapKeyboard = false;
    mMusicVolume = mSoundVolume = 100;
#ifndef _WIN32
    loadControls();
#endif
}

void settings::loadControls(const char* path)
{
    const std::string resolved = path ? path : storagePath("controls.cfg");
    path = resolved.c_str();
    FILE* file = std::fopen(path, "r");
    if (!file) return;
    char line[128], name[64]; int value;
    while (std::fgets(line, sizeof(line), file)) {
        if (std::sscanf(line, "%63[^=]=%d", name, &value) != 2) continue;
        if (!std::strcmp(name, "swap_sticks") && (value == 0 || value == 1)) mSwapSticks = value != 0;
        if (!std::strcmp(name, "swap_keyboard") && (value == 0 || value == 1)) mSwapKeyboard = value != 0;
        if (!std::strcmp(name, "music_volume") && value >= 0 && value <= 100) mMusicVolume = value;
        if (!std::strcmp(name, "sound_volume") && value >= 0 && value <= 100) mSoundVolume = value;
    }
    std::fclose(file);
    sound::setMasterVolumes(mMusicVolume, mSoundVolume);
}

bool settings::saveControls(const char* path) const
{
    const std::string resolved = path ? path : storagePath("controls.cfg");
    path = resolved.c_str();
    const std::string temporary = std::string(path) + ".tmp";
    FILE* file = std::fopen(temporary.c_str(), "w");
    if (!file) return false;
    const bool written = std::fprintf(file, "swap_sticks=%d\nswap_keyboard=%d\nmusic_volume=%d\nsound_volume=%d\n",
        mSwapSticks ? 1 : 0, mSwapKeyboard ? 1 : 0, mMusicVolume, mSoundVolume) > 0;
    const bool closed = std::fclose(file) == 0;
    if (written && closed && std::rename(temporary.c_str(), path) == 0) return true;
    std::remove(temporary.c_str());
    return false;
}

settings::~settings()
{
}
