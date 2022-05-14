#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QThread>
#include <QWaitCondition>
#include <avdecoder.h>
#include <SDL.h>

class AudioPlayer
{
public:
    AudioPlayer();
    void Init(AVDecoder*);

    void Start();
    void Resume();
    void Stop();
    void Close();
    static void SetVolume(double volume);

private:
    void release();
    static void fillAudioBuffer(void*, Uint8*, int);

private:
    SDL_AudioSpec m_sdl_audio_spec;

    uint8_t* m_extra_data=nullptr;
    int m_extra_len=-1;
    static double m_volume;

    AVInfomation* m_information;
    AVDecoder* m_decoder=nullptr;
};

#endif // AUDIOPLAYER_H
