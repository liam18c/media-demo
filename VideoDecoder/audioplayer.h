#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QThread>
#include <QWaitCondition>

#include <SDL.h>

#include "avdecoder.h"

class AudioPlayer:public QObject
{
    Q_OBJECT
public:
    AudioPlayer();
    void Init(AVDecoder*);
    void Start();
    void Resume();
    void Stop();
    void Close();
    void SetPlayMode(int flag);
    static void SetVolume(double volume);
    double static GetVolume();
    AudioFrame* GetCurrentFrame();

signals:
    void PlayFinish();
    void AudioPositionChange(qint64);

private:
    static void fillAudioBuffer(void*, Uint8*, int);

private:
    SDL_AudioSpec m_sdl_audio_spec;

    AudioFrame* m_audio_frame=nullptr;
    int m_extra_len=-1;
    static double m_volume;
    int m_play_mode;
    std::atomic_bool m_exit = false;

    AVInfomation* m_information;
    AVDecoder* m_decoder=nullptr;
};

#endif // AUDIOPLAYER_H
