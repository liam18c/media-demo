#ifndef VIDEOPLAYERTHREAD_H
#define VIDEOPLAYERTHREAD_H

#include <QThread>
#include <QMutex>
#include "avdecoder.h"
#include "audioplayer.h"

#include <SDL.h>


class VideoPlayerThread : public QThread
{
    Q_OBJECT
public:
    VideoPlayerThread();
    void Init(AVDecoder*decoder,void*winId);

    void Start();
    void Resume();
    void Stop();
    void Close();

    void SetPlayMode(int flag);
    VideoFrame* GetCurrentFrame();

signals:
    void PlayFinish();

protected:
    void run() override;

private:
    void release();

private:
    SDL_Window* m_sdl_window;
    SDL_Renderer* m_sdl_render;
    SDL_Texture* m_sdl_texture;
    SDL_Rect m_sdl_rect;

    QMutex m_mutex;
    std::atomic_bool m_exit = false;
    std::atomic_bool m_stop = false;
    int m_play_mode;

    AVDecoder* m_decoder;
    AudioPlayer* m_audio_player;
    AVInfomation*m_information;
    VideoFrame* m_videoFrame;
};

#endif // VIDEOPLAYERTHREAD_H
