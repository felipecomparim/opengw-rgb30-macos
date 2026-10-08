#include "sound.h"
#include "game.h"
#include <atomic>
#include <algorithm>
namespace {
std::atomic<int> musicVolume{100}, effectsVolume{100};
}
void sound::setMasterVolumes(int music, int effects) {
    musicVolume.store(std::max(0, std::min(100, music)));
    effectsVolume.store(std::max(0, std::min(100, effects)));
}
float sound::masterGain(int track) {
    return (track == SOUNDID_MUSICLOOP || track == SOUNDID_MENU_MUSICLOOP ?
        musicVolume.load() : effectsVolume.load()) / 100.f;
}


#define SAMPLE_SIZE 512
#define NUM_TRACKS  200

sound::TRACK* sound::mTracks;
float* sound::mLeftSamples;
float* sound::mRightSamples;


sound::sound()
{
    OutputDebugString(L"Initing sound\n");

#ifdef _WIN32
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
#endif

    sound::mTracks = new TRACK[NUM_TRACKS];

    for (int i=0; i < NUM_TRACKS; i++)
    {
        mTracks[i].data = NULL;
        mTracks[i].vol = 0;
        mTracks[i].playing = FALSE;
        mTracks[i].paused = FALSE;
        mTracks[i].speed = 1;
    }

    mLeftSamples = new float [SAMPLE_SIZE*4];
    mRightSamples = new float [SAMPLE_SIZE*4];
}

sound::~sound()
{
    stopSound();
	stopAllTracks();

    SDL_LockAudio();

    for (int i=0; i < NUM_TRACKS; i++)
    {
        if (mTracks[i].data)
            free(mTracks[i].data);
    }

    delete[] sound::mTracks;

    delete[] mLeftSamples;
    delete[] mRightSamples;

    SDL_UnlockAudio();

    SDL_CloseAudio();
#ifdef _WIN32
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
#endif
}

void sound::bufferCallback(void *unused, Uint8 *stream, int len)
{
	Sint16 *buf = (Sint16 *)stream;

    // Clear the buffers
    memset(buf, 0, len);

    memset(mLeftSamples, 0, (len / 4) * sizeof(float));
    memset(mRightSamples, 0, (len / 4) * sizeof(float));

    len /= 2;

    int i, s;

	const int max_audioval = ((1<<(16-1))-1);
	const int min_audioval = -(1<<(16-1));
    const int max_int_val = (1<<16);

    // Fill the input buffers

    for (i=0; i<NUM_TRACKS; ++i)
    {
        int rIndex = 0;
        int lIndex = 0;

        TRACK* track = &mTracks[i];
        const float volume = track->vol * masterGain(i);

        if (track->playing && !track->paused)
        {
        	Sint16 *data = (Sint16 *)track->data;

		    for (s=0; s < len; s++)
		    {
                // Stream the audio data
                if (s&1)
                {
                    // Right channel
                    float fPos = track->pos;
                    float iPos = (int)fPos;

                    float v1 = (data[(int)iPos] * (volume/2));
                    float fval = (v1 / max_int_val);


                    mRightSamples[rIndex] += fval;
                    ++rIndex;
                }
                else
                {
                    // Left channel
                    float fPos = track->pos;
                    float iPos = (int)fPos;

                    float v1 = (data[(int)iPos] * (volume/2));
                    float fval = (v1 / max_int_val);

                    mLeftSamples[lIndex] += fval;
                    ++lIndex;
                }

                // Advance the track data position

                track->pos += track->speed;

			    if (track->pos >= track->len)
			    {
                    if (track->loop)
                    {
                        track->pos -= track->len;
                    }
                    else
                    {
                        track->pos = 0;
                        track->playing = FALSE;
                        break;
                    }
			    }
		    }
        }
    }

    // Fill the output buffer
    for (int i=0, b=0; i<len/2; i++)
    {
        Sint16 left = (mLeftSamples[i]) * (max_int_val);
        Sint16 right = (mRightSamples[i]) * (max_int_val);
        buf[b++] = left;
        buf[b++] = right;
    }
}

void sound::loadTrack(char *file, int track, float volume, bool loop/*=false*/)
{
    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;
    SDL_AudioCVT cvt;

    // Load the sound file and convert it to 16-bit stereo at 44Hz
    if ( SDL_LoadWAV(file, &wave, &data, &dlen) == NULL )
    {
        TCHAR s[512];
        wsprintf(s, L"Failed loading audio track %d\n", track);
        OutputDebugString(s);
        return;
    }

    TCHAR s[512];
    wsprintf(s, L"Loaded audio track %d\n", track);
    OutputDebugString(s);

    if (SDL_BuildAudioCVT(&cvt,
        wave.format,
        wave.channels,
        wave.freq,
        AUDIO_S16SYS,
        2,
        44100) < 0) {
        fprintf(stderr, "Audio conversion setup: %s\n", SDL_GetError());
        SDL_FreeWAV(data);
        return;
    }

    cvt.buf = (Uint8*)malloc(dlen*cvt.len_mult);
    if (!cvt.buf) { SDL_FreeWAV(data); return; }
    memcpy(cvt.buf, data, dlen);
    cvt.len = dlen;
    SDL_FreeWAV(data);

    if (SDL_ConvertAudio(&cvt) != 0)
    {
        TCHAR s[256];
        wsprintf(s, L"Failed to convert track %d\n", track);
        OutputDebugString(s);
        free(cvt.buf);
        return;
    }

    if (mTracks[track].data)
    {
        free(mTracks[track].data);
        mTracks[track].data = NULL;
    }
    SDL_LockAudio();
    mTracks[track].data = cvt.buf;
    mTracks[track].len = cvt.len_cvt/2;
    mTracks[track].pos = 0;
    mTracks[track].loop = loop;
    mTracks[track].vol = volume;
    mTracks[track].playing = FALSE;
    SDL_UnlockAudio();


}

void sound::startSound()
{
    SDL_AudioSpec desired = {};
    desired.channels = 2;
    desired.freq = 44100;
    desired.format = AUDIO_S16SYS;
    desired.samples = SAMPLE_SIZE;
    desired.callback = sound::bufferCallback;
    if (SDL_OpenAudio(&desired, NULL) != 0)
        fprintf(stderr, "Unable to open audio: %s\n", SDL_GetError());
    else SDL_PauseAudio(0);
}

void sound::stopSound()
{
    SDL_PauseAudio(1);
}

void sound::playTrack(int track)
{
    SDL_LockAudio();
    if (!mTracks[track].data || !mTracks[track].len) { SDL_UnlockAudio(); return; }
    mTracks[track].pos = 0;
    mTracks[track].playing = TRUE;
    SDL_UnlockAudio();
}

void sound::stopTrack(int track)
{
    SDL_LockAudio();
    mTracks[track].pos = 0;
    mTracks[track].playing = FALSE;
    SDL_UnlockAudio();
}

void sound::stopAllTracks()
{
    SDL_LockAudio();
    for (int i=0; i<NUM_TRACKS; i++)
    {
        mTracks[i].pos = 0;
        mTracks[i].playing = FALSE;
    }
    SDL_UnlockAudio();
}

void sound::stopAllTracksBut(int track)
{
    SDL_LockAudio();
    for (int i=0; i<NUM_TRACKS; i++)
    {
        if (i != track)
        {
            mTracks[i].pos = 0;
            mTracks[i].playing = FALSE;
        }
    }
    SDL_UnlockAudio();
}

void sound::pauseTrack(int track)
{
    SDL_LockAudio();
    mTracks[track].paused = TRUE;
    SDL_UnlockAudio();
}

void sound::unpauseTrack(int track)
{
    SDL_LockAudio();
    mTracks[track].paused = FALSE;
    SDL_UnlockAudio();
}

void sound::pauseAllTracks()
{
    SDL_LockAudio();
    for (int i=0; i<NUM_TRACKS; i++)
    {
        mTracks[i].paused = TRUE;
    }
    SDL_UnlockAudio();
}

void sound::pauseAllTracksBut(int track)
{
    SDL_LockAudio();
    for (int i=0; i<NUM_TRACKS; i++)
    {
        if (i != track)
        {
            mTracks[i].paused = TRUE;
        }
    }
    SDL_UnlockAudio();
}

void sound::unpauseAllTracks()
{
    SDL_LockAudio();
    for (int i=0; i<NUM_TRACKS; i++)
    {
        mTracks[i].paused = FALSE;
    }
    SDL_UnlockAudio();
}

void sound::setTrackSpeed(int track, double speed)
{
    SDL_LockAudio();
    mTracks[track].speed = speed;
    SDL_UnlockAudio();
}

bool sound::isTrackPlaying(int track)
{
    SDL_LockAudio();
    bool playing = mTracks[track].playing;
    SDL_UnlockAudio();
    return playing;
}

int sound::playTrackGroup(int trackFirst, int trackLast)
{
    for (int i=trackFirst; i<=trackLast; i++)
    {
        if (!isTrackPlaying(i))
        {
            playTrack(i);
            return i;
        }
    }
    // Just retrigger the first one
    playTrack(trackFirst);
    return trackFirst;
}
