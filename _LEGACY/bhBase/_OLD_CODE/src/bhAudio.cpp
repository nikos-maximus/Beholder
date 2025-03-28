#include "bhAudio.h"
#include <SDL.h>
#include <vector>

#define NUM_CHANNELS 2

namespace bhAudio
{
    static SDL_AudioDeviceID g_deviceID = 0;
    static SDL_AudioSpec g_deviceSpec = {};

    struct Sample
    {
        float value[NUM_CHANNELS];
    };

    std::vector<Sample> outBuf;

    void AudioCallback(void* userdata, Uint8* stream, int len)
    {
        static float angle = 0.f;
        sinf(angle);
        for (int s = 0; s < g_deviceSpec.samples; ++s)
        {
            outBuf[s].value[0] = outBuf[s].value[1] = sinf(angle);
            angle += 0.03f;
        }
        memcpy(stream, outBuf.data(), len);
    }

    bool Init()
    {
        if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
        {
            return false;
        }
        SDL_AudioSpec desired = {};
        desired.freq = 44100;
        desired.format = AUDIO_F32SYS;
        desired.channels = NUM_CHANNELS;
        desired.samples = 1024;
        desired.callback = AudioCallback;
        //desired.userdata = ;

        g_deviceID = SDL_OpenAudioDevice(nullptr, 0, &desired, &g_deviceSpec, 0);
        if (g_deviceID < 2) //success
        {
            return false;
        }
        outBuf.resize(g_deviceSpec.samples);
        return true;
    }

    void Destroy()
    {
        SDL_CloseAudioDevice(g_deviceID);
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
    }

    void Play()
    {
        SDL_PauseAudioDevice(g_deviceID, 0);
    }

    void Pause()
    {
        SDL_PauseAudioDevice(g_deviceID, 1);
    }
}
